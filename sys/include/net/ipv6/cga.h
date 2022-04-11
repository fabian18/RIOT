/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_ipv6_cga    IPv6 CGA
 * @ingroup     net_ipv6
 * @brief       IPv6 Cryptographically Generated Address generation and verification
 * @{
 *
 * @file
 * @brief       Interface for Cryptographically Generated Addresses (CGA) RFC3972
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */


#ifndef NET_IPV6_CGA_H
#define NET_IPV6_CGA_H

#include <stdint.h>

#include "net/ipv6/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Minimum CGA security value (Sec)
 */
#define IPV6_CGA_SEC_MIN                0

/**
 * @brief   Maximum CGA security value (Sec)
 */
#define IPV6_CGA_SEC_MAX                7

/**
 * @brief   Default CGA security value (SeC)
 *
 * The higher the security value, the higher the strength against
 * brute force attcks. The default value is the minimum.
 *
 * @warning For Sec values greater than zero, the CGA generation algorithm
 * is not guaranteed to terminate after a certain number of iterations.
 * You really don´t want to use Sec >= 2.
 */
#define IPV6_CGA_SEC_DEFAULT            IPV6_CGA_SEC_MIN

/**
 * @brief   CGA parameters
 */
typedef struct {
    uint8_t modifier[16];       /**< Randomly generated modifier */
    uint8_t prefix[8];   /**< IPv6 subnet prefix */
    uint8_t collision_count;    /**< Number of DAD collisions */
} ipv6_cga_parameters_t;

/**
 * @brief   Generate a CGA
 *
 * @param[in, out]  cga             In: IPv6 prefix,
 *                                  Out: generated address on success
 * @param[out]      parameters      Resulting CGA parameters
 * @param[in]       pk              Encoded SubjectPublicKeyInfo
 * @param[in]       pk_size         SubjectPublicKeyInfo size in bytes
 * @param[in]       sec             Security value
 * @param[in]       collisions      Number of previous DAD collisions
 *
 * @retval  0 on success
 */
int ipv6_cga_generate(ipv6_addr_t *cga,
                      ipv6_cga_parameters_t *parameters,
                      const void *pk,
                      size_t pk_size,
                      unsigned sec,
                      unsigned collisions);

/**
 * @brief   Verify a CGA
 *
 * The @p cga_parameters mus follow a SubjectPublicKeyInfo object in DER format
 * of @p pk_size bytes.
 *
 * @param[in]       cga             Address to verify
 * @param[in]       parameters      CGA parameters
 * @param[in]       pk              Encoded SubjectPublicKeyInfo
 * @param[in]       pk_size         SubjectPublicKeyInfo size in bytes
 */
int ipv6_cga_verify(const ipv6_addr_t *cga,
                    const ipv6_cga_parameters_t *parameters,
                    const void *pk,
                    size_t pk_size);

#ifdef __cplusplus
}
#endif

#endif /* NET_IPV6_CGA_H */
/** @} */
