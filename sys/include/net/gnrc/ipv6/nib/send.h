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
 * @brief       SEcure Neighbor Discovery API
 * @{
 *
 * @file
 * @brief   SEND NIB API
 *
 * @author  Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef NET_GNRC_IPV6_NIB_SEND_H
#define NET_GNRC_IPV6_NIB_SEND_H

#include "net/gnrc/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   SEND operation modes
 */
typedef enum {
    GNRC_NIB_IPV6_SEND_MODE_OFF = 0,    /**< SEND is turned off */
    GNRC_NIB_IPV6_SEND_MODE_STRICT,     /**< Only SEND secured ICMPv6 messages are accepted */
    GNRC_NIB_IPV6_SEND_MODE_COMPAT,     /**< Unsecured messages are accepted but entries created from
                                             secured messages are preferred */
} gnrc_ipv6_nib_send_mode_t;

/**
 * @brief   Enable SEND on an interface
 *
 * @param[in]       netif           Network interface on which to enable SEND
 * @param[in]       mode            Either @ref GNRC_NIB_IPV6_SEND_MODE_STRICT or
 *                                  @ref GNRC_NIB_IPV6_SEND_MODE_COMPAT
 *
 * @retval  0 on success
 * @retval  Negative number on error
 */
int gnrc_ipv6_nib_send_enable(gnrc_netif_t *netif, gnrc_ipv6_nib_send_mode_t mode);

/**
 * @brief   Get the SEND operation mode running on interface @p netif
 *
 * @param[in]       netif           Network interface from which to get the SEND mode from
 *
 * @return SEND operation mode @ref gnrc_ipv6_nib_send_mode_t
 */
gnrc_ipv6_nib_send_mode_t gnrc_ipv6_nib_send_get_mode(gnrc_netif_t *netif);

/**
 * @brief   Set the SEND operation mode
 *
 * @param[in]       netif           Network interface on which to set the SEND mode
 * @param[in]       mode            SEND mode either @ref GNRC_NIB_IPV6_SEND_MODE_STRICT or
 *                                  @ref GNRC_NIB_IPV6_SEND_MODE_COMPAT
 */
void gnrc_ipv6_nib_send_set_mode(gnrc_netif_t *netif, gnrc_ipv6_nib_send_mode_t mode);

/**
 * @brief   Disable SEND on an interface
 *
 * @param[in]       netif           Network interface on which to disable SEND
 */
void gnrc_ipv6_nib_send_disable(gnrc_netif_t *netif);

#ifdef __cplusplus
}
#endif

#endif /* NET_GNRC_IPV6_NIB_SEND_H */
/** @} */
