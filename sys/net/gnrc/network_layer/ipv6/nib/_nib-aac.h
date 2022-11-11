/*
 * Copyright (C) 2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup net_gnrc_ipv6_nib
 * @brief
 * @{
 *
 * @file
 * @brief   Definions related to AAC functionality of the NIB
 * @see     @ref CONFIG_GNRC_IPV6_NIB_SLAAC
 * @internal
 *
 * @author  Martine Lenders <m.lenders@fu-berlin.de>
 */
#ifndef PRIV_NIB_AAC_H
#define PRIV_NIB_AAC_H

#include <kernel_defines.h>
#include <stdint.h>

#include "net/gnrc/ipv6/nib/conf.h"
#include "net/gnrc/netif.h"
#include "net/ipv6/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Generate a CGA from @p pfx, assign it to @p netif, and start the DAD process
 *
 * @param[in, out] netif   Interface to which to assign the generated address
 * @param[in]      pfx     IPv6 Prefix to generate the address from
 * @param[in]      pfx_len Prefix length in bits
 */
void _auto_configure_cga(gnrc_netif_t *netif,
                         const ipv6_addr_t *pfx, uint8_t pfx_len);
/**
 * @brief   Should be called if the DAD process of @p address failed to attempt
 *          generate a new address and restart the DAD process
 *
 * @param[in, out] netif   Interface to which to assign the generated address
 * @param[in]      address Address for which DAD failed
 */
void _auto_reconfigure_cga(gnrc_netif_t *netif, const ipv6_addr_t *address);

#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_6LN) || IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_SLAAC) || defined(DOXYGEN)
/**
 * @brief   Auto-configures an address from a given prefix
 *
 * @param[in] netif     The network interface the address should be added to.
 * @param[in] pfx       The prefix for the address.
 * @param[in] pfx_len   Length of @p pfx in bits.
 */
void _auto_configure_addr(gnrc_netif_t *netif, const ipv6_addr_t *pfx,
                          uint8_t pfx_len);
#else   /* CONFIG_GNRC_IPV6_NIB_6LN || CONFIG_GNRC_IPV6_NIB_SLAAC */
#define _auto_configure_addr(netif, pfx, pfx_len) \
    (void)netif; (void)pfx; (void)pfx_len;
#endif  /* CONFIG_GNRC_IPV6_NIB_6LN || CONFIG_GNRC_IPV6_NIB_SLAAC */
#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_SLAAC) || defined(DOXYGEN)
/**
 * @brief   Removes a tentative address from the interface and tries to
 *          reconfigure a new address
 *
 * @param[in] netif The network interface the address is to be removed from.
 * @param[in] addr  The address to remove.
 */
void _remove_tentative_addr(gnrc_netif_t *netif, const ipv6_addr_t *addr);
#else   /* CONFIG_GNRC_IPV6_NIB_SLAAC */
#define _remove_tentative_addr(netif, addr) \
    (void)netif; (void)addr
#endif

#if IS_ACTIVE(CONFIG_GNRC_IPV6_NIB_SLAAC) || IS_USED(MODULE_IPV6_CGA) || defined(DOXYGEN)
/**
 * @brief   Handle @ref GNRC_IPV6_NIB_DAD event
 *
 * @param[in] addr  A TENTATIVE address.
 *
 * @pre     @p addr must point into the address array of @ref gnrc_netif_ipv6_t
 *          and the pointer must have been tagged with the index, using
 *          @ref gnrc_netif_ipv6_set_addr_index()
 */
void _handle_dad(ipv6_addr_t *addr);

/**
 * @brief   Handle @ref GNRC_IPV6_NIB_VALID_ADDR event
 *
 * @param[in] addr  A TENTATIVE address.
 *
 * @pre     @p addr must point into the address array of @ref gnrc_netif_ipv6_t
 *          and the pointer must have been tagged with the index, using
 *          @ref gnrc_netif_ipv6_set_addr_index()
 */
void _handle_valid_addr(ipv6_addr_t *addr);
#else   /* CONFIG_GNRC_IPV6_NIB_SLAAC */
#define _handle_dad(addr)           (void)addr
#define _handle_valid_addr(addr)    (void)addr
#endif  /* CONFIG_GNRC_IPV6_NIB_SLAAC */

#ifdef __cplusplus
}
#endif

#endif /* PRIV_NIB_AAC_H */
/** @} */
