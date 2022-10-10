/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup net_gnrc_netif
 * @{
 *
 * @file
 * @brief   IPv6 definitions for @ref net_gnrc_netif
 *
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */
#ifndef NET_GNRC_NETIF_IPV6_H
#define NET_GNRC_NETIF_IPV6_H

#include <assert.h>

#include "modules.h"
#include "evtimer_msg.h"
#include "net/ipv6/addr.h"
#include "net/ipv6/cga.h"
#ifdef MODULE_GNRC_IPV6_NIB
#include "net/gnrc/ipv6/nib/conf.h"
#endif
#include "net/gnrc/netapi.h"
#include "net/gnrc/netif/conf.h"
#ifdef MODULE_NETSTATS_IPV6
#include "net/netstats.h"
#endif
#if IS_USED(MODULE_GNRC_SEND)
#include "net/gnrc/send.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    IPv6 unicast and anycast address flags
 * @anchor  net_gnrc_netif_ipv6_addrs_flags
 * @{
 */
/**
 * @brief   Mask for the address' state
 */
#define GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_MASK             (0x1fU)

/**
 * @brief   Tentative states (with encoded DAD retransmissions)
 *
 * The retransmissions of DAD transmits can be decoded from this state by
 * applying it as a mask to the [flags](gnrc_netif_ipv6_t::addrs_flags) of the
 * address.
 */
#define GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_TENTATIVE        (0x07U)

/**
 * @brief   Deprecated address state (still valid, but not preferred)
 */
#define GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_DEPRECATED       (0x08U)

/**
 * @brief   Valid address state
 */
#define GNRC_NETIF_IPV6_ADDRS_FLAGS_STATE_VALID            (0x10U)

/**
 * @brief   Address is an anycast address
 */
#define GNRC_NETIF_IPV6_ADDRS_FLAGS_ANYCAST                (0x20U)
/** @} */

/**
 * @brief   Flags to enable/disable an address privacy extension
 */
typedef uint8_t ipv6_aac_priv_flags_t;

/**
 * @name    Address privacy extension flags @ref ipv6_aac_priv_flags_t
 * @{
 */
/**
 * @brief   No privacy
 */
#define GNRC_NETIF_IPV6_AAC_FLAG_PRIV_NONE                  (0U)
/**
 * @brief   Enable SLAAC privacy extension
 */
#define GNRC_NETIF_IPV6_AAC_FLAG_PRIV_SLAAC                 (1U)
/**
 * @brief   Enable AAC of CGA
 */
#define GNRC_NETIF_IPV6_AAC_FLAG_PRIV_CGA                   (2U)
/** @} */

/**
 * @brief   Data type to decode which address privacy extension is used
 */
typedef uint8_t ipv6_addr_priv_t;

/**
 * @name    Values to be used as @ref ipv6_addr_priv_t
 * @{
 */
/**
 * @brief   Hardware identifier is used
 */
#define GNRC_NETIF_IPV6_ADDR_PRIV_NONE                      (0U)
/**
 * @brief   SLAAC privacy extension is used
 */
#define GNRC_NETIF_IPV6_ADDR_PRIV_SLAAC                     (1U)
/**
 * @brief   Address is a Cryptographically Generated Address
 */
#define GNRC_NETIF_IPV6_ADDR_PRIV_CGA                       (2U)
/** @} */

/**
 * @brief   Forward declaration of @ref gnrc_netif_ipv6_t struct
 */
struct gnrc_netif_ipv6;

/**
 * @brief   AAC bootstrapping function type, to be called on successful DAD
 *
 * @param[in]   netif   Interface on which @p add was added successfully
 * @param[in]   addr    IPv6 address
 * @param[in]   pfx_len Prefix length in bits
 */
typedef void (*gnrc_ipv6_aac_bootstrap_t)(struct gnrc_netif_ipv6 *netif,
                                          const ipv6_addr_t *addr,
                                          uint8_t pfx_len);

/**
 * @brief   Internal CGA context to correlate CGA parameters with addresses
 * @internal
 */
typedef struct {
    /**
     * @brief CGA parameters
     */
    ipv6_cga_parameters_t params[CONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF];
    /**
     * @brief   Address index associated with parameters
     */
    int addr_idx[CONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF];
} gnrc_ipv6_cga_ctx_t;

static_assert(CONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF <= 16,
              "Only a maximum of 16 IPv6 addresses are supported per interface");

/**
 * @brief   IPv6 component for @ref gnrc_netif_t
 *
 * @note only available with @ref net_gnrc_ipv6.
 * @note This struct has an alignment of 16 bytes to allow pointer tagging
 *       an address pointer with 16 different values.
 *       A pointer to an assigned ipv6 address passed to a function, may carry
 *       its index in the address array. Once we know the index, we can even
 *       figure out the pointer of this struct using container_of().
 */
typedef struct gnrc_netif_ipv6 {
    /**
     * @brief   IPv6 unicast and anycast addresses of the interface
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     */
    ipv6_addr_t addrs[CONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF];

    /**
     * @brief   IPv6 multicast groups of the interface
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     */
    ipv6_addr_t groups[GNRC_NETIF_IPV6_GROUPS_NUMOF];

    /**
     * @brief   Flags for gnrc_netif_t::ipv6_addrs
     *
     * @see net_gnrc_netif_ipv6_addrs_flags
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     */
    uint8_t addrs_flags[CONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF];

    /**
     * @brief   IPv6 address privacy extension
     */
    ipv6_addr_priv_t addrs_priv[CONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF];

#ifdef MODULE_NETSTATS_IPV6
    /**
     * @brief IPv6 packet statistics
     *
     * @note    Only available with module `netstats_ipv6`.
     */
    netstats_t stats;
#endif
#if defined(MODULE_GNRC_IPV6_NIB) || DOXYGEN
#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_ROUTER) || DOXYGEN
    /**
     * @brief   Route info callback
     *
     * This callback is called by the @ref net_gnrc_ipv6_nib "NIB" to inform
     * the routing protocol about state changes, route usages, missing routes
     * etc.
     *
     * The callback may be `NULL` if no such behavior is required by the routing
     * protocol (or no routing protocol is present).
     *
     * @param[in] type      [Type](@ref net_gnrc_ipv6_nib_route_info_type) of
     *                      the route info.
     * @param[in] ctx_addr  Context address of the route info.
     * @param[in] ctx       Further context of the route info.
     */
    void (*route_info_cb)(unsigned type, const ipv6_addr_t *ctx_addr,
                          const void *ctx);
    /**
     * @brief   Event for @ref GNRC_IPV6_NIB_SND_MC_RA
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     *          and @ref net_gnrc_ipv6_nib "NIB" and if
     *          @ref CONFIG_GNRC_IPV6_NIB_ROUTER != 0
     */
    evtimer_msg_event_t snd_mc_ra;
#endif  /* CONFIG_GNRC_IPV6_NIB_ROUTER */
#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_ARSM) || DOXYGEN
    /**
     * @brief   Event for @ref GNRC_IPV6_NIB_RECALC_REACH_TIME
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     *          and @ref net_gnrc_ipv6_nib "NIB" and if
     *          @ref CONFIG_GNRC_IPV6_NIB_ARSM != 0
     */
    evtimer_msg_event_t recalc_reach_time;
#endif /* CONFIG_GNRC_IPV6_NIB_ARSM */
    /**
     * @brief   Event for @ref GNRC_IPV6_NIB_SEARCH_RTR
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     *          and @ref net_gnrc_ipv6_nib "NIB"
     */
    evtimer_msg_event_t search_rtr;
#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_6LN) || IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_SLAAC) || DOXYGEN
    /**
     * @brief   Timers for address re-registration
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6" and
     *          @ref net_gnrc_ipv6_nib "NIB" and if
     *          @ref CONFIG_GNRC_IPV6_NIB_6LN != 0 or
     *          @ref CONFIG_GNRC_IPV6_NIB_SLAAC != 0
     * @note    Might also be usable in the later default SLAAC implementation
     *          for NS retransmission timers.
     */
    evtimer_msg_event_t addrs_timers[CONFIG_GNRC_NETIF_IPV6_ADDRS_NUMOF];
#endif

#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_ROUTER) || DOXYGEN
    /**
     * @brief   Timestamp in milliseconds of last unsolicited router
     *          advertisement
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     *          and @ref net_gnrc_ipv6_nib "NIB" and if
     *          @ref CONFIG_GNRC_IPV6_NIB_ROUTER != 0
     */
    uint32_t last_ra;
#endif  /* CONFIG_GNRC_IPV6_NIB_ROUTER */
#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_ARSM) || defined(DOXYGEN)
    /**
     * @brief   Base for random reachable time calculation and advertised
     *          reachable time in ms (if @ref GNRC_NETIF_FLAGS_IPV6_RTR_ADV is
     *          set)
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     *          and @ref net_gnrc_ipv6_nib "NIB" and if
     *          @ref CONFIG_GNRC_IPV6_NIB_ARSM != 0
     */
    uint32_t reach_time_base;

    /**
     * @brief   Reachable time (in ms)
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     *          and @ref net_gnrc_ipv6_nib "NIB" and if
     *          @ref CONFIG_GNRC_IPV6_NIB_ARSM != 0
     */
    uint32_t reach_time;
#endif /* CONFIG_GNRC_IPV6_NIB_ARSM */
    /**
     * @brief   Retransmission time and advertised retransmission time (in ms)
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6" and
     *          @ref net_gnrc_ipv6_nib "NIB"
     */
    uint32_t retrans_time;
#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_ROUTER) || DOXYGEN
    /**
     * @brief   (Advertised) Router lifetime (default 1800).
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     *          and @ref net_gnrc_ipv6_nib "NIB" and if
     *          @ref CONFIG_GNRC_IPV6_NIB_ROUTER != 0
     */
    uint16_t rtr_ltime;
    /**
     * @brief   number of unsolicited router advertisements sent
     *
     * This only counts up to the first @ref NDP_MAX_INIT_RA_NUMOF on interface
     * initialization. The last @ref NDP_MAX_FIN_RA_NUMOF of an advertising
     * interface are counted from UINT8_MAX - @ref NDP_MAX_FIN_RA_NUMOF + 1.
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     *          and @ref net_gnrc_ipv6_nib "NIB" and if
     *          @ref CONFIG_GNRC_IPV6_NIB_ROUTER != 0
     */
    uint8_t ra_sent;
#endif
    /**
     * @brief   number of unsolicited router solicitations scheduled
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6" and
     *          @ref net_gnrc_ipv6_nib "NIB"
     */
    uint8_t rs_sent;
    /**
     * @brief   number of unsolicited neighbor advertisements scheduled
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6" and
     *          @ref net_gnrc_ipv6_nib "NIB"
     */
    uint8_t na_sent;
#endif /* MODULE_GNRC_IPV6_NIB */

    /**
     * @brief   IPv6 auto-address configuration mode flags
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6"
     */
    uint8_t aac_mode;

    /**
     * @brief   IPv6 private auto-address configuration extension flags
     */
    ipv6_aac_priv_flags_t aac_priv;

    /**
     * @brief   Maximum transmission unit (MTU) for IPv6 packets
     *
     * @note    Only available with module @ref net_gnrc_ipv6 "gnrc_ipv6".
     */
    uint16_t mtu;
#if IS_USED(MODULE_IPV6_CGA)
    /**
     * @brief   CGA cryptographic context to save for the interface
     *
     * @note    Only available with module @ref net_ipv6_cga "ipv6_cga".
     */
    gnrc_ipv6_cga_ctx_t cga_ctx;
#if IS_USED(MODULE_GNRC_SEND)
    gnrc_send_ctx_t send_ctx;
#endif /* MODULE_GNRC_SEND */
#endif /* MODULE_IPV6_CGA */
} __attribute__((aligned(sizeof(ipv6_addr_t)))) gnrc_netif_ipv6_t;

/**
 * @brief   Get the tag of a pointer to an assigned IPv6 address
 *
 * @param[in, out]  addr    Pointer to address pointer to be untagged
 *
 * @return          The tag
 */
static inline unsigned gnrc_netif_ipv6_get_addr_index(ipv6_addr_t **addr)
{
    unsigned idx = ((uintptr_t)*addr) & (sizeof(ipv6_addr_t) - 1);
    *addr = (ipv6_addr_t *)(((uintptr_t)*addr) & ~(sizeof(ipv6_addr_t) - 1));
    return idx;
}

/**
 * @brief   Set a tag to an address pointer
 *
 * @param[in, out]  addr    Pointer to address to be tagged
 * @param[in]       idx     Address index to be encoded into @p addr
 *
 * @return          The tagged address pointer
 */
static inline ipv6_addr_t *gnrc_netif_ipv6_set_addr_index(ipv6_addr_t *addr, unsigned idx)
{
    addr = (ipv6_addr_t *)(((uintptr_t)addr) & ~(sizeof(ipv6_addr_t) - 1));
    addr = (ipv6_addr_t *)(((uintptr_t)addr) | idx);
    return addr;
}


/**
 * @brief   Accessor function to be used rather that `netif->cga_ctx`
 *
 * @param[in]   netif   GNRC IPv6 interface component
 *
 * @return      `netif->cga_ctx` or NULL if not supported
 */
static inline gnrc_ipv6_cga_ctx_t *gnrc_netif_ipv6_get_cga_ctx(gnrc_netif_ipv6_t *netif)
{
#if IS_USED(MODULE_IPV6_CGA)
    return &netif->cga_ctx;
#endif
    (void)netif;
    return NULL;
}

/**
 * @brief   Forward declare @ref gnrc_send_ctx_t
 */
struct gnrc_send_ctx;

/**
 * @brief   Accessor function to be used rather that `netif->send_ctx`
 *
 * @param[in]   netif   GNRC IPv6 interface component
 *
 * @return      `netif->send_ctx` or NULL if not supported
 */
static inline struct gnrc_send_ctx *gnrc_netif_ipv6_get_send_ctx(gnrc_netif_ipv6_t *netif)
{
#if IS_USED(MODULE_GNRC_SEND)
    return &netif->send_ctx;
#endif
    (void)netif;
    return NULL;
}


#ifdef __cplusplus
}
#endif

#endif /* NET_GNRC_NETIF_IPV6_H */
/** @} */
