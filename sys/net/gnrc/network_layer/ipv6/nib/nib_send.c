/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Fabian Hüßler <fabian.huessler@ovgu.de>
 * @}
 */
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <sys/errno.h>

#include "libbase58.h"
#include "mutex.h"
#include "net/gnrc/ipv6/nib.h"
#include "net/gnrc/ipv6/nib/pl.h"
#include "net/gnrc/ndp.h"
#include "net/gnrc/send.h"
#include "net/gnrc/ipv6/nib/send.h"
#include "net/gnrc/netif/internal.h"
#include "_nib-aac.h"
#include "_nib-internal.h"
#include "send_conf.h"
#include "send_internal.h"
#include "fmt.h"
#include "vfs.h"
#include "vfs_util.h"

#define ENABLE_DEBUG    0
#include "debug.h"

#define DEBUG_NIB_SEND(s, ...)  DEBUG("[SEND NIB] %s(): " s, __func__, ## __VA_ARGS__)

/* TODO: move to common header */
#define FOREACH_OPT(ndp_pkt, opt, icmpv6_len) \
    for (opt = (ndp_opt_t *)(ndp_pkt + 1); \
         icmpv6_len > 0; \
         icmpv6_len -= (opt->len << 3), \
         opt = (ndp_opt_t *)(((uint8_t *)opt) + (opt->len << 3)))

typedef struct nonce_cache {
    mutex_t mtx;        /* protect concurrent accesses */
    uint8_t counter;    /* [0, GNRC_SEND_NONCE_CACHE_NUMOF - 1] */
    gnrc_send_cache_nonce_t cache[GNRC_SEND_NONCE_CACHE_NUMOF];
} nonce_cache_t;

static_assert(GNRC_SEND_NONCE_CACHE_NUMOF <= UINT8_MAX,
              "GNRC_SEND_NONCE_CACHE_NUMOF must be less or equal to 255");

static nonce_cache_t _nonce_cache = { .mtx = MUTEX_INIT };

typedef struct cps_cache {
    mutex_t mtx;        /* protect concurrent accesses */
    gnrc_send_cache_cps_t cache[GNRC_SEND_CPS_CACHE_NUMOF];
} cps_cache_t;

static_assert(GNRC_SEND_CPS_CACHE_NUMOF <= UINT8_MAX,
              "GNRC_SEND_CPS_CACHE_NUMOF must be less or equal to 255");

static cps_cache_t _cps_cache = { .mtx = MUTEX_INIT };

typedef struct cpa_cache {
    mutex_t mtx;
    gnrc_send_cache_cpa_t cache[GNRC_SEND_CPA_CACHE_NUMOF];
} cpa_cache_t;

static_assert(GNRC_SEND_CPA_CACHE_NUMOF <= UINT8_MAX,
              "GNRC_SEND_CPA_CACHE_NUMOF must be less or equal to 255");

static cpa_cache_t _cpa_cache = { .mtx = MUTEX_INIT };

int gnrc_ipv6_nib_send_enable(gnrc_netif_t *netif)
{
    int ret;
    gnrc_netif_acquire(netif);
    if (!(netif->ipv6.aac_mode & GNRC_NETIF_AAC_AUTO)) {
        ret = -ENOTSUP;
        goto release;
    }
    if ((ret = gnrc_send_enable_iface(&netif->ipv6)) < 0) {
        goto release;
    }
    for (unsigned i = 0; i < ARRAY_SIZE(netif->ipv6.addrs); i++) {
        /* 6LN keeps EUI64 link-local address.
           Global and tentative addresses are also not removed */
        if (!gnrc_netif_is_6ln(netif) || !ipv6_addr_is_link_local(&netif->ipv6.addrs[i])) {
            if ((netif->ipv6.addrs_flags[i] & GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_MASK)
                == GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID) {
                gnrc_netif_ipv6_addr_remove_internal(netif, &netif->ipv6.addrs[i]);
            }
        }
    }
    netif->ipv6.aac_priv = GNRC_NETIF_IPV6_AAC_FLAG_PRIV_CGA;
    gnrc_netif_release(netif);

    gnrc_ipv6_nib_pl_t entry = {
        .iface = netif->pid,
        .pfx_len = 64,
        .pref_until = UINT32_MAX,
        .valid_until = UINT32_MAX,
        .flags.addrconf = 1,
        .flags.onlink = 1,
    };
    memcpy(entry.pfx.u8, &ipv6_addr_link_local_prefix, sizeof(entry.pfx.u8));
    void *state = NULL;
    do {
        if (entry.flags.addrconf) {
            _auto_configure_cga(netif, &entry.pfx, entry.pfx_len);
        }
    } while (gnrc_ipv6_nib_pl_iter(netif->pid, &state, &entry));

    return 0;
release:
    gnrc_netif_release(netif);
    return ret;
}

void gnrc_ipv6_nib_send_disable(gnrc_netif_t *netif)
{
    gnrc_netif_acquire(netif);
    gnrc_send_disable_iface(&netif->ipv6);
    for (unsigned i = 0; i < ARRAY_SIZE(netif->ipv6.addrs); i++) {
        if (netif->ipv6.addrs_priv[i] == GNRC_NETIF_IPV6_ADDR_PRIV_CGA) {
            gnrc_netif_ipv6_addr_remove_internal(netif, &netif->ipv6.addrs[i]);
            _evtimer_del(&netif->ipv6.addrs_timers[i]);
        }
    }
    netif->ipv6.aac_priv = GNRC_NETIF_IPV6_AAC_FLAG_PRIV_NONE;
    gnrc_netif_release(netif);

    gnrc_ipv6_nib_pl_t entry = {
        .iface = netif->pid,
        .pfx_len = 64,
        .pref_until = UINT32_MAX,
        .valid_until = UINT32_MAX,
        .flags.addrconf = 1,
        .flags.onlink = 1,
    };
    memcpy(entry.pfx.u8, &ipv6_addr_link_local_prefix, sizeof(entry.pfx.u8));
    void *state = NULL;
    do {
        if (entry.flags.addrconf) {
            _auto_configure_addr(netif, &entry.pfx, entry.pfx_len);
        }
    } while (gnrc_ipv6_nib_pl_iter(netif->pid, &state, &entry));
}

static int _check_cga_and_signature(gnrc_netif_t *netif, const ipv6_addr_t *cg_addr,
                                    const ipv6_hdr_t *ipv6,
                                    const icmpv6_hdr_t *icmpv6,
                                    const ndp_opt_cga_params_t *cga_opt,
                                    const ndp_opt_sig_t *sig_opt)
{
    (void)netif;
    int ret;
    void *pk = NULL;
    size_t pk_size = 0;
    if (cga_opt) {
        pk = gnrc_send_opt_cga_get_pk(cga_opt);
        pk_size = gnrc_send_opt_cga_get_pk_size(cga_opt);
        if (ipv6_cga_verify(cg_addr, &cga_opt->cga_par,
                            gnrc_send_opt_cga_get_pk(cga_opt),
                            gnrc_send_opt_cga_get_pk_size(cga_opt))) {
            return -GNRC_SEND_STATUS_CGA_FAIL;
        }
    }
    if (sig_opt) {
        if ((ret = gnrc_send_signature_check(icmpv6, sig_opt,
                                             pk, pk_size, &ipv6->src, &ipv6->dst))) {
            return ret;
        }
    }
    return 0;
}

static int _check_nonce(const ndp_opt_nonce_t *nonce_opt)
{
    if (nonce_opt->len * 8 - sizeof(*nonce_opt) != GNRC_SEND_NONCE_SIZE) {
        return -GNRC_SEND_STATUS_NONCE_MISMATCH;
    }
    const uint8_t *nonce = gnrc_send_opt_nonce_get_nonce(nonce_opt);
    uint8_t i = nonce[GNRC_SEND_NONCE_SIZE - 1] & (GNRC_SEND_NONCE_CACHE_NUMOF - 1);
    mutex_lock(&_nonce_cache.mtx);
    int cmp = memcmp(_nonce_cache.cache[i].nonce, nonce, sizeof(_nonce_cache.cache[i].nonce));
    mutex_unlock(&_nonce_cache.mtx);
    return cmp == 0 ? cmp : -GNRC_SEND_STATUS_NONCE_MISMATCH;
}

static void *_handle_nonce(const ndp_opt_nonce_t *nonce_opt, size_t *nonce_size)
{
    *nonce_size = nonce_opt->len * 8 - sizeof(*nonce_opt);
    return gnrc_send_opt_nonce_get_nonce(nonce_opt);
}

void *gnrc_send_nib_handle_nonce(const ndp_opt_nonce_t *nonce_opt, size_t *nonce_size)
{
    return _handle_nonce(nonce_opt, nonce_size);
}

int gnrc_send_nib_handle_nbr_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_nbr_sol_t *nbr_sol, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("handle NS\n");
    const ipv6_addr_t *cg_addr = ipv6_addr_is_unspecified(&ipv6->src)
                                 ? &nbr_sol->tgt : &ipv6->src;
    ndp_opt_cga_params_t *cga_opt = NULL;
    ndp_opt_sig_t *sig_opt = NULL;
    ndp_opt_nonce_t *nonce_opt = NULL;
    {
        size_t tmp_len = icmpv6_len - sizeof(ndp_nbr_sol_t);
        ndp_opt_t *opt;
        FOREACH_OPT(nbr_sol, opt, tmp_len) {
            if (!cga_opt && opt->type == NDP_OPT_CGA_PARAMETERS) {
                cga_opt = (ndp_opt_cga_params_t *)opt;
            }
            else if (!nonce_opt && opt->type == NDP_OPT_NONCE) {
                nonce_opt = (ndp_opt_nonce_t *)opt;
            }
            else if (opt->type == NDP_OPT_SIGNATURE) {
                sig_opt = (ndp_opt_sig_t *)opt;
                /* ignore any options that come after the first RSA Signature option */
                break;
            }
        }
    }
    if (sig_opt && !nonce_opt) {
        DEBUG_NIB_SEND("(NS) signature option requires nonce option\n");
        return -GNRC_SEND_STATUS_NONCE_MISSING;
    }
    if (!cga_opt || !sig_opt) {
        if (!cga_opt) {
            DEBUG_NIB_SEND("(NS) no CGA option\n");
        }
        if (!sig_opt) {
            DEBUG_NIB_SEND("(NS) no signature option\n");
        }
        return GNRC_SEND_STATUS_UNSECURED;
    }
    /* The nonce option in a Nbr. Sol. is handeled on the fly
       and does not have to be stored anywhere */
    int ret = _check_cga_and_signature(netif, cg_addr, ipv6,
                                       (icmpv6_hdr_t *)nbr_sol,
                                       cga_opt, sig_opt);
    return ret;
}

int gnrc_send_nib_handle_nbr_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_nbr_adv_t *nbr_adv, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("handle NA\n");
    ndp_opt_cga_params_t *cga_opt = NULL;
    ndp_opt_sig_t *sig_opt = NULL;
    ndp_opt_nonce_t *nonce_opt = NULL;
    {
        size_t tmp_len = icmpv6_len - sizeof(ndp_nbr_adv_t);
        ndp_opt_t *opt;
        FOREACH_OPT(nbr_adv, opt, tmp_len) {
            if (!nonce_opt && opt->type == NDP_OPT_NONCE) {
                nonce_opt = (ndp_opt_nonce_t *)opt;
            }
            else if (!cga_opt && opt->type == NDP_OPT_CGA_PARAMETERS) {
                cga_opt = (ndp_opt_cga_params_t *)opt;
            }
            else if (opt->type == NDP_OPT_SIGNATURE) {
                sig_opt = (ndp_opt_sig_t *)opt;
                /* ignore any options that come after the first RSA Signature option */
                break;
            }
        }
    }
    if (sig_opt && !nonce_opt && !ipv6_addr_is_multicast(&ipv6->dst)) {
        ((ndp_nbr_adv_t *)nbr_adv)->flags &= ~(NDP_NBR_ADV_FLAGS_S);
    }
    if (!cga_opt || !sig_opt || !nonce_opt) {
        if (!cga_opt) {
            DEBUG_NIB_SEND("(NA) no CGA option\n");
        }
        if (!sig_opt) {
            DEBUG_NIB_SEND("(NA) no signature option\n");
        }
        if (!nonce_opt) {
            DEBUG_NIB_SEND("(NA) no nonce option\n");
        }
        return GNRC_SEND_STATUS_UNSECURED;
    }
    int ret;
    if ((ret = _check_nonce(nonce_opt))) {
        return ret;
    }
    if ((ret = _check_cga_and_signature(netif, &ipv6->src, ipv6,
                                        (icmpv6_hdr_t *)nbr_adv,
                                        cga_opt, sig_opt))) {
        goto release;
    }

    return GNRC_SEND_STATUS_OK;
release:
    gnrc_netif_release(netif);
    return ret;
}

int gnrc_send_nib_handle_rtr_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_rtr_sol_t *rtr_sol, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("handle RS\n");
    ndp_opt_cga_params_t *cga_opt = NULL;
    ndp_opt_sig_t *sig_opt = NULL;
    ndp_opt_nonce_t *nonce_opt = NULL;
    {
        size_t tmp_len = icmpv6_len - sizeof(ndp_rtr_sol_t);
        ndp_opt_t *opt;
        FOREACH_OPT(rtr_sol, opt, tmp_len) {
            if (!cga_opt && opt->type == NDP_OPT_CGA_PARAMETERS) {
                cga_opt = (ndp_opt_cga_params_t *)opt;
            }
            else if (!nonce_opt && opt->type == NDP_OPT_NONCE) {
                nonce_opt = (ndp_opt_nonce_t *)opt;
            }
            else if (opt->type == NDP_OPT_SIGNATURE) {
                sig_opt = (ndp_opt_sig_t *)opt;
                /* ignore any options that come after the first RSA Signature option */
                break;
            }
        }
    }
    if (sig_opt && !nonce_opt) {
        DEBUG_NIB_SEND("(RS) signature option requires nonce option\n");
        return -GNRC_SEND_STATUS_NONCE_MISSING;
    }
    if (nonce_opt) {
        size_t nonce_size;
        void * nonce = _handle_nonce(nonce_opt, &nonce_size);
        if (nonce_size > sizeof(netif->ipv6.send_ctx.ra_nonce)) {
            gnrc_netif_release(netif);
            return -GNRC_SEND_STATUS_NONCE_MISMATCH;
        }
        memcpy(netif->ipv6.send_ctx.ra_nonce, nonce,
               netif->ipv6.send_ctx.ra_nonce_size = nonce_size);
    }
    if (!nonce_opt || ((!cga_opt || !sig_opt) && !ipv6_addr_is_unspecified(&ipv6->src))) {
        if (!nonce_opt) {
            DEBUG_NIB_SEND("(RS) no nonce option\n");
        }
        if (!cga_opt) {
            DEBUG_NIB_SEND("(RS) no CGA option\n");
        }
        if (!sig_opt) {
            DEBUG_NIB_SEND("(RS) no signature option\n");
        }
        return GNRC_SEND_STATUS_UNSECURED;
    }
    int ret = _check_cga_and_signature(netif, &ipv6->src, ipv6,
                                       (icmpv6_hdr_t *)rtr_sol,
                                       cga_opt, sig_opt);
    return ret;
}

int gnrc_send_nib_handle_rtr_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_rtr_adv_t *rtr_adv, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("handle RA\n");
    ndp_opt_cga_params_t *cga_opt = NULL;
    ndp_opt_sig_t *sig_opt = NULL;
    ndp_opt_nonce_t *nonce_opt = NULL;
    {
        size_t tmp_len = icmpv6_len - sizeof(ndp_rtr_adv_t);
        ndp_opt_t *opt;
        FOREACH_OPT(rtr_adv, opt, tmp_len) {
            if (!nonce_opt && opt->type == NDP_OPT_NONCE) {
                nonce_opt = (ndp_opt_nonce_t *)opt;
            }
            else if (!cga_opt && opt->type == NDP_OPT_CGA_PARAMETERS) {
                cga_opt = (ndp_opt_cga_params_t *)opt;
            }
            else if (opt->type == NDP_OPT_SIGNATURE) {
                sig_opt = (ndp_opt_sig_t *)opt;
                /* ignore any options that come after the first RSA Signature option */
                break;
            }
        }
    }
    if (!cga_opt || !sig_opt || !nonce_opt) {
        if (!cga_opt) {
            DEBUG_NIB_SEND("(RA) no CGA option\n");
        }
        if (!sig_opt) {
            DEBUG_NIB_SEND("(RA) no signature option\n");
        }
        if (!nonce_opt) {
            DEBUG_NIB_SEND("(RA) no nonce option\n");
        }
        return GNRC_SEND_STATUS_UNSECURED;
    }
    int ret;
    if ((ret = _check_nonce(nonce_opt))) {
        return ret;
    }
    if ((ret = _check_cga_and_signature(netif, &ipv6->src, ipv6,
                                        (icmpv6_hdr_t *)rtr_adv,
                                        cga_opt, sig_opt))) {
        //gnrc_netif_release(netif);
        return ret;
    }
    gnrc_send_ident_t rtr_ident;
    gnrc_send_cp_identifier(&rtr_ident,
                            gnrc_send_opt_cga_get_pk(cga_opt),
                            gnrc_send_opt_cga_get_pk_size(cga_opt));
    if (!gnrc_send_get_cp_by_ident(&rtr_ident, NULL)) {
        /* create a copy and handle the advertisement after CPS and CPA exchange */
        const ipv6_addr_t *dst = &ipv6->src, *src;
        if (!(src = gnrc_netif_ipv6_addr_best_src(netif, dst, true))) {
            src = &ipv6_addr_unspecified;
        }
        gnrc_send_cache_cps_t *ctx = NULL;
        if ((ret = gnrc_send_cp_sol_send(netif, src, dst, SEND_CPS_ALL_COMP, ipv6,
                                         &rtr_ident, rtr_adv, icmpv6_len, (void **)&ctx))) {
            DEBUG_NIB_SEND("Could not send CPS\n");
        }
        if (ctx) {
            gnrc_send_set_cps_timeout(ctx, CONFIG_SEND_CPS_RETRY_MS);
        }
        return -GNRC_SEND_STATUS_ADD_IN_PROGRESS;
    }
    return GNRC_SEND_STATUS_OK;
}

void gnrc_send_nonce_get(void *nonce)
{
    mutex_lock(&_nonce_cache.mtx);
    uint8_t i = _nonce_cache.counter++ & (ARRAY_SIZE(_nonce_cache.cache) - 1);
    gnrc_send_random_get(NULL, _nonce_cache.cache[i].nonce, sizeof(_nonce_cache.cache[i].nonce));
    _nonce_cache.cache[i].nonce[GNRC_SEND_NONCE_SIZE - 1] &= ~(ARRAY_SIZE(_nonce_cache.cache) - 1);
    _nonce_cache.cache[i].nonce[GNRC_SEND_NONCE_SIZE - 1] |= i;
    memcpy(nonce, _nonce_cache.cache[i].nonce, sizeof(_nonce_cache.cache[i].nonce));
    mutex_unlock(&_nonce_cache.mtx);
}

gnrc_send_cache_cps_t *gnrc_send_get_cps_ctx(gnrc_netif_t *netif,
                                             const ipv6_hdr_t *rtr_ipv6,
                                             const gnrc_send_ident_t *rtr_ident,
                                             const ndp_rtr_adv_t *rtr_adv,
                                             size_t rtr_adv_len)
{
    gnrc_send_cache_cps_t *new = NULL, *match = NULL;
    mutex_lock(&_cps_cache.mtx);
    for (unsigned i = 0; i < GNRC_SEND_CPS_CACHE_NUMOF; i++) {
        if (!new && _cps_cache.cache[i].identifier == 0) {
            new = &_cps_cache.cache[i]; /* new */
            assert(!new->rtr_adv); /* leak? */
        }
        if (!memcmp(rtr_ident->hash, _cps_cache.cache[i].rtr_ident.hash, sizeof(rtr_ident->hash))) {
            match = netif->pid == _cps_cache.cache[i].netif->pid
                    ? &_cps_cache.cache[i] : NULL;
            break; /* certificate retrieval is already in progress */
        }
    }
    if (!new || match) {
        mutex_unlock(&_cps_cache.mtx);
        return NULL;
    }
    /* safe rtr_adv to handle it after certificate exchange */
    gnrc_pktsnip_t *adv_cpy = gnrc_pktbuf_add(NULL, rtr_adv, rtr_adv_len, GNRC_NETTYPE_ICMPV6);
    if (!adv_cpy) {
        mutex_unlock(&_cps_cache.mtx);
        return NULL; /* no memory */
    }
    uint16_t ident;
    do {
        gnrc_send_random_get(NULL, (uint8_t *)&ident, sizeof(ident));
    } while (ident < GNRC_SEND_CPS_CACHE_NUMOF);
    ident &= ~(GNRC_SEND_CPS_CACHE_NUMOF - 1);
    ident |= (new - _cps_cache.cache);
    *new = (gnrc_send_cache_cps_t) {
        .netif = netif,
        .rtr_adv = adv_cpy,
        .identifier = ident,
        .rtr_ident = *rtr_ident,
        .rtr_ipv6 = *rtr_ipv6,
        .timeout_ms = CONFIG_SEND_CPS_RETRY_MS,
    };
    mutex_unlock(&_cps_cache.mtx);
    return new;
}

static void _nib_send_free_cps_ctx(gnrc_send_cache_cps_t **cps_ctx)
{
    DEBUG_NIB_SEND("delete CPS event %p\n", (void *)&(*cps_ctx)->ev_retransmission);
    _evtimer_del(&(*cps_ctx)->ev_retransmission);
    gnrc_pktbuf_release((*cps_ctx)->rtr_adv);
    memset(*cps_ctx, 0, sizeof(**cps_ctx));
    *cps_ctx = NULL;
}

void gnrc_send_free_cps_ctx(gnrc_send_cache_cps_t **cps_ctx)
{
    _nib_send_free_cps_ctx(cps_ctx);
}

void gnrc_send_set_cps_timeout(gnrc_send_cache_cps_t *cps_ctx, uint32_t offset)
{
    mutex_lock(&_cps_cache.mtx);
    if (cps_ctx->netif) {
        /* make sure it has not been deleted */
        DEBUG_NIB_SEND("add CPS event %p, offset: %"PRIu32"\n", (void *)&cps_ctx->ev_retransmission, offset);
        _evtimer_add(cps_ctx, GNRC_IPV6_NIB_SEND_CP_SOL, &cps_ctx->ev_retransmission, offset);
    }
    mutex_unlock(&_cps_cache.mtx);
}

gnrc_send_cache_cpa_t *gnrc_send_get_cpa_ctx(gnrc_netif_t *netif, const ipv6_addr_t *sol_addr,
                                             uint16_t ident, const gnrc_send_cp_t *cp)
{
    gnrc_send_cache_cpa_t *entry = NULL;
    mutex_lock(&_cpa_cache.mtx);
    for (unsigned i = 0; i < GNRC_SEND_CPA_CACHE_NUMOF; i++) {
        if (!entry && _cpa_cache.cache[i].identifier == 0) {
            entry = &_cpa_cache.cache[i];
            break;
        }
    }
    if (!entry) {
        mutex_unlock(&_cpa_cache.mtx);
        return NULL;
    }
    *entry = (gnrc_send_cache_cpa_t) {
        .netif = netif,
        .cp = cp,
        .sol_addr = *sol_addr,
        .identifier = ident,
        .hi_comp = cp->num_comp - 1,
    };
    mutex_unlock(&_cpa_cache.mtx);
    return entry;
}

static void _nib_send_free_cpa_ctx(gnrc_send_cache_cpa_t **cpa_ctx)
{
    DEBUG_NIB_SEND("delete CPA event %p\n", (void *)&(*cpa_ctx)->ev_transmission);
    _evtimer_del(&(*cpa_ctx)->ev_transmission);
    memset(*cpa_ctx, 0, sizeof(**cpa_ctx));
    *cpa_ctx = NULL;
}

void gnrc_send_free_cpa_ctx(gnrc_send_cache_cpa_t **cpa_ctx)
{
    mutex_lock(&_cpa_cache.mtx);
    _nib_send_free_cpa_ctx(cpa_ctx);
    mutex_unlock(&_cpa_cache.mtx);
}


void gnrc_send_set_cpa_timeout(gnrc_send_cache_cpa_t *cpa_ctx, uint32_t offset)
{
    mutex_lock(&_cpa_cache.mtx);
    if (cpa_ctx->netif) {
        DEBUG_NIB_SEND("add CPA event %p, offset: %"PRIu32"\n", (void *)&cpa_ctx->ev_transmission, offset);
        _evtimer_add(cpa_ctx, GNRC_IPV6_NIB_SEND_CP_ADV, &cpa_ctx->ev_transmission, offset);
    }
    mutex_unlock(&_cpa_cache.mtx);
}

static const gnrc_send_ta_t *_handle_ta(const icmpv6_hdr_t *icmpv6, const ndp_opt_ta_t *ta_opt)
{
    (void)icmpv6;
    if (ta_opt->name_type == NDP_TA_TYPE_DER) {
        return gnrc_send_get_ta_by_name(gnrc_send_opt_ta_get_name(ta_opt),
                                        gnrc_send_opt_ta_get_name_size(ta_opt));
    }
    else if (ta_opt->name_type == NDP_TA_TYPE_FQDN) {
        return gnrc_send_get_ta_by_fqdn(gnrc_send_opt_ta_get_name(ta_opt),
                                        gnrc_send_opt_ta_get_name_size(ta_opt));
    }
    return NULL;
}

void gnrc_send_nib_handle_cp_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_cp_sol_t *cp_sol, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("handle CPS\n");
    const gnrc_send_ta_t *ta = NULL;
    const gnrc_send_cp_t *cp = NULL;
    ndp_opt_ta_t *ta_opt = NULL;
    {
        size_t tmp_len = icmpv6_len - sizeof(ndp_cp_sol_t);
        ndp_opt_t *opt;
        FOREACH_OPT(cp_sol, opt, tmp_len) {
            if (opt->type == NDP_OPT_TRUST_ANCHOR) {
                if (!ta_opt && ((ndp_opt_ta_t *)opt)->name_type != NDP_TA_TYPE_DER) {
                    DEBUG_NIB_SEND("Warning: First Ta option in CPS is not DER encoded.\n");
                }
                ta_opt = (ndp_opt_ta_t *)opt;
                if ((ta = _handle_ta((const icmpv6_hdr_t *)cp_sol, ta_opt))) {
                    if ((cp = gnrc_send_get_cp_by_ident(gnrc_send_get_self_ident(), ta))) {
                        break;
                    }
                }
            }
        }
    }
    gnrc_pktsnip_t *opts = NULL;
    if (!cp) {
        /* If the router is unable to find a path to the requested anchor, it
           SHOULD send an advertisement without any certificates. In this case,
           the router SHOULD include the Trust Anchor options that were
           solicited. */
        gnrc_pktsnip_t *hdr;
        {
            size_t tmp_len = icmpv6_len - sizeof(ndp_cp_sol_t);
            ndp_opt_t *opt;
            FOREACH_OPT(cp_sol, opt, tmp_len) {
                if (opt->type == NDP_OPT_TRUST_ANCHOR) {
                    ta_opt = (ndp_opt_ta_t *)opt;
                    if ((hdr = gnrc_pktbuf_add(opts, ta_opt, ta_opt->len * 8, GNRC_NETTYPE_UNDEF))) {
                        opts = hdr;
                    }
                }
            }
        }
    }
    uint16_t sol_ident = byteorder_ntohs(cp_sol->ident);
    uint16_t sol_comp = byteorder_ntohs(cp_sol->comp);
    gnrc_send_cache_cpa_t *ctx = NULL;
    if (ipv6_addr_is_unspecified(&ipv6->src)) {
        const ipv6_addr_t *dst = &ipv6_addr_all_nodes_link_local;
        gnrc_send_cp_adv_send(netif,
                              gnrc_netif_ipv6_addr_best_src(netif, dst, true), dst,
                              sol_comp, sol_ident, cp, (void **)&ctx, opts);
    }
    else {
        ipv6_addr_t dst;
        ipv6_addr_set_solicited_nodes(&dst, &ipv6->src);
        gnrc_send_cp_adv_send(netif,
                              gnrc_netif_ipv6_addr_best_src(netif, &dst, true), &dst,
                              sol_comp, sol_ident, cp, (void **)&ctx, opts);
    }
    if (ctx) {
        gnrc_send_set_cpa_timeout(ctx, MS_PER_SEC / CONFIG_SEND_CPA_RATE_MAX);
    }
}

static int _handle_cert(const icmpv6_hdr_t *icmpv6, const ndp_opt_cert_t *cert_opt, const gnrc_send_ta_t *ta,
                        const char *vfs_path)
{
    int ret;
    uint16_t adv_comp = byteorder_ntohs(((const ndp_cp_adv_t *)icmpv6)->comp);
    uint16_t num_comp = byteorder_ntohs(((const ndp_cp_adv_t *)icmpv6)->all_comp);
    size_t cert_size = (cert_opt->len * 8) - sizeof(*cert_opt);
    size_t verify_certs_size;
    uint8_t *verify_certs_buf;
    gnrc_send_x509_crt_t *verify_certs = gnrc_send_cert_acquire(&verify_certs_size,
                                                                (void **)&verify_certs_buf);
    if (verify_certs_size < cert_size) {
        gnrc_send_cert_release();
        return -ENOBUFS;
    }
    memcpy(verify_certs_buf, gnrc_send_opt_cert_get_cert(cert_opt), cert_size);
    if ((ret = gnrc_send_load_crt(NULL,
                                  verify_certs_buf,
                                  cert_size,
                                  verify_certs) < 0)) {
        gnrc_send_cert_release();
        return ret;
    }
    verify_certs_buf += gnrc_send_x509_get_size(verify_certs);
    verify_certs_size -= gnrc_send_x509_get_size(verify_certs);
    char path[GNRC_SEND_PATH_MAX];
    if (adv_comp == num_comp - 1) {
        if (strlen(CONFIG_GNRC_SEND_TA_ROOT"/") + strlen(ta->rel_path)
            >= sizeof(path) - strlen(path)) {
            gnrc_send_cert_release();
            return -ENOBUFS;
        }
        strcpy(path, CONFIG_GNRC_SEND_TA_ROOT"/");
        strcat(path, ta->rel_path);
    }
    else {
        if (strlen(vfs_path) + 1 >= sizeof(path)) {
            gnrc_send_cert_release();
            return -ENOBUFS;
        }
        char pre_comp_name[10] = { 0 };
        fmt_u32_dec(pre_comp_name, adv_comp + 1);
        strcpy(path, vfs_path);
        strcat(path, "/");
        strcat(pre_comp_name, ".der");
        if (strlen(pre_comp_name) >= sizeof(path) - strlen(path)) {
            gnrc_send_cert_release();
            return -ENOBUFS;
        }
        strcat(path, pre_comp_name);
    }
    if ((ret = gnrc_send_load_crt(path, verify_certs_buf,
                                  verify_certs_size, verify_certs + 1)) < 0) {
        gnrc_send_cert_release();
        return ret;
    }
    if ((ret = gnrc_send_cert_verify(verify_certs, verify_certs + 1)) < 0) {
        gnrc_send_cert_release();
        return ret;
    }
    char comp_name[10] = { 0 };
    fmt_u32_dec(comp_name, adv_comp);
    strcat(comp_name, ".der");
    if (strlen(vfs_path) + 1 >= sizeof(path)) {
        gnrc_send_cert_release();
        return -ENOBUFS;
    }
    strcpy(path, vfs_path);
    strcat(path, "/");
    if (strlen(comp_name) >= sizeof(path) - strlen(path)) {
        gnrc_send_cert_release();
        return -ENOBUFS;
    }
    strcat(path, comp_name);
    if ((ret = vfs_file_from_buffer(path,
                                    gnrc_send_x509_get(verify_certs),
                                    gnrc_send_x509_get_size(verify_certs))) < 0) {
        gnrc_send_cert_release();
        return ret;
    }
    gnrc_send_cert_release();
    return 0;
}

void gnrc_send_nib_handle_cp_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_cp_adv_t *cp_adv, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("handle CPA\n");
    (void)ipv6; (void)netif;
    const gnrc_send_ta_t *ta = NULL;
    const ndp_opt_cert_t *crt_opt = NULL;
    uint16_t adv_ident = byteorder_ntohs(cp_adv->ident);
    uint16_t adv_comp = byteorder_ntohs(cp_adv->comp);
    uint16_t adv_comp_numof = byteorder_ntohs(cp_adv->all_comp);
    if (adv_comp == adv_comp_numof - 1) {
        size_t tmp_len = icmpv6_len - sizeof(ndp_cp_adv_t);
        const ndp_opt_t *opt;
        FOREACH_OPT(cp_adv, opt, tmp_len) {
            if (opt->type == NDP_OPT_TRUST_ANCHOR) {
                if ((ta = _handle_ta((const icmpv6_hdr_t *)cp_adv, (const ndp_opt_ta_t *)opt))) {
                    break;
                }
            }
        }
        if (!ta) {
            return;
        }
    }
    {
        size_t tmp_len = icmpv6_len - sizeof(ndp_cp_adv_t);
        const ndp_opt_t *opt;
        FOREACH_OPT(cp_adv, opt, tmp_len) {
            if (opt->type == NDP_OPT_CERTIFICATE) {
                crt_opt = (const ndp_opt_cert_t *)opt;
                break;
            }
        }
    }
    gnrc_send_cache_cps_t *c = &_cps_cache.cache[adv_ident & (ARRAY_SIZE(_cps_cache.cache) - 1)];
    gnrc_pktsnip_t *rtr_adv = NULL;
    ipv6_hdr_t rtr_ipv6;
    mutex_lock(&_cps_cache.mtx);
    if (c->identifier != adv_ident) {
        mutex_unlock(&_cps_cache.mtx);
        return;
    }
    if (adv_comp == adv_comp_numof - 1) {
        c->hi_comp = adv_comp_numof - 1;
        c->num_comp = adv_comp_numof;
    }
    else if (c->num_comp != adv_comp_numof) {
        mutex_unlock(&_cps_cache.mtx);
        return;
    }
    if (crt_opt) {
        if (c->hi_comp != adv_comp) {
            mutex_unlock(&_cps_cache.mtx);
            return;
        }
        if (!c->ta) {
            c->ta = ta;
        }
        char rtr_path[GNRC_SEND_PATH_MAX] = { 0 };
        strcat(rtr_path, CONFIG_GNRC_SEND_CP_ROOT"/");
        size_t rtr_path_len = sizeof(rtr_path) - strlen(rtr_path);
        if (!b58enc(rtr_path + strlen(rtr_path), &rtr_path_len,
            c->rtr_ident.hash, sizeof(c->rtr_ident.hash))) {
            mutex_unlock(&_cps_cache.mtx);
            return;
        }
        rtr_path[strlen(rtr_path)] = '\0';
        vfs_mkdir(rtr_path, 0);
        if (strlen(c->ta->fqdn) + 1 >= sizeof(rtr_path) - strlen(rtr_path)) {
            mutex_unlock(&_cps_cache.mtx);
            return;
        }
        strcat(rtr_path, "/");
        const char *extn = strrchr(c->ta->fqdn, '.');
        strncat(rtr_path, c->ta->fqdn,
                !strcmp(extn ? extn : "" , ".der") ? strlen(c->ta->fqdn) - 4 : strlen(c->ta->fqdn));
        vfs_mkdir(rtr_path, 0);
        if (!_handle_cert((const icmpv6_hdr_t *)cp_adv, crt_opt, c->ta, rtr_path)) {
            DEBUG_NIB_SEND("Certificate in CPA is valid\n");
            _evtimer_del(&c->ev_retransmission);
            c->timeout_ms = 0;
            if (c->hi_comp > 1) {
                /* this event triggers a CP solicitation for single components */
                DEBUG_NIB_SEND("add CPS event to retrieve eventually missing fragments %p\n",
                               (void *)&c->ev_retransmission);
                _evtimer_add(c, GNRC_IPV6_NIB_SEND_CP_SOL, &c->ev_retransmission, CONFIG_SEND_CPS_RETRY_FRAGMENTS_MS);
                c->hi_comp--;
            }
            else {
                gnrc_send_cp_t cp = {
                    .num_comp = c->num_comp,
                    .ta = c->ta,
                };
                memcpy(&cp.cp_ident, &c->rtr_ident, sizeof(cp.cp_ident));
                strcpy(cp.rel_path, rtr_path);
                gnrc_send_cp_acquire();
                int ret = gnrc_send_cp_add(&cp);
                gnrc_send_cp_release();
                if (ret) {
                    DEBUG_NIB_SEND("Certificate Path error. Aborting ADD.\n");
                    _nib_send_free_cps_ctx(&c);
                    mutex_unlock(&_cps_cache.mtx);
                    vfs_unlink_recursive(rtr_path, rtr_path, sizeof(rtr_path));
                    return;
                }
                /* received all certificate components,
                   prepare to handle the router advertisement finally */
                rtr_adv = c->rtr_adv;
                rtr_ipv6 = c->rtr_ipv6;
                gnrc_pktbuf_hold(rtr_adv, 1); /* handle below */
            }
        }
        else {
            DEBUG_NIB_SEND("Certificate error. Aborting ADD.\n");
            _nib_send_free_cps_ctx(&c);
            mutex_unlock(&_cps_cache.mtx);
            vfs_unlink_recursive(rtr_path, rtr_path, sizeof(rtr_path));
            return;
        }
    }
    else {
        DEBUG_NIB_SEND("No Certificate found in CPA. Aborting ADD.\n");
        _nib_send_free_cps_ctx(&c);
        mutex_unlock(&_cps_cache.mtx);
        return;
    }
    _nib_send_free_cps_ctx(&c); /* releases saved RA */
    mutex_unlock(&_cps_cache.mtx);
    if (rtr_adv) {
        extern void _nib_handle_rtr_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                        const ndp_rtr_adv_t *rtr_adv, size_t icmpv6_len);
        _nib_handle_rtr_adv(netif, &rtr_ipv6, (const ndp_rtr_adv_t *)rtr_adv->data,
                        rtr_adv->size);
        gnrc_pktbuf_release(rtr_adv);
    }
}

void gnrc_send_nib_handle_snd_cp_sol(gnrc_send_cache_cps_t *cps_ctx)
{
    DEBUG_NIB_SEND("handle CPS event %p\n", &cps_ctx->ev_retransmission);
    int ret;
    mutex_lock(&_cps_cache.mtx);
    if (!cps_ctx->netif) {
        /* ctx has been deleted after the message already has been posted from ISR */
        mutex_unlock(&_cps_cache.mtx);
        return;
    }
    if (cps_ctx->timeout_ms * 2 > CONFIG_SEND_CPS_RETRY_MAX_MS) {
        DEBUG_NIB_SEND("ADD process timed out. Abort ADD.\n");
        _nib_send_free_cps_ctx(&cps_ctx);
        mutex_unlock(&_cps_cache.mtx);
        return;
    }
    if ((ret = gnrc_send_cp_sol_send(cps_ctx->netif, NULL, &cps_ctx->rtr_ipv6.src,
                                     cps_ctx->hi_comp, &cps_ctx->rtr_ipv6, &cps_ctx->rtr_ident,
                                     cps_ctx->rtr_adv->data, cps_ctx->rtr_adv->size,
                                     (void **)&cps_ctx)) < 0) {
        DEBUG_NIB_SEND("Could not send CPS retransmission.\n");
    }
    cps_ctx->timeout_ms *= 2;
    uint32_t timeout = !cps_ctx->timeout_ms ? CONFIG_SEND_CPS_RETRY_MS : (cps_ctx->timeout_ms);
    mutex_unlock(&_cps_cache.mtx);
    gnrc_send_set_cps_timeout(cps_ctx, timeout);
}

void gnrc_send_nib_handle_snd_cp_adv(gnrc_send_cache_cpa_t *cpa_ctx)
{
    assert((void *)cpa_ctx == cpa_ctx->netif->ipv6.send_ctx.cpa_ctx);
    DEBUG_NIB_SEND("handle CPA event %p\n", &cpa_ctx->ev_transmission);
    int ret;
    mutex_lock(&_cpa_cache.mtx);
    if (!cpa_ctx->netif) {
        /* ctx has been deleted after the message already has been posted from ISR */
        mutex_unlock(&_cpa_cache.mtx);
        return;
    }
    if (cpa_ctx->hi_comp <= 1) {
        /* finished */
        _nib_send_free_cpa_ctx((gnrc_send_cache_cpa_t **)&cpa_ctx->netif->ipv6.send_ctx.cpa_ctx);
        return;
    }
    if ((ret = gnrc_send_cp_adv_send(cpa_ctx->netif, NULL, &cpa_ctx->sol_addr,
                                     cpa_ctx->hi_comp, cpa_ctx->identifier,
                                     cpa_ctx->cp, (void **)&cpa_ctx, NULL))) {
        DEBUG_NIB_SEND("Could not send CPA component %u.\n",
                       cpa_ctx->hi_comp);
    }
    cpa_ctx->hi_comp--;
    mutex_unlock(&_cpa_cache.mtx);
    gnrc_send_set_cpa_timeout(cpa_ctx, MS_PER_SEC / CONFIG_SEND_CPA_RATE_MAX);
}
