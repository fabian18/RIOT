/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <assert.h>
#include <string.h>
#include <kernel_defines.h>

#include "net/gnrc/ipv6/nib.h"
#include "net/gnrc/netif/internal.h"
#include "net/gnrc/sixlowpan/nd.h"

#include "_nib-6lr.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_6LR)

static char addr_str[IPV6_ADDR_MAX_STR_LEN];

static void _update_nce_ar_state(gnrc_netif_t *netif,
                                 const sixlowpan_nd_opt_ar_t *aro,
                                 _nib_onl_entry_t *nce)
{
    assert(nce);
    memcpy(&nce->eui64, &aro->eui64, sizeof(aro->eui64));
    _evtimer_add(nce, GNRC_IPV6_NIB_ADDR_REG_TIMEOUT,
                 &nce->addr_reg_timeout,
                 byteorder_ntohs(aro->ltime) * SEC_PER_MIN * MS_PER_SEC);
    if (IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_ARSM)) {
        switch (_get_nud_state(nce)) {
        case GNRC_IPV6_NIB_NC_INFO_NUD_STATE_UNMANAGED:
        case GNRC_IPV6_NIB_NC_INFO_NUD_STATE_REACHABLE:
            /* nothing to do */
            break;
        default:
            assert(netif != NULL);
            _evtimer_del(&nce->nud_timeout);
            _set_nud_state(netif, nce,
                           GNRC_IPV6_NIB_NC_INFO_NUD_STATE_STALE);
            break;
        }
    }
    _set_ar_state(nce,
                  GNRC_IPV6_NIB_NC_INFO_AR_STATE_REGISTERED);
    DEBUG("nib: Successfully registered %s\n",
          ipv6_addr_to_str(addr_str, &nce->ipv6, sizeof(addr_str)));
}

uint8_t _reg_addr_upstream(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                           const icmpv6_hdr_t *icmpv6,
                           const sixlowpan_nd_opt_ar_t *aro,
                           const ndp_opt_t *sl2ao, _nib_onl_entry_t *nce)
{
    if (!ipv6_addr_is_unspecified(&ipv6->src) && (sl2ao != NULL)) {
        DEBUG("nib: Trying to register %s with EUI-64 "
              "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
              ipv6_addr_to_str(addr_str, &ipv6->src, sizeof(addr_str)),
              aro->eui64.uint8[0], aro->eui64.uint8[1], aro->eui64.uint8[2],
              aro->eui64.uint8[3], aro->eui64.uint8[4], aro->eui64.uint8[5],
              aro->eui64.uint8[6], aro->eui64.uint8[7]);
        if ((nce == NULL) || !(nce->mode & _NC) ||
            (_get_ar_state(nce) == GNRC_IPV6_NIB_NC_INFO_AR_STATE_GC) ||
            (!memcmp(&nce->eui64, &aro->eui64, sizeof(aro->eui64)))) {
            /* no duplicate */
            if (aro->ltime.u16 != 0) {
                if (nce) {
                    uint16_t ar = _get_ar_state(nce);
                    if (ar == GNRC_IPV6_NIB_NC_INFO_AR_STATE_TENTATIVE ||
                        ar == GNRC_IPV6_NIB_NC_INFO_AR_STATE_REGISTERED) {
                        /* nce may be TENATATIVE or REGISTERED,
                           where a TENTATICE NCE may have been created from a received RS */
                        _evtimer_del(&nce->addr_reg_timeout);
                    }
                    _handle_sl2ao(netif, ipv6, icmpv6, sl2ao);
                    if (ar == GNRC_IPV6_NIB_NC_INFO_AR_STATE_TENTATIVE) {
                        if (IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_MULTIHOP_DAD)) {
                            if (gnrc_netif_is_6lr(netif) && !gnrc_netif_is_6lbr(netif)) {
                                _evtimer_add(nce, GNRC_IPV6_NIB_ADDR_REG_TIMEOUT,
                                             &nce->addr_reg_timeout,
                                             SIXLOWPAN_ND_TENTATIVE_NCE_SEC_LTIME * MS_PER_SEC);
                                /* TODO: send DAR to upstream */
                                return _ADDR_REG_STATUS_TENTATIVE;
                            }
                        }
                    }
                }
                else {
                    _handle_sl2ao(netif, ipv6, icmpv6, sl2ao);
                    if (!(nce = _nib_onl_get(&ipv6->src, netif->pid))) {
                        /* NIB is full */
                        return SIXLOWPAN_ND_STATUS_NC_FULL;
                    }
                    if (IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_MULTIHOP_DAD)) {
                        if (gnrc_netif_is_6lr(netif) && !gnrc_netif_is_6lbr(netif)) {
                            _set_ar_state(nce, GNRC_IPV6_NIB_NC_INFO_AR_STATE_TENTATIVE);
                            _evtimer_add(nce, GNRC_IPV6_NIB_ADDR_REG_TIMEOUT,
                                         &nce->addr_reg_timeout,
                                         SIXLOWPAN_ND_TENTATIVE_NCE_SEC_LTIME * MS_PER_SEC);
                            /* TODO: send DAR to upstream */
                            return _ADDR_REG_STATUS_TENTATIVE;
                        }
                    }
                }
                _update_nce_ar_state(netif, aro, nce);
                return SIXLOWPAN_ND_STATUS_SUCCESS;
            }
            else if (nce) {
                _nib_nc_remove(nce);
                return SIXLOWPAN_ND_STATUS_SUCCESS;
            }
        }
        else if (_get_ar_state(nce) != GNRC_IPV6_NIB_NC_INFO_AR_STATE_GC) {
            if (IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_MULTIHOP_DAD)) {
                if (gnrc_netif_is_6lr(netif) && !gnrc_netif_is_6lbr(netif)) {
                    /* If such a Tentative NCE exists, then the 6LR SHOULD silently
                       ignore the ARO, thereby relying on the host retransmitting the ARO */
                    if (_get_ar_state(nce) == GNRC_IPV6_NIB_NC_INFO_AR_STATE_TENTATIVE) {
                        return _ADDR_REG_STATUS_IGNORE;
                    }
                }
            }
            DEBUG("nib: Could not register %s, duplicate entry with EUI-64 "
                "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
                ipv6_addr_to_str(addr_str, &ipv6->src, sizeof(addr_str)),
                nce->eui64.uint8[0], nce->eui64.uint8[1], nce->eui64.uint8[2],
                nce->eui64.uint8[3], nce->eui64.uint8[4], nce->eui64.uint8[5],
                nce->eui64.uint8[6], nce->eui64.uint8[7]);
            return SIXLOWPAN_ND_STATUS_DUP;
        }
    }
    return _ADDR_REG_STATUS_IGNORE;
}

gnrc_pktsnip_t *_copy_and_handle_aro(gnrc_netif_t *netif,
                                     const ipv6_hdr_t *ipv6,
                                     const ndp_nbr_sol_t *nbr_sol,
                                     const sixlowpan_nd_opt_ar_t *aro,
                                     const ndp_opt_t *sl2ao)
{
    gnrc_pktsnip_t *reply_aro = NULL;
    assert(aro);
    uint8_t status = _handle_aro(netif, ipv6, (icmpv6_hdr_t *)nbr_sol, aro,
                                 sl2ao, _nib_onl_nc_get(&ipv6->src, netif->pid));

    if (status != _ADDR_REG_STATUS_TENTATIVE &&
        status != _ADDR_REG_STATUS_IGNORE) {
        reply_aro = gnrc_sixlowpan_nd_opt_ar_build(status,
                                                   byteorder_ntohs(aro->ltime),
                                                   (eui64_t *)&aro->eui64,
                                                   NULL);
        if (reply_aro == NULL) {
            DEBUG("nib: No space left in packet buffer. Not replying NS");
        }
    }
#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_MULTIHOP_DAD)
    else if (status == _ADDR_REG_STATUS_TENTATIVE) {
        DEBUG("nib: Address was marked TENTATIVE => not replying NS, "
              "waiting for DAC\n");
    }
#endif  /* CONFIG_GNRC_IPV6_NIB_MULTIHOP_DAD */
    return reply_aro;
}
#else  /* CONFIG_GNRC_IPV6_NIB_6LR */
typedef int dont_be_pedantic;
#endif /* CONFIG_GNRC_IPV6_NIB_6LR */

/** @} */
