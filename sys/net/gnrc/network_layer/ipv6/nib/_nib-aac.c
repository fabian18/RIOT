/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Martine Lenders <m.lenders@fu-berlin.de>
 */

#include <kernel_defines.h>
#include <stdbool.h>

#include "log.h"
#include "luid.h"
#include "net/gnrc/ipv6/nib.h"
#include "net/gnrc/netif/internal.h"

#include "_nib-6ln.h"
#include "_nib-arsm.h"
#include "_nib-aac.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/* In practice, CGAs are only used with SEND */
static inline int _cga_generate(ipv6_addr_t *pfx,
                                ipv6_cga_parameters_t *params,
                                gnrc_netif_ipv6_t *netif)
{
    (void)pfx; (void)params; (void)netif;
    return -1;
}

static inline void _call_bootstrap(gnrc_netif_t *netif, const ipv6_addr_t *addr)
{
    if (!ipv6_addr_is_link_local(addr) && gnrc_netif_is_6lbr(netif)) {
        (void)gnrc_ipv6_nib_abr_add(addr);
    }
    gnrc_ipv6_aac_bootstrap_t bootstrap = NULL;
    uint8_t pfx_len = 0;
    gnrc_ipv6_nib_pl_t ple;
    void *state = NULL;
    while (gnrc_ipv6_nib_pl_iter(netif->pid, &state, &ple)) {
        if (ple.pfx_len >= pfx_len && ipv6_addr_match_prefix(addr, &ple.pfx) >= ple.pfx_len) {
            bootstrap = ple.bootstrap;
            pfx_len = ple.pfx_len;
        }
    }
    if (bootstrap) {
        bootstrap(&netif->ipv6, addr, pfx_len);
    }
}

#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_6LN) || IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_SLAAC)
static char addr_str[IPV6_ADDR_MAX_STR_LEN];

void _auto_configure_addr(gnrc_netif_t *netif, const ipv6_addr_t *pfx,
                          uint8_t pfx_len)
{
    ipv6_addr_t addr = IPV6_ADDR_UNSPECIFIED;
    int idx;
    gnrc_netif_acquire(netif);
    uint8_t flags = (gnrc_netif_is_6ln(netif) && ipv6_addr_is_link_local(pfx)) ||
                    gnrc_netif_is_6lbr(netif)
                    ? GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID
                    : GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_TENTATIVE;

#if !IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_SLAAC)
    if (!gnrc_netif_is_6ln(netif)) {
        LOG_WARNING("SLAAC not activated; will not auto-configure IPv6 address "
                         "for interface %u.\n"
                    "    Use CONFIG_GNRC_IPV6_NIB_SLAAC=1 to activate.\n",
                    netif->pid);
        gnrc_netif_release(netif);
        return;
    }
#endif
    if (!(netif->flags & GNRC_NETIF_FLAGS_HAS_L2ADDR)) {
        DEBUG("nib: interface %i has no link-layer addresses\n", netif->pid);
        gnrc_netif_release(netif);
        return;
    }
    DEBUG("nib: add address based on %s/%u automatically to interface %u\n",
          ipv6_addr_to_str(addr_str, pfx, sizeof(addr_str)),
          pfx_len, netif->pid);
    if (gnrc_netif_ipv6_get_iid(netif, (eui64_t *)&addr.u64[1]) < 0) {
        DEBUG("nib: Can't get IID on interface %u\n", netif->pid);
        gnrc_netif_release(netif);
        return;
    }
    ipv6_addr_init_prefix(&addr, pfx, pfx_len);
    if ((idx = gnrc_netif_ipv6_addr_idx(netif, &addr)) < 0) {
        if ((idx = gnrc_netif_ipv6_addr_add_internal(netif, &addr, pfx_len, flags,
                                                     GNRC_NETIF_IPV6_ADDR_PRIV_NONE)) < 0) {
            DEBUG("nib: Can't add link-local address on interface %u\n",
                  netif->pid);
            gnrc_netif_release(netif);
            return;
        }
        if ((netif->ipv6.addrs_flags[idx] & GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_MASK)
            != GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID) {
            if (gnrc_netif_is_6ln(netif) && !gnrc_netif_is_6lbr(netif)) {
                _handle_rereg_address(&netif->ipv6.addrs[idx]);
            }
        }
        else {
            _handle_valid_addr(gnrc_netif_ipv6_set_addr_index(&netif->ipv6.addrs[idx], idx));
        }
    }
    gnrc_netif_release(netif);
}
#endif  /* CONFIG_GNRC_IPV6_NIB_6LN || CONFIG_GNRC_IPV6_NIB_SLAAC */

#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_SLAAC)
static bool _try_l2addr_reconfiguration(gnrc_netif_t *netif)
{
    uint8_t hwaddr[GNRC_NETIF_L2ADDR_MAXLEN];
    uint16_t hwaddr_len;

    if (gnrc_netapi_get(netif->pid, NETOPT_SRC_LEN, 0, &hwaddr_len,
                        sizeof(hwaddr_len)) < 0) {
        return false;
    }
    luid_get(hwaddr, hwaddr_len);
#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_6LN)
    if (hwaddr_len == IEEE802154_LONG_ADDRESS_LEN) {
        if (gnrc_netapi_set(netif->pid, NETOPT_ADDRESS_LONG, 0, hwaddr,
                            hwaddr_len) < 0) {
            return false;
        }
    }
    else
#endif
    if (gnrc_netapi_set(netif->pid, NETOPT_ADDRESS, 0, hwaddr,
                        hwaddr_len) < 0) {
        return false;
    }
    return true;
}

static bool _try_addr_reconfiguration(gnrc_netif_t *netif)
{
    eui64_t orig_iid;
    bool remove_old = false, hwaddr_reconf;

    if (gnrc_netif_ipv6_get_iid(netif, &orig_iid) > 0) {
        remove_old = true;
    }
    /* seize netif to netif thread since _try_l2addr_reconfiguration uses
     * gnrc_netapi_get()/gnrc_netapi_set(). Since these are synchronous this is
     * safe */
    gnrc_netif_release(netif);
    /* reacquire netif for IPv6 address reconfiguraton */
    hwaddr_reconf = _try_l2addr_reconfiguration(netif);
    gnrc_netif_acquire(netif);
    if (hwaddr_reconf) {
        if (remove_old) {
            for (unsigned i = 0; i < CONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF; i++) {
                ipv6_addr_t *addr = &netif->ipv6.addrs[i];
                if (addr->u64[1].u64 == orig_iid.uint64.u64) {
                    gnrc_netif_ipv6_addr_remove_internal(netif, addr);
                }
            }
        }
        DEBUG("nib: Changed hardware address, due to DAD\n");
        _auto_configure_addr(netif, &ipv6_addr_link_local_prefix, 64U);
    }
    return hwaddr_reconf;
}

void _remove_tentative_addr(gnrc_netif_t *netif, const ipv6_addr_t *addr)
{
    DEBUG("nib: other node has TENTATIVE address %s assigned "
          "=> removing that address\n",
          ipv6_addr_to_str(addr_str, addr, sizeof(addr_str)));
    gnrc_netif_ipv6_addr_remove_internal(netif, addr);

    if (!ipv6_addr_is_link_local(addr) ||
        !_try_addr_reconfiguration(netif)) {
        /* Cannot use target address as personal address and can
         * not change hardware address to retry SLAAC => use purely
         * DHCPv6 instead */
        if (IS_USED(MODULE_DHCPV6_CLIENT_IA_NA)) {
            netif->ipv6.aac_mode &= ~GNRC_NETIF_AAC_AUTO;
            netif->ipv6.aac_mode |= GNRC_NETIF_AAC_DHCP;
            dhcpv6_client_req_ia_na(netif->pid);
        }
        else {
            DEBUG("nib: would set interface %i to DHCPv6, "
                  "but DHCPv6 is not provided", netif->pid);
        }
    }
}
#endif

void _auto_configure_cga(gnrc_netif_t *netif,
                         const ipv6_addr_t *pfx, uint8_t pfx_len)
{
    (void)netif; (void)pfx; (void)pfx_len;
#if IS_USED(MODULE_IPV6_CGA)
    ipv6_cga_parameters_t *params = NULL;
    ipv6_addr_t addr = IPV6_ADDR_UNSPECIFIED;
    int idx, p_idx;
    uint8_t flags = gnrc_netif_is_6lbr(netif)
                    ? GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID
                    : GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_TENTATIVE;
    ipv6_addr_init_prefix(&addr, pfx, pfx_len);
    gnrc_ipv6_cga_ctx_t *cga_ctx = gnrc_netif_ipv6_get_cga_ctx(&netif->ipv6);

    gnrc_netif_acquire(netif);
    for (int i = 0; i < (int)ARRAY_SIZE(netif->ipv6.addrs); i++) {
        if (ipv6_addr_match_prefix(&netif->ipv6.addrs[i], pfx) >= pfx_len &&
            netif->ipv6.addrs_priv[i] == GNRC_NETIF_IPV6_ADDR_PRIV_CGA) {
            gnrc_netif_release(netif);
            return;
        }
    }
    for (p_idx = 0; p_idx < (int)ARRAY_SIZE(cga_ctx->addr_idx); p_idx++) {
        if (cga_ctx->addr_idx[p_idx] < 0) {
            params = &cga_ctx->params[p_idx];
            break;
        }
    }
    if (!params || _cga_generate(&addr, params, &netif->ipv6)) {
        gnrc_netif_release(netif);
        return;
    }
    if ((idx = gnrc_netif_ipv6_addr_idx(netif, &addr)) < 0) {
        DEBUG("nib: add CGA based on %s/%u automatically to interface %u\n",
              ipv6_addr_to_str(addr_str, pfx, sizeof(addr_str)), pfx_len, netif->pid);
        if ((idx = gnrc_netif_ipv6_addr_add_internal(netif, &addr, pfx_len, flags,
                                                    GNRC_NETIF_IPV6_ADDR_PRIV_CGA)) < 0) {
            memset(params, 0, sizeof(*params));
            gnrc_netif_release(netif);
            return;
        }
        cga_ctx->addr_idx[p_idx] = idx;
        if ((netif->ipv6.addrs_flags[idx] & GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_MASK)
            != GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID) {
            if (gnrc_netif_is_6ln(netif) && !gnrc_netif_is_6lbr(netif)) {
                _handle_rereg_address(&netif->ipv6.addrs[idx]);
            }
        }
        else {
            _handle_valid_addr(gnrc_netif_ipv6_set_addr_index(&netif->ipv6.addrs[idx], idx));
        }
    }
    gnrc_netif_release(netif);
#endif
}

void _auto_reconfigure_cga(gnrc_netif_t *netif, const ipv6_addr_t *address)
{
    (void)netif; (void)address;
#if IS_USED(MODULE_IPV6_CGA)
    ipv6_addr_t addr = *address;
    int idx, p_idx;
    gnrc_ipv6_cga_ctx_t *cga_ctx = gnrc_netif_ipv6_get_cga_ctx(&netif->ipv6);

    gnrc_netif_acquire(netif);
    if ((idx = gnrc_netif_ipv6_addr_idx(netif, &addr)) < 0 ||
        netif->ipv6.addrs_priv[idx] != GNRC_NETIF_IPV6_ADDR_PRIV_CGA) {
        gnrc_netif_release(netif);
        return;
    }
    if (gnrc_netif_ipv6_addr_dad_trans(netif, idx) <= 0) {
        gnrc_netif_release(netif);
        return;
    }
    for (p_idx = 0; p_idx < (int)ARRAY_SIZE(cga_ctx->params); p_idx++) {
        if ((idx = cga_ctx->addr_idx[p_idx]) >= 0 &&
            !memcmp(address, &netif->ipv6.addrs[idx], sizeof(*address))) {
            break;
        }
    }
    if (p_idx >= (int)ARRAY_SIZE(cga_ctx->params)) {
        gnrc_netif_release(netif);
        return;
    }
    cga_ctx->params[p_idx].collision_count++;
    if (_cga_generate(&addr, &cga_ctx->params[p_idx], &netif->ipv6)) {
        memset(&cga_ctx->params[p_idx], 0, sizeof(cga_ctx->params[p_idx]));
        cga_ctx->addr_idx[p_idx] = -1;
        gnrc_netif_release(netif);
        return;
    }
    gnrc_netif_ipv6_addr_remove_internal(netif, &addr);
    if ((idx = gnrc_netif_ipv6_addr_add_internal(netif, &addr, 64,
                                                 GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_TENTATIVE,
                                                 GNRC_NETIF_IPV6_ADDR_PRIV_CGA)) < 0) {
        memset(&cga_ctx->params[p_idx], 0, sizeof(cga_ctx->params[p_idx]));
        cga_ctx->addr_idx[p_idx] = -1;
        gnrc_netif_release(netif);
        return;
    }
    cga_ctx->addr_idx[p_idx] = idx;
    gnrc_netif_release(netif);
#endif
}

#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_SLAAC) || IS_USED(MODULE_IPV6_CGA)
void _handle_dad(ipv6_addr_t *addr)
{
    int idx = gnrc_netif_ipv6_get_addr_index(&addr);
    assert(idx < CONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF);
    gnrc_netif_ipv6_t *netif_ip = container_of(addr, gnrc_netif_ipv6_t, addrs[idx]);
    gnrc_netif_t *netif = container_of(netif_ip, gnrc_netif_t, ipv6);
    gnrc_netif_acquire(netif);
    if (!ipv6_addr_is_unspecified(addr) &&
        (idx = gnrc_netif_ipv6_addr_idx(netif, addr)) >= 0) {
        uint8_t dad;
        if ((dad = gnrc_netif_ipv6_addr_dad_trans(netif, idx))) {
            if (dad == GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_TENTATIVE) {
                netif->ipv6.addrs_flags[idx] &= ~GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_TENTATIVE;
            }
            ipv6_addr_t sol_nodes;
            ipv6_addr_set_solicited_nodes(&sol_nodes, addr);
            DEBUG("nib: DAD %s\n", ipv6_addr_to_str(addr_str, addr, sizeof(addr_str)));
            netif->ipv6.addrs_flags[idx]++;
            _snd_ns(addr, netif, &ipv6_addr_unspecified, &sol_nodes);
            _evtimer_add(gnrc_netif_ipv6_set_addr_index(addr, idx),
                         GNRC_IPV6_NIB_VALID_ADDR,
                         &netif->ipv6.addrs_timers[idx],
                         netif->ipv6.retrans_time);
        }
    }
    gnrc_netif_release(netif);
}

void _handle_valid_addr(ipv6_addr_t *addr)
{
    int idx = gnrc_netif_ipv6_get_addr_index(&addr);
    assert(idx < CONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF);
    gnrc_netif_ipv6_t *netif_ip = container_of(addr, gnrc_netif_ipv6_t, addrs[idx]);
    gnrc_netif_t *netif = container_of(netif_ip, gnrc_netif_t, ipv6);
    gnrc_netif_acquire(netif);
    if (!ipv6_addr_is_unspecified(addr) &&
        (idx = gnrc_netif_ipv6_addr_idx(netif, addr)) >= 0) {
        DEBUG("nib: validating address %s (idx: %d, netif: %d)\n",
              ipv6_addr_to_str(addr_str, addr, sizeof(addr_str)), idx,
              netif->pid);
        netif->ipv6.addrs_flags[idx] &= ~GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_MASK;
        netif->ipv6.addrs_flags[idx] |= GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID;
        gnrc_netif_ipv6_bus_post(netif, GNRC_IPV6_EVENT_ADDR_VALID, &netif->ipv6.addrs[idx]);
        _call_bootstrap(netif, addr);
    }
    gnrc_netif_release(netif);
}
#endif
/** @} */
