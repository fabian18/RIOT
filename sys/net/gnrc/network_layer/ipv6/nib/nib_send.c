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
 * @brief   GNRC IPv6 NIB SEND implementation
 *
 * @author  Fabian Hüßler <fabian.huessler@ovgu.de>
 * @}
 */
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "assert.h"
#include "kernel_defines.h"
#include "libbase58.h"
#include "mutex.h"
#include "net/gnrc/ipv6/nib.h"
#include "net/gnrc/ipv6/nib/nc.h"
#include "net/gnrc/ipv6/nib/pl.h"
#include "net/gnrc/ndp.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/send.h"
#include "net/gnrc/ipv6/nib/send.h"
#include "net/gnrc/netif/internal.h"
#include "_nib-aac.h"
#include "_nib-internal.h"
#include "_nib-send.h"
#include "net/ipv6/addr.h"
#include "fmt.h"
#include "net/send.h"
#include "vfs.h"
#include "vfs_util.h"
#if IS_USED(MODULE_GNRC_SEND_C509)
#include "c509.h"
#endif

#include "_nib-arsm.h"
#include "send_conf.h"
#include "send_internal.h"
#include "x509_ip_extn.h"

#define ENABLE_DEBUG    0
#include "debug.h"

#define DEBUG_NIB_SEND(s, ...)  DEBUG("[SEND NIB] %s(): " s, __func__, ## __VA_ARGS__)

extern void _handle_search_rtr(gnrc_netif_t *netif);

static char addr_str[IPV6_ADDR_MAX_STR_LEN];

typedef struct nonce_cache {
    uint8_t counter;    /* [0, GNRC_SEND_NONCE_CACHE_NUMOF - 1] */
    gnrc_send_cache_nonce_t cache[GNRC_SEND_NONCE_CACHE_NUMOF];
} nonce_cache_t;

static_assert(GNRC_SEND_NONCE_CACHE_NUMOF <= UINT8_MAX,
              "GNRC_SEND_NONCE_CACHE_NUMOF must be less or equal to 255");

static nonce_cache_t _nonce_cache;

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

static int _gen_vfs_name(char *name, size_t len, const gnrc_send_ident_t *ident, const char *extn)
{
    size_t max = len;
    if (extn) {
        if (len <= 1 /*.*/ + strlen(extn)) {
            return -ENOBUFS;
        }
        max = len - 1 - strlen(extn);
    }
    /* note that b58enc includes a \0 */
    if (!b58enc(name, &max, ident->hash, sizeof(ident->hash))) {
        return -ENOBUFS;
    }
    if (extn) {
        name[max - 1] = '.'; /* override \0 */
        strcpy(&name[max], extn);
    }
    return strlen(name);
}

int gnrc_ipv6_nib_send_enable(gnrc_netif_t *netif, gnrc_ipv6_nib_send_mode_t mode)
{
    int ret;
    if (mode != GNRC_NIB_IPV6_SEND_MODE_STRICT &&
        mode != GNRC_NIB_IPV6_SEND_MODE_COMPAT) {
        return -EINVAL;
    }
    gnrc_netif_acquire(netif);
    if (netif->ipv6.send_ctx.mode == GNRC_NIB_IPV6_SEND_MODE_STRICT ||
        netif->ipv6.send_ctx.mode == GNRC_NIB_IPV6_SEND_MODE_COMPAT) {
        ret = -EALREADY;
        goto release;
    }
    if ((ret = gnrc_send_check_iface(&netif->ipv6))) {
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
    netif->ipv6.send_ctx.mode = mode;
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

gnrc_ipv6_nib_send_mode_t gnrc_ipv6_nib_send_get_mode(gnrc_netif_t *netif)
{
    gnrc_netif_acquire(netif);
    gnrc_ipv6_nib_send_mode_t mode = netif->ipv6.send_ctx.mode;
    gnrc_netif_release(netif);
    return mode;
}

void gnrc_ipv6_nib_send_set_mode(gnrc_netif_t *netif, gnrc_ipv6_nib_send_mode_t mode)
{
    if (mode != GNRC_NIB_IPV6_SEND_MODE_STRICT &&
        mode != GNRC_NIB_IPV6_SEND_MODE_COMPAT) {
        return;
    }
    gnrc_netif_acquire(netif);
    netif->ipv6.send_ctx.mode = mode;
    gnrc_netif_release(netif);
}

void gnrc_ipv6_nib_send_disable(gnrc_netif_t *netif)
{
    gnrc_netif_acquire(netif);
    if (netif->ipv6.send_ctx.mode == GNRC_NIB_IPV6_SEND_MODE_OFF) {
        gnrc_netif_release(netif);
        return;
    }
    gnrc_send_disable_iface(&netif->ipv6);
    for (unsigned i = 0; i < ARRAY_SIZE(netif->ipv6.addrs); i++) {
        if (netif->ipv6.addrs_priv[i] == GNRC_NETIF_IPV6_ADDR_PRIV_CGA) {
            gnrc_netif_ipv6_addr_remove_internal(netif, &netif->ipv6.addrs[i]);
            _evtimer_del(&netif->ipv6.addrs_timers[i]);
        }
    }
    netif->ipv6.aac_priv = GNRC_NETIF_IPV6_AAC_FLAG_PRIV_NONE;
    netif->ipv6.send_ctx.mode = GNRC_NIB_IPV6_SEND_MODE_OFF;
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
    int cmp = memcmp(_nonce_cache.cache[i].nonce, nonce, sizeof(_nonce_cache.cache[i].nonce));
    return cmp == 0 ? cmp : -GNRC_SEND_STATUS_NONCE_MISMATCH;
}

static void *_handle_nonce(const ndp_opt_nonce_t *nonce_opt, size_t *nonce_size)
{
    *nonce_size = nonce_opt->len * 8 - sizeof(*nonce_opt);
    return gnrc_send_opt_nonce_get_nonce(nonce_opt);
}

void *gnrc_ipv6_nib_send_handle_nonce(const ndp_opt_nonce_t *nonce_opt, size_t *nonce_size)
{
    return _handle_nonce(nonce_opt, nonce_size);
}

int gnrc_ipv6_nib_send_handle_nbr_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_nbr_sol_t *nbr_sol, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("Iface: %d handle NS from %s\n",
                   netif->pid, ipv6_addr_to_str(addr_str, &ipv6->src, sizeof(addr_str)));
    const ipv6_addr_t *cg_addr = ipv6_addr_is_unspecified(&ipv6->src)
                                 ? &nbr_sol->tgt : &ipv6->src;
    ndp_opt_cga_params_t *cga_opt = NULL;
    ndp_opt_sig_t *sig_opt = NULL;
    ndp_opt_nonce_t *nonce_opt = NULL;
    FOREACH_OPT(nbr_sol, opt, icmpv6_len - sizeof(ndp_nbr_sol_t)) {
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
    if (sig_opt && !nonce_opt) {
        DEBUG_NIB_SEND("(NS) signature option requires nonce option\n");
        return -GNRC_SEND_STATUS_BAD_FORMAT;
    }
    if (!cga_opt || !sig_opt) {
        if (!cga_opt) {
            DEBUG_NIB_SEND("(NS) no CGA option\n");
        }
        if (!sig_opt) {
            DEBUG_NIB_SEND("(NS) no signature option\n");
        }
        return (netif->ipv6.send_ctx.mode == GNRC_NIB_IPV6_SEND_MODE_STRICT ? -1 : 1)
               * GNRC_SEND_STATUS_UNSECURED;
    }
    /* The nonce option in a Nbr. Sol. is handled on the fly
       and does not have to be stored anywhere */
    int ret;
    if ((ret = _check_cga_and_signature(netif, cg_addr, ipv6,
                                        (icmpv6_hdr_t *)nbr_sol,
                                        cga_opt, sig_opt))) {
        DEBUG_NIB_SEND("(NS) CGA or signature failure\n");
        return ret;
    }
    return ret;
}

int gnrc_ipv6_nib_send_handle_nbr_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_nbr_adv_t *nbr_adv, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("Iface: %d handle NA from %s\n",
                   netif->pid, ipv6_addr_to_str(addr_str, &ipv6->src, sizeof(addr_str)));
    bool clear_solicited = false;
    ndp_opt_cga_params_t *cga_opt = NULL;
    ndp_opt_sig_t *sig_opt = NULL;
    ndp_opt_nonce_t *nonce_opt = NULL;
    FOREACH_OPT(nbr_adv, opt, icmpv6_len - sizeof(ndp_nbr_adv_t)) {
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
    if (sig_opt && !nonce_opt && !ipv6_addr_is_multicast(&ipv6->dst)) {
        clear_solicited = true;
    }
    int ret;
    if (nbr_adv->flags & NDP_NBR_ADV_FLAGS_S) {
        if (!ipv6_addr_is_multicast(&ipv6->dst)) {
            if (!nonce_opt){
                DEBUG_NIB_SEND("(NA) no nonce option\n");
                return (netif->ipv6.send_ctx.mode == GNRC_NIB_IPV6_SEND_MODE_STRICT ? -1 : 1)
                       * GNRC_SEND_STATUS_UNSECURED;
            }
            if ((ret = _check_nonce(nonce_opt))) {
                DEBUG_NIB_SEND("(NA) nonce mismatch\n");
                return ret;
            }
        }
        if (!nonce_opt) {
            clear_solicited = true;
        }
    }
    if (!cga_opt || !sig_opt) {
        if (!cga_opt) {
            DEBUG_NIB_SEND("(NA) no CGA option\n");
        }
        if (!sig_opt) {
            DEBUG_NIB_SEND("(NA) no signature option\n");
        }
        return (netif->ipv6.send_ctx.mode == GNRC_NIB_IPV6_SEND_MODE_STRICT ? -1 : 1)
               * GNRC_SEND_STATUS_UNSECURED;
    }
    if ((ret = _check_cga_and_signature(netif, &ipv6->src, ipv6,
                                        (icmpv6_hdr_t *)nbr_adv,
                                        cga_opt, sig_opt))) {
        DEBUG_NIB_SEND("(NA) CGA or signature failure\n");
        return ret;
    }
    if (clear_solicited) {
        DEBUG_NIB_SEND("(NA) clear solicited flag\n");
        /* clear solicited flag after signature verification */
        ((ndp_nbr_adv_t *)nbr_adv)->flags &= ~(NDP_NBR_ADV_FLAGS_S);
    }
    return GNRC_SEND_STATUS_OK;
}

int gnrc_ipv6_nib_send_handle_rtr_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_rtr_sol_t *rtr_sol, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("Iface: %d handle RS from %s\n",
                   netif->pid, ipv6_addr_to_str(addr_str, &ipv6->src, sizeof(addr_str)));
    ndp_opt_cga_params_t *cga_opt = NULL;
    ndp_opt_sig_t *sig_opt = NULL;
    ndp_opt_nonce_t *nonce_opt = NULL;
    FOREACH_OPT(rtr_sol, opt, icmpv6_len - sizeof(ndp_rtr_sol_t)) {
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
    if (sig_opt && !nonce_opt) {
        DEBUG_NIB_SEND("(RS) signature option requires nonce option\n");
        return -GNRC_SEND_STATUS_BAD_FORMAT;
    }
    if ((!cga_opt || !sig_opt) && !ipv6_addr_is_unspecified(&ipv6->src)) {
        if (!cga_opt) {
            DEBUG_NIB_SEND("(RS) no CGA option\n");
        }
        if (!sig_opt) {
            DEBUG_NIB_SEND("(RS) no signature option\n");
        }
        return (netif->ipv6.send_ctx.mode == GNRC_NIB_IPV6_SEND_MODE_STRICT ? -1 : 1)
               * GNRC_SEND_STATUS_UNSECURED;
    }
    int ret;
    if((ret = _check_cga_and_signature(netif, &ipv6->src, ipv6,
                                       (icmpv6_hdr_t *)rtr_sol,
                                       cga_opt, sig_opt))) {
        DEBUG_NIB_SEND("(RS) CGA or signature failure\n");
        return ret;
    }
    return ret;
}

int gnrc_ipv6_nib_send_handle_rtr_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_rtr_adv_t *rtr_adv, size_t icmpv6_len,
                                      const gnrc_send_cp_t **rtr_cp)
{
    assert(rtr_cp);
    DEBUG_NIB_SEND("Iface: %d handle RA from %s\n",
                   netif->pid, ipv6_addr_to_str(addr_str, &ipv6->src, sizeof(addr_str)));
    ndp_opt_cga_params_t *cga_opt = NULL;
    ndp_opt_sig_t *sig_opt = NULL;
    ndp_opt_nonce_t *nonce_opt = NULL;
    FOREACH_OPT(rtr_adv, opt, icmpv6_len - sizeof(ndp_rtr_adv_t)) {
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
    int ret;
    if (!ipv6_addr_is_multicast(&ipv6->dst)) {
        if (!nonce_opt) {
            DEBUG_NIB_SEND("(RA) no nonce option\n");
            return (netif->ipv6.send_ctx.mode == GNRC_NIB_IPV6_SEND_MODE_STRICT ? -1 : 1)
                   * GNRC_SEND_STATUS_UNSECURED;
        }
        if ((ret = _check_nonce(nonce_opt))) {
            DEBUG_NIB_SEND("(RA) nonce mismatch\n");
            return ret;
        }
    }
    if (!cga_opt || !sig_opt) {
        if (!cga_opt) {
            DEBUG_NIB_SEND("(RA) no CGA option\n");
        }
        if (!sig_opt) {
            DEBUG_NIB_SEND("(RA) no signature option\n");
        }
        return (netif->ipv6.send_ctx.mode == GNRC_NIB_IPV6_SEND_MODE_STRICT ? -1 : 1)
               * GNRC_SEND_STATUS_UNSECURED;
    }
    if ((ret = _check_cga_and_signature(netif, &ipv6->src, ipv6,
                                        (icmpv6_hdr_t *)rtr_adv,
                                        cga_opt, sig_opt))) {
        DEBUG_NIB_SEND("(RA) CGA or signature failure\n");
        return ret;
    }
    gnrc_send_ident_t rtr_ident;
    gnrc_send_pk_identifier(&rtr_ident,
                            gnrc_send_opt_cga_get_pk(cga_opt),
                            gnrc_send_opt_cga_get_pk_size(cga_opt));
    if (!(*rtr_cp = gnrc_send_get_cp_by_pk_ident(&rtr_ident, NULL))) {
        ipv6_addr_t dst;
        ipv6_addr_set_solicited_nodes(&dst, &ipv6->src);
        _nib_onl_entry_t *nce = _nib_onl_nc_get(&ipv6->src, netif->pid);
        if (nce) {
            uint16_t state = _get_nud_state(nce);
            if (state == GNRC_IPV6_NIB_NC_INFO_NUD_STATE_REACHABLE) {
                dst = ipv6->src;
            }
        }
        const ipv6_addr_t *src;
        if (!(src = gnrc_netif_ipv6_addr_best_src(netif, &dst, true))) {
            src = &ipv6_addr_unspecified;
        }
        gnrc_send_cache_cps_t *ctx;
        /* create a copy and handle the advertisement after CPS and CPA exchange */
        if (!(ctx = gnrc_send_new_cps_ctx(netif, ipv6, &rtr_ident, rtr_adv, icmpv6_len))) {
            DEBUG_NIB_SEND("No CPS context available\n");
            return -GNRC_SEND_STATUS_RESOURCE_NOT_AVAILABLE;
        }
        else if (ctx == CPS_CTX_ALREADY) {
            DEBUG_NIB_SEND("ADD for this router already in progress\n");
            return -GNRC_SEND_STATUS_ADD_IN_PROGRESS;
        }
        if ((ret = gnrc_send_cp_sol_send(netif, src, &dst, SEND_CPS_ALL_COMP, ctx))) {
            DEBUG_NIB_SEND("Could not send CPS\n");
        }
        assert(ctx->timeout_ms == SEND_CPS_RETRY_MS);
        gnrc_send_set_cps_timeout(ctx, SEND_CPS_RETRY_MS);
        gnrc_send_release_cps_ctx(ctx);
        /* Disable sending of RS while ADD is in progress, to not trigegr further RAs */
        _evtimer_del(&netif->ipv6.search_rtr);
        return -GNRC_SEND_STATUS_ADD_IN_PROGRESS;
    }
    return GNRC_SEND_STATUS_OK;
}

void gnrc_send_nonce_get(void *nonce)
{
    uint8_t i = _nonce_cache.counter++ & (ARRAY_SIZE(_nonce_cache.cache) - 1);
    gnrc_send_random_get(NULL, _nonce_cache.cache[i].nonce, sizeof(_nonce_cache.cache[i].nonce));
    _nonce_cache.cache[i].nonce[GNRC_SEND_NONCE_SIZE - 1] &= ~(ARRAY_SIZE(_nonce_cache.cache) - 1);
    _nonce_cache.cache[i].nonce[GNRC_SEND_NONCE_SIZE - 1] |= i;
    memcpy(nonce, _nonce_cache.cache[i].nonce, sizeof(_nonce_cache.cache[i].nonce));
}

gnrc_send_cache_cps_t *gnrc_send_new_cps_ctx(gnrc_netif_t *netif,
                                             const ipv6_hdr_t *rtr_ipv6,
                                             const gnrc_send_ident_t *rtr_ident,
                                             const ndp_rtr_adv_t *rtr_adv,
                                             size_t rtr_adv_len)
{
    gnrc_send_cache_cps_t *new = NULL, *match = NULL;
    mutex_lock(&_cps_cache.mtx);
    for (unsigned i = 0; i < GNRC_SEND_CPS_CACHE_NUMOF; i++) {
        if (!new && !_cps_cache.cache[i].netif) {
            new = &_cps_cache.cache[i]; /* new */
            assert(!new->rtr_adv); /* leak? */
        }
        if (!memcmp(rtr_ident->hash, _cps_cache.cache[i].rtr_ident.hash, sizeof(rtr_ident->hash))) {
            match = &_cps_cache.cache[i];
            break; /* certificate retrieval is already in progress */
        }
    }
    if (match) {
        mutex_unlock(&_cps_cache.mtx);
        return CPS_CTX_ALREADY;
    }
    else if (!new) {
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
        .timeout_ms = SEND_CPS_RETRY_MS,
        .hi_comp = SEND_CPS_ALL_COMP,
    };
    /* mutex remains locked and must be released with gnrc_send_release_cps_ctx() */
    return new;
}

void gnrc_send_acquire_cps_ctx(gnrc_send_cache_cps_t *cps_ctx)
{
    (void)cps_ctx;
    mutex_lock(&_cps_cache.mtx);
}

void gnrc_send_release_cps_ctx(gnrc_send_cache_cps_t *cps_ctx)
{
    (void)cps_ctx;
    mutex_unlock(&_cps_cache.mtx);
}

void gnrc_send_free_cps_ctx(gnrc_send_cache_cps_t *cps_ctx)
{
    _evtimer_del(&cps_ctx->ev_retransmission);
    gnrc_pktbuf_release(cps_ctx->rtr_adv);
    memset(cps_ctx, 0, sizeof(*cps_ctx));
}

void gnrc_send_set_cps_timeout(gnrc_send_cache_cps_t *cps_ctx, uint32_t offset)
{
    if (cps_ctx->netif) {
        _evtimer_add(cps_ctx, GNRC_IPV6_NIB_SEND_CP_SOL, &cps_ctx->ev_retransmission, offset);
    }
}

gnrc_send_cache_cpa_t *gnrc_send_new_cpa_ctx(gnrc_netif_t *netif, const ipv6_addr_t *sol_addr,
                                             uint16_t ident, const gnrc_send_cp_t *cp)
{
    gnrc_send_cache_cpa_t *entry = NULL;
    mutex_lock(&_cpa_cache.mtx);
    for (unsigned i = 0; i < GNRC_SEND_CPA_CACHE_NUMOF; i++) {
        if (!entry && !_cpa_cache.cache[i].netif) {
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
        .hi_comp = cp->num_comp - 1, /* TA is not sent but counts as component */
    };
    /* mutex remains locked and must be released with gnrc_send_release_cpa_ctx()! */
    return entry;
}

void gnrc_send_acquire_cpa_ctx(gnrc_send_cache_cpa_t *cpa_ctx)
{
    (void)cpa_ctx;
    mutex_lock(&_cpa_cache.mtx);
}

void gnrc_send_release_cpa_ctx(gnrc_send_cache_cpa_t *cpa_ctx)
{
    (void)cpa_ctx;
    mutex_unlock(&_cpa_cache.mtx);
}

void gnrc_send_free_cpa_ctx(gnrc_send_cache_cpa_t *cpa_ctx)
{
    _evtimer_del(&cpa_ctx->ev_transmission);
    memset(cpa_ctx, 0, sizeof(*cpa_ctx));
}

void gnrc_send_set_cpa_timeout(gnrc_send_cache_cpa_t *cpa_ctx, uint32_t offset)
{
    if (cpa_ctx->netif) {
        _evtimer_add(cpa_ctx, GNRC_IPV6_NIB_SEND_CP_ADV, &cpa_ctx->ev_transmission, offset);
    }
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
    else if (ta_opt->name_type == NDP_TA_TYPE_CBOR) {
        return gnrc_send_get_ta_by_name_cbor(gnrc_send_opt_ta_get_name(ta_opt),
                                             gnrc_send_opt_ta_get_name_size(ta_opt));
    }
    return NULL;
}

void gnrc_ipv6_nib_send_handle_cp_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_cp_sol_t *cp_sol, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("Iface: %d handle CPS from %s\n",
                   netif->pid, ipv6_addr_to_str(addr_str, &ipv6->src, sizeof(addr_str)));
    int ret = 0;
    const gnrc_send_ta_t *ta = NULL;
    const gnrc_send_cp_t *cp = NULL;
    ndp_opt_ta_t *ta_opt = NULL;
    FOREACH_OPT(cp_sol, opt, icmpv6_len - sizeof(ndp_cp_sol_t)) {
        if (opt->type == NDP_OPT_TRUST_ANCHOR) {
            if (IS_USED(MODULE_GNRC_SEND_C509)) {
                if (!ta_opt && ((ndp_opt_ta_t *)opt)->name_type != NDP_TA_TYPE_CBOR) {
                    DEBUG_NIB_SEND("Warning: First Ta option in CPS is not CBOR encoded.\n");
                }
            }
            else if (!ta_opt && ((ndp_opt_ta_t *)opt)->name_type != NDP_TA_TYPE_DER) {
                DEBUG_NIB_SEND("Warning: First Ta option in CPS is not DER encoded.\n");
            }
            ta_opt = (ndp_opt_ta_t *)opt;
            if ((ta = _handle_ta((const icmpv6_hdr_t *)cp_sol, ta_opt))) {
                if ((cp = gnrc_send_get_cp_by_pk_ident(gnrc_send_get_self_ident(), ta))) {
                    break;
                }
            }
        }
    }
    uint16_t sol_ident = byteorder_ntohs(cp_sol->ident);
    uint16_t sol_comp = byteorder_ntohs(cp_sol->comp);
    gnrc_pktsnip_t *opts = NULL;
    if (!cp) {
        /* If the router is unable to find a path to the requested anchor, it
           SHOULD send an advertisement without any certificates. In this case,
           the router SHOULD include the Trust Anchor options that were
           solicited. */
        DEBUG_NIB_SEND("No CP found\n");
        gnrc_pktsnip_t *hdr;
        FOREACH_OPT(cp_sol, opt, icmpv6_len - sizeof(ndp_cp_sol_t)) {
            if (opt->type == NDP_OPT_TRUST_ANCHOR) {
                ta_opt = (ndp_opt_ta_t *)opt;
                if ((hdr = gnrc_pktbuf_add(opts, ta_opt, ta_opt->len * 8, GNRC_NETTYPE_UNDEF))) {
                    opts = hdr;
                }
            }
        }
        sol_comp = 0;
    }

    gnrc_send_cache_cpa_t *ctx = NULL;
    ipv6_addr_t dst;
    bool to_all;
    if ((to_all = ipv6_addr_is_unspecified(&ipv6->src)) ||
        (to_all = !gnrc_send_check_cpa_rate(&netif->ipv6.send_ctx))) {
        dst = ipv6_addr_all_nodes_link_local;
    }
    else {
        if (gnrc_netif_is_6lo(netif)) {
            dst = ipv6->src;
        }
        else {
            ipv6_addr_set_solicited_nodes(&dst, &ipv6->src);
        }
    }
    if (cp && !(ctx = gnrc_send_new_cpa_ctx(netif, &dst, sol_ident, cp))) {
        DEBUG_NIB_SEND("No CPA context available\n");
        return;
    }
    if (to_all) {
        DEBUG_NIB_SEND("Send CPA to all nodes\n");
        ret = gnrc_send_cp_adv_send(netif,
                                    gnrc_netif_ipv6_addr_best_src(netif, &dst, true), &dst,
                                    sol_comp, 0, cp, ctx, opts);
    }
    else {
        ret = gnrc_send_cp_adv_send(netif,
                                    gnrc_netif_ipv6_addr_best_src(netif, &dst, true), &dst,
                                    sol_comp, sol_ident, cp, ctx, opts);
    }
    if (ctx) {
        if (ret) {
            gnrc_send_free_cpa_ctx(ctx);
        }
        else {
            ctx->hi_comp--;
            if (!to_all) {
                if (cib_full(&netif->ipv6.send_ctx.last_cp_adv.cib)) {
                    cib_get(&netif->ipv6.send_ctx.last_cp_adv.cib);
                }
                netif->ipv6.send_ctx.last_cp_adv.ts[
                    cib_put(&netif->ipv6.send_ctx.last_cp_adv.cib)
                ] = ztimer_now(ZTIMER_MSEC);
            }
            gnrc_send_set_cpa_timeout(ctx, MS_PER_SEC / SEND_CPA_RATE_MAX);
        }
        gnrc_send_release_cpa_ctx(ctx);
    }
}

static gnrc_send_crt_t *_get_cert_by_pk(const gnrc_send_x509_crt_t *x509)
{
    gnrc_send_ident_t pk;
    gnrc_send_pk_identifier(&pk,
                            gnrc_send_x509_get_pubkey(x509),
                            gnrc_send_x509_get_pubkey_size(x509));
    return gnrc_send_get_crt_by_pk_ident(&pk);
}

static gnrc_send_crt_t *_get_cert_parent(const gnrc_send_x509_crt_t *x509)
{
    gnrc_send_ident_t issuer;
    gnrc_send_name_identifier(&issuer,
                              gnrc_send_x509_get_issuer(x509),
                              gnrc_send_x509_get_issuer_size(x509));
    return gnrc_send_get_crt_by_subject_ident(&issuer);
}

static int _handle_cert(const icmpv6_hdr_t *icmpv6, const ndp_opt_cert_t *cert_opt)
{
    int ret;
    uint16_t adv_comp = byteorder_ntohs(((const ndp_cp_adv_t *)icmpv6)->comp);
    uint16_t num_comp = byteorder_ntohs(((const ndp_cp_adv_t *)icmpv6)->all_comp);
    size_t cert_size = (cert_opt->len * 8) - sizeof(*cert_opt);
    size_t size;
    uint8_t *buf;
    gnrc_send_x509_extn_t extn = { .ip_block_numof = 0 };
    if (cert_opt->cert_type != NDP_CERT_TYPE_DER &&
        cert_opt->cert_type != NDP_CERT_TYPE_CBOR) {
        DEBUG_NIB_SEND("Unsupprted certificate type\n");
        return -EINVAL;
    }
    gnrc_send_x509_crt_t *x509 = gnrc_send_x509_acquire(&size, (void **)&buf);
    if (size < cert_size) {
        DEBUG_NIB_SEND("Certificate too big\n");
        gnrc_send_x509_release();
        return -ENOBUFS;
    }
    memcpy(buf, gnrc_send_opt_cert_get_cert(cert_opt), cert_size);
    if (cert_opt->cert_type == NDP_CERT_TYPE_CBOR) {
#if IS_USED(MODULE_GNRC_SEND_C509)
        void *x = ((uint8_t *)buf) + cert_size;
        if ((ret = c509_to_x509(x, size - cert_size, buf, cert_size)) < 0) {
            DEBUG_NIB_SEND("Certificate C509 conversion failed\n");
            return -EINVAL;
        }
        memmove(buf, x, ret);
#endif
    }
    if ((ret = gnrc_send_load_x509(NULL, buf, size, x509, &extn) < 0)) {
        gnrc_send_x509_release();
        return ret;
    }
    if (_get_cert_by_pk(x509)) {
        gnrc_send_x509_release();
        return 0; /* do not consider this an error */
    }
    if (gnrc_send_x509_get_issuer_size(x509) == gnrc_send_x509_get_subject_size(x509) &&
        !memcmp(gnrc_send_x509_get_issuer(x509),
                gnrc_send_x509_get_subject(x509),
                gnrc_send_x509_get_issuer_size(x509))) {
        gnrc_send_x509_release();
        DEBUG_NIB_SEND("Selfsigned certificae not allowed\n");
        return -EPERM; /* self signed certificate not allowed in path */
    }
    buf += gnrc_send_x509_get_size(x509);
    size -= gnrc_send_x509_get_size(x509);
    gnrc_send_crt_t *parent = _get_cert_parent(x509);
    if (!parent) {
        gnrc_send_x509_release();
        DEBUG_NIB_SEND("Issuer certificate not found\n");
        return -ENOENT;
    }
    char path[GNRC_SEND_PATH_MAX];
    if (adv_comp == num_comp - 1) {
        strcpy(path, GNRC_SEND_TA_ROOT"/");
    }
    else {
        strcpy(path, GNRC_SEND_CRT_ROOT"/");
    }
    strcat(path, parent->rel_path);
    if ((ret = gnrc_send_load_x509(path, buf, size, x509 + 1, &extn)) < 0) {
        gnrc_send_x509_release();
        DEBUG_NIB_SEND("Issuer certificate error\n");
        return ret;
    }
    if ((ret = gnrc_send_x509_verify(x509, x509 + 1)) < 0) {
        gnrc_send_x509_release();
        DEBUG_NIB_SEND("Verification error\n");
        return ret;
    }
    gnrc_send_cp_t cp;
    gnrc_send_crt_t crt;
    gnrc_send_crt_init(&crt, x509);
    crt.extn = extn;
    crt.parent = parent;
    if (adv_comp == 1) {
        cp = (gnrc_send_cp_t){ .num_comp = num_comp };
        strcpy(path, GNRC_SEND_CP_ROOT"/");
        _gen_vfs_name(path + strlen(path), sizeof(path) - strlen(path), &crt.pk_ident, NULL);
        strcpy(crt.rel_path, path + strlen(GNRC_SEND_CP_ROOT"/"));
        while (parent->parent) {
            parent = parent->parent;
        }
        if (!(cp.ta = gnrc_send_get_ta_by_subject_ident(&parent->subject_ident))) {
            gnrc_send_x509_release();
            DEBUG_NIB_SEND("Trust anchor not found\n");
            return -ENOENT;
        }
    }
    else {
        strcpy(path, GNRC_SEND_CRT_ROOT"/");
        _gen_vfs_name(path + strlen(path), sizeof(path) - strlen(path), &crt.pk_ident, NULL);
        strcpy(crt.rel_path, path + strlen(GNRC_SEND_CRT_ROOT"/"));
    }
    if ((ret = vfs_file_from_buffer(path,
                                    gnrc_send_x509_get(x509),
                                    gnrc_send_x509_get_size(x509))) < 0) {
        /* I have observed rare cases where the call to vfs_open() fails but a file with 0 bytes was created.
           The file cannot be deleted with vfs_unlink() and this makes it necessary to re-initialize
           a node :( */
        gnrc_send_x509_release();
        DEBUG_NIB_SEND("Could not save certificate\n");
        vfs_unlink(path);
        return ret;
    }
    if (adv_comp == 1) {
        if (!gnrc_send_cp_add(&cp, &crt)) {
            DEBUG_NIB_SEND("No more certificates (paths) can be added.\n");
        }
    }
    else {
        if (!gnrc_send_crt_add(&crt)) {
            DEBUG_NIB_SEND("No more certificates can be added.\n");
        }
    }
    gnrc_send_x509_release();
    return 0;
}

static void _gnrc_ipv6_nib_send_handle_unsolicited_cp_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                                          const ndp_cp_adv_t *cp_adv, size_t icmpv6_len)
{
    (void)netif; (void)ipv6;
    const gnrc_send_ta_t *ta = NULL;
    const ndp_opt_cert_t *crt_opt = NULL;
    uint16_t adv_comp = byteorder_ntohs(cp_adv->comp);
    uint16_t adv_comp_numof = byteorder_ntohs(cp_adv->all_comp);
    if (adv_comp == 0 || adv_comp_numof <= 1) {
        DEBUG_NIB_SEND("CPA without components.\n");
        return;
    }
    else if (adv_comp == adv_comp_numof - 1) {
        FOREACH_OPT(cp_adv, opt, icmpv6_len - sizeof(ndp_cp_adv_t)) {
            if (opt->type == NDP_OPT_TRUST_ANCHOR) {
                if ((ta = _handle_ta((const icmpv6_hdr_t *)cp_adv, (const ndp_opt_ta_t *)opt))) {
                    break;
                }
            }
        }
        if (!ta) {
            DEBUG_NIB_SEND("No or unknown Trust Anchor in first CP component.\n");
            return;
        }
    }
    FOREACH_OPT(cp_adv, opt, icmpv6_len - sizeof(ndp_cp_adv_t)) {
        if (opt->type == NDP_OPT_CERTIFICATE) {
            crt_opt = (const ndp_opt_cert_t *)opt;
            break;
        }
    }
    if (!crt_opt) {
        DEBUG_NIB_SEND("No Certificate found in CPA.\n");
        return;
    }
    if (!_handle_cert((const icmpv6_hdr_t *)cp_adv, crt_opt)) {
        DEBUG_NIB_SEND("Certificate in CPA is valid\n");
    }
}

static inline gnrc_send_cache_cps_t *_get_cps_ctx_by_identifier(uint16_t identifier)
{
    return &_cps_cache.cache[identifier & (ARRAY_SIZE(_cps_cache.cache) - 1)];
}

void gnrc_ipv6_nib_send_handle_cp_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                      const ndp_cp_adv_t *cp_adv, size_t icmpv6_len)
{
    DEBUG_NIB_SEND("Iface: %d handle CPA from %s\n",
                   netif->pid, ipv6_addr_to_str(addr_str, &ipv6->src, sizeof(addr_str)));
    (void)netif;
    const gnrc_send_ta_t *ta = NULL;
    const ndp_opt_cert_t *crt_opt = NULL;
    uint16_t adv_ident = byteorder_ntohs(cp_adv->ident);
    uint16_t adv_comp = byteorder_ntohs(cp_adv->comp);
    uint16_t adv_comp_numof = byteorder_ntohs(cp_adv->all_comp);
    if (!ipv6_addr_equal(&ipv6->dst, &ipv6_addr_all_nodes_link_local)) {
        gnrc_pktsnip_t *rtr_adv = NULL;
        ipv6_hdr_t rtr_ipv6;
        gnrc_send_cache_cps_t *c = _get_cps_ctx_by_identifier(adv_ident);
        assert(adv_ident != 0); /* checked already */
        gnrc_send_acquire_cps_ctx(c);
        if (c->identifier != adv_ident) {
            gnrc_send_release_cps_ctx(c);
            DEBUG_NIB_SEND("CPA identifier misamatch\n");
            return;
        }
        if (c->hi_comp != SEND_CPS_ALL_COMP && adv_comp != 0 && c->hi_comp != adv_comp) {
            gnrc_send_release_cps_ctx(c);
            DEBUG_NIB_SEND("Did not receive the expected component\n");
            return;
        }
        if (adv_comp == 0 || adv_comp_numof <= 1) {
            DEBUG_NIB_SEND("CPA without components.\n"
                           "Router does not share common Trust Anchors. Aborting ADD.\n");
            gnrc_send_free_cps_ctx(c);
            gnrc_send_release_cps_ctx(c);
            /* re-enable sending of RS */
            _handle_search_rtr(netif);
            return;
        }
        else if (adv_comp == adv_comp_numof - 1) {
            FOREACH_OPT(cp_adv, opt, icmpv6_len - sizeof(ndp_cp_adv_t)) {
                if (opt->type == NDP_OPT_TRUST_ANCHOR) {
                    if ((ta = _handle_ta((const icmpv6_hdr_t *)cp_adv, (const ndp_opt_ta_t *)opt))) {
                        break;
                    }
                }
            }
            if (!ta) {
                DEBUG_NIB_SEND("No or unknown Trust Anchor in first CP component. Aborting ADD.\n");
                gnrc_send_free_cps_ctx(c);
                gnrc_send_release_cps_ctx(c);
                /* re-enable sending of RS */
                _handle_search_rtr(netif);
                return;
            }
            c->ta = ta;
            c->hi_comp = adv_comp;
            c->num_comp = adv_comp_numof;
        }
        else if (c->num_comp != adv_comp_numof) {
            gnrc_send_release_cps_ctx(c);
            DEBUG_NIB_SEND("CP number of components changed\n");
            return;
        }
        FOREACH_OPT(cp_adv, opt, icmpv6_len - sizeof(ndp_cp_adv_t)) {
            if (opt->type == NDP_OPT_CERTIFICATE) {
                crt_opt = (const ndp_opt_cert_t *)opt;
                break;
            }
        }
        if (!crt_opt) {
            DEBUG_NIB_SEND("No Certificate found in CPA\n");
            gnrc_send_release_cps_ctx(c);
            return;
        }
        if (!_handle_cert((const icmpv6_hdr_t *)cp_adv, crt_opt)) {
            DEBUG_NIB_SEND("Certificate in CPA is valid\n");
            _evtimer_del(&c->ev_retransmission);
            c->timeout_ms = 0;
            if (c->hi_comp > 1) {
                /* this event triggers a CP solicitation for single components */
                DEBUG_NIB_SEND("add CPS event to retrieve eventually missing fragments %p\n",
                               (void *)&c->ev_retransmission);
                c->hi_comp--;
                gnrc_send_set_cps_timeout(c, SEND_CPS_RETRY_FRAGMENTS_MS);
            }
            else {
                /* received all certificate components,
                prepare to handle the router advertisement finally */
                rtr_adv = c->rtr_adv;
                rtr_ipv6 = c->rtr_ipv6;
                gnrc_pktbuf_hold(rtr_adv, 1); /* handle below */
                gnrc_send_free_cps_ctx(c); /* releases saved RA */
            }
        }
        else {
            DEBUG_NIB_SEND("Certificate error. Aborting ADD.\n");
            gnrc_send_free_cps_ctx(c);
            gnrc_send_release_cps_ctx(c);
            /* re-enable sending of RS */
            _handle_search_rtr(netif);
            return;
        }
        gnrc_send_release_cps_ctx(c);
        if (rtr_adv) {
            gnrc_pktsnip_t *pkt_ipv6 = gnrc_pktbuf_add(rtr_adv, &rtr_ipv6,
                                                       sizeof(rtr_ipv6), GNRC_NETTYPE_IPV6);
            if (!pkt_ipv6) {
                DEBUG_NIB_SEND("No space in packet buffer. Discarding RA\n");
                gnrc_pktbuf_release(rtr_adv);
                _handle_search_rtr(netif);
                return;
            }
            if (gnrc_pktbuf_merge(pkt_ipv6) != 0) {
                DEBUG_NIB_SEND("No space to merge packets in packet buffer. Discarding RA\n");
                gnrc_pktbuf_release(rtr_adv);
                gnrc_pktbuf_release(pkt_ipv6);
                _handle_search_rtr(netif);
                return;
            }
            /* Do not handle CP advertisement immediately, due to stack size */
            msg_t m_ra = { .type = GNRC_NETAPI_MSG_TYPE_RCV, .content.ptr = pkt_ipv6 };
            msg_send_to_self(&m_ra);
        }
    }
    else {
        _gnrc_ipv6_nib_send_handle_unsolicited_cp_adv(netif, ipv6, cp_adv, icmpv6_len);
    }
}

void gnrc_ipv6_nib_send_handle_snd_cp_sol(gnrc_send_cache_cps_t *cps_ctx)
{
    int ret;
    gnrc_send_acquire_cps_ctx(cps_ctx);
    gnrc_netif_t *netif = cps_ctx->netif;
    if (!netif) {
        /* ctx has been deleted after the message already has been posted from ISR */
        gnrc_send_release_cps_ctx(cps_ctx);
        return;
    }
    if (cps_ctx->timeout_ms * 2 > SEND_CPS_RETRY_MAX_MS) {
        DEBUG_NIB_SEND("ADD process timed out. Aborting ADD.\n");
        gnrc_send_free_cps_ctx(cps_ctx);
        gnrc_send_release_cps_ctx(cps_ctx);
        /* re-enable sending of RS */
        _handle_search_rtr(netif);
        return;
    }
    gnrc_netif_acquire(cps_ctx->netif);
    if ((ret = gnrc_send_cp_sol_send(cps_ctx->netif, NULL, &cps_ctx->rtr_ipv6.src,
                                     cps_ctx->hi_comp, cps_ctx)) < 0) {
        DEBUG_NIB_SEND("Could not send CPS retransmission.\n");
    }
    gnrc_netif_release(cps_ctx->netif);
    cps_ctx->timeout_ms *= 2;
    uint32_t timeout = !cps_ctx->timeout_ms ? CONFIG_SEND_CPS_RETRY_MS : cps_ctx->timeout_ms;
    gnrc_send_set_cps_timeout(cps_ctx, timeout);
    gnrc_send_release_cps_ctx(cps_ctx);
}

void gnrc_ipv6_nib_send_handle_snd_cp_adv(gnrc_send_cache_cpa_t *cpa_ctx)
{
    int ret;
    gnrc_send_acquire_cpa_ctx(cpa_ctx);
    if (!cpa_ctx->netif) {
        /* ctx has been deleted after the message already has been posted from ISR */
        gnrc_send_release_cpa_ctx(cpa_ctx);
        return;
    }
    gnrc_netif_acquire(cpa_ctx->netif);
    if (cpa_ctx->hi_comp < 1) {
        DEBUG_NIB_SEND("ADD process finished.\n");
        gnrc_netif_release(cpa_ctx->netif);
        gnrc_send_free_cpa_ctx(cpa_ctx);
        gnrc_send_release_cpa_ctx(cpa_ctx);
        return;
    }
    bool to_all = ipv6_addr_equal(&cpa_ctx->sol_addr, &ipv6_addr_all_nodes_link_local) ||
                  !gnrc_send_check_cpa_rate(&cpa_ctx->netif->ipv6.send_ctx);
    if ((ret = gnrc_send_cp_adv_send(cpa_ctx->netif, NULL, &cpa_ctx->sol_addr,
                                     cpa_ctx->hi_comp, to_all ? 0 : cpa_ctx->identifier,
                                     cpa_ctx->cp, cpa_ctx, NULL))) {
        DEBUG_NIB_SEND("Could not send CPA component %u.\n",
                       cpa_ctx->hi_comp);
    }
    gnrc_netif_release(cpa_ctx->netif);
    cpa_ctx->hi_comp--;
    if (!to_all) {
        if (cib_full(&cpa_ctx->netif->ipv6.send_ctx.last_cp_adv.cib)) {
            cib_get(&cpa_ctx->netif->ipv6.send_ctx.last_cp_adv.cib);
        }
        cpa_ctx->netif->ipv6.send_ctx.last_cp_adv.ts[
            cib_put(&cpa_ctx->netif->ipv6.send_ctx.last_cp_adv.cib)
        ] = ztimer_now(ZTIMER_MSEC);
    }
    gnrc_send_set_cpa_timeout(cpa_ctx, MS_PER_SEC / SEND_CPA_RATE_MAX);
    gnrc_send_release_cpa_ctx(cpa_ctx);
}

int gnrc_ipv6_nib_send_check_prefix(const ipv6_addr_t *pfx, uint8_t len,
                                    const gnrc_send_ip_restrict_t *ip_res)
{
    x509_ip_address_block_t ip[ARRAY_SIZE(ip_res->range)] = { 0 };
    for (unsigned i = 0; i < ip_res->numof; i++) {
        ip[i].is_range = true;
        ip[i].address_or_range.u_range.min = ip_res->range[i].lower;
        ip[i].address_or_range.u_range.max = ip_res->range[i].upper;
    }
    x509_ip_address_block_t ip_sub = { .is_range = false };
    ip_sub.address_or_range.u_prefix.addr = *pfx;
    ip_sub.address_or_range.u_prefix.len = len;
    x509_ip_prefix_to_range(&ip_sub);
    for (unsigned i = 0; i < ip_res->numof; i++) {
        if (0 == x509_ip_is_subrange(NULL, &ip[i], &ip_sub)) {
            /* subrange of any restriction range is sufficient */
            return GNRC_SEND_STATUS_OK;
        }
    }
    return !ip_res->numof ? GNRC_SEND_STATUS_OK : GNRC_SEND_STATUS_UNSECURED;
}

void gnrc_ipv6_nib_send_get_ip_restriction(gnrc_send_ip_restrict_t *ip_res, const gnrc_send_cp_t *cp)
{
    ip_res->range->lower = cp->crt->extn.ip_block->address_or_range.u_range.min;
    ip_res->range->upper = cp->crt->extn.ip_block->address_or_range.u_range.max;
    ip_res->numof = cp->crt->extn.ip_block_numof;
}
