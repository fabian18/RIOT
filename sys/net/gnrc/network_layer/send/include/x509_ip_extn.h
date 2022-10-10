/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gnrc_send
 * @{
 *
 * @file
 * @brief       Internal X509 IP block extension handling
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef GNRC_X509_IP_EXTN_H
#define GNRC_X509_IP_EXTN_H

#include <errno.h>
#include <stdbool.h>

#include "net/ipv6/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Shortcut for the frequent occurrence of this combination in DER
 */
#define ASN1_CONSTRUCTED_SEQUENCE   (MBEDTLS_ASN1_SEQUENCE | MBEDTLS_ASN1_CONSTRUCTED)
/**
 * @brief   DER object ID reserved for the IP block extension
 */
#define X509_EXTN_IP_RESOURCE_OID   { 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x07 }

/**
 * @brief   Structure representing an IP block extension
 */
typedef struct x509_ip_address_block {
    union {
        struct  {
            ipv6_addr_t addr;           /**< Prefix */
            uint8_t len;                /**< Prefix bitlength */
        } u_prefix;                 /**< Restriction is a prefix */
        struct {
            ipv6_addr_t min;            /**< IP range minimum */
            ipv6_addr_t max;            /**< IP range maximum */
        } u_range;                  /**< Restriction is expressed as an IP range */
    } address_or_range;         /**< A restriction can either be a prefix or an IP range */
    bool is_range;              /**< True if this struct expresses an Ip range and not a prefix */
} x509_ip_address_block_t;

/**
 * @brief   Convert an IP prefix restriction to an IP range restriction
 *
 * Note that every IP restriction can be expressed as a range but may not be
 * expressible as a prefix.
 */
void x509_ip_prefix_to_range(x509_ip_address_block_t *ip);

/**
 * @brief   Check whether @p sub is an IP subrange of @p range
 *          and build the intersection of the two address ranges
 *
 * @param[out]      intersect       If not NULL, intersection range of @p sub and @p range
 * @param[in]       range           Expected IP superrange
 * @param[in]       sub             Expected IP subrange
 *
 * @retval  Negative number if there is no intersection
 * @retval  Positive number if the intersection is not empty but @p sub
 *          is not a full subrange of @p range
 * @retval  0 if the intersection of @p sub and @p range is equal to @p sub
 */
int x509_ip_is_subrange(x509_ip_address_block_t *intersect,
                        const x509_ip_address_block_t *range,
                        const x509_ip_address_block_t *sub);

/**
 * @brief   Parse the DER encoding of an IP block extension
 *
 * @param[out]      ip_list         Array to write the parsed restrictions to
 * @param[in]       max             Maximum number of IP block restrictions that can be processed
 * @param[in]       p               Beginning of the IP block DER encoding
 * @param[in]       end             End of DER encoding
 *
 * @return Number of parsed IP block restrictions or negative number on error
 */
int x509_parse_ip_address_block(x509_ip_address_block_t *ip_list, unsigned max,
                                const uint8_t *p, const uint8_t *end);

#ifdef __cplusplus
}
#endif
#endif /* GNRC_X509_IP_EXTN_H */
/** @} */
