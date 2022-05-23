/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gnrc_ipv6_nib_send  NIB SEND
 * @ingroup     net_gnrc_ipv6_nib
 * @brief       SEcure Neighbor Discovery API, modifying the neighbor information base
 * @{
 *
 * @file
 * @brief   SEND NIB API
 *
 * @author  Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef NET_GNRC_IPV6_NIB_SEND_H
#define NET_GNRC_IPV6_NIB_SEND_H

#include "kernel_defines.h"
#include "net/gnrc/netif.h"
#include "ptrtag.h"
#include "net/send.h"

#ifdef __cplusplus
extern "C" {
#endif

#if IS_USED(MODULE_GNRC_SEND)
#define GNRC_SEND_EXTRA_STACK_SIZE              (4096U)
#else
#define GNRC_SEND_EXTRA_STACK_SIZE              (0U)
#endif

enum {
    GNRC_SEND_STATUS_OK = 0x00,
    GNRC_SEND_STATUS_UNSECURED = 0x01u,
    GNRC_SEND_STATUS_CGA_FAIL,
    GNRC_SEND_STATUS_SIGNATURE_FAIL,
    GNRC_SEND_STATUS_BAD_FORMAT,
    GNRC_SEND_STATUS_NO_KEY,
    GNRC_SEND_STATUS_NONCE_MISMATCH,
    GNRC_SEND_STATUS_NONCE_MISSING,
    GNRC_SEND_STATUS_ADD_IN_PROGRESS,
};

/**
 * @brief
 */
#define GNRC_SEND_SECURED(sec) (IS_USED(MODULE_GNRC_SEND) && ((sec) == GNRC_SEND_STATUS_OK))

typedef struct gnrc_send_cache_nonce gnrc_send_cache_nonce_t;

typedef struct gnrc_send_cache_cps gnrc_send_cache_cps_t;

typedef struct gnrc_send_cache_cpa gnrc_send_cache_cpa_t;


/**
 * @brief
 */
static inline void *gnrc_send_set_status(const void *ptr, uint8_t stat)
{
#if IS_USED(MODULE_GNRC_SEND)
    return ptrtag((void *)ptr, stat);
#endif
    (void)stat;
    return (void *)ptr;
}

/**
 * @brief
 */
static inline int gnrc_send_remove_status(const void **ptr)
{
#if IS_USED(MODULE_GNRC_SEND)
    int stat = ptrtag_tag((void *)*ptr);
    *ptr = ptrtag_ptr((void *)*ptr);
    return stat;
#endif
    (void)ptr;
    return 0;
}


/**
 * @brief
 */
int gnrc_ipv6_nib_send_enable(gnrc_netif_t *netif);

/**
 * @brief
 */
int gnrc_ipv6_nib_send_is_enabled(gnrc_netif_t *netif);

/**
 * @brief
 */
void gnrc_ipv6_nib_send_disable(gnrc_netif_t *netif);

/**
 * @brief
 */
int gnrc_send_nib_handle_nbr_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_nbr_sol_t *nbr_sol, size_t icmpv6_len);

/**
 * @brief
 */
int gnrc_send_nib_handle_nbr_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_nbr_adv_t *nbr_adv, size_t icmpv6_len);

/**
 * @brief
 */
int gnrc_send_nib_handle_rtr_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_rtr_sol_t *rtr_sol, size_t icmpv6_len);

/**
 * @brief
 */
int gnrc_send_nib_handle_rtr_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_rtr_adv_t *rtr_adv, size_t icmpv6_len);

/**
 * @brief
 */
void gnrc_send_nib_handle_cp_sol(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_cp_sol_t *cp_sol, size_t icmpv6_len);

/**
 * @brief
 */
void gnrc_send_nib_handle_cp_adv(gnrc_netif_t *netif, const ipv6_hdr_t *ipv6,
                                 const ndp_cp_adv_t *cp_adv, size_t icmpv6_len);

/**
 * @brief
 */
void *gnrc_send_nib_handle_nonce(const ndp_opt_nonce_t *nonce_opt, size_t *nonce_size);

/**
 * @brief
 */
void gnrc_send_nib_handle_snd_cp_sol(gnrc_send_cache_cps_t *cps_ctx);

/**
 * @brief
 */
void gnrc_send_nib_handle_snd_cp_adv(gnrc_send_cache_cpa_t *cpa_ctx);

#ifdef __cplusplus
}
#endif

#endif /* NET_GNRC_IPV6_NIB_SEND_H */
/** @} */
