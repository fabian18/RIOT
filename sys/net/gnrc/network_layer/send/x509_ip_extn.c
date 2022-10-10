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
 * @brief   Handling of the X509 IP block extension
 *          @see [RFC3779](https://datatracker.ietf.org/doc/html/rfc3779)
 *
 * @author  Fabian Hüßler <fabian.huessler@ovgu.de>
 * @}
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "mbedtls/x509.h"
#include "net/ipv6/addr.h"
#include "x509_ip_extn.h"

static inline bool _is_null(x509_ip_address_block_t *ip)
{
    if (ip->is_range) {
        return !memcmp(&ip->address_or_range.u_range.min,
                       &ipv6_addr_unspecified, sizeof(ipv6_addr_t)) &&
               !memcmp(&ip->address_or_range.u_range.max,
                       &ipv6_addr_mask, sizeof(ipv6_addr_t));
    }
    return !memcmp(&ip->address_or_range.u_prefix.addr,
                   &ipv6_addr_unspecified, sizeof(ipv6_addr_t)) &&
           ip->address_or_range.u_prefix.len == 0;
}

void x509_ip_prefix_to_range(x509_ip_address_block_t *ip)
{
    if (ip->is_range) {
        return;
    }
    ipv6_addr_t pfx = ip->address_or_range.u_prefix.addr;
    uint8_t pfx_len = ip->address_or_range.u_prefix.len;
    memset(&ip->address_or_range.u_range.min, 0x00, sizeof(ip->address_or_range.u_range.min));
    memcpy(&ip->address_or_range.u_range.min, &pfx, (pfx_len + 7) / 8);
    memset(&ip->address_or_range.u_range.max, 0xff, sizeof(ip->address_or_range.u_range.max));
    memcpy(&ip->address_or_range.u_range.max, &pfx, (pfx_len + 7) / 8);
    uint8_t unused_bits = (8 - (pfx_len % 8)) % 8;
    ip->address_or_range.u_range.max.u8[((pfx_len + 7) / 8) - 1] |= (~(((uint8_t)0xff) << unused_bits));
    ip->is_range = true;
}

int x509_ip_is_subrange(x509_ip_address_block_t *intersect,
                        const x509_ip_address_block_t *range,
                        const x509_ip_address_block_t *sub)
{
/* If an addressPrefix or addressRange is not contained within the
   delegating authority's subnet prefixes or ranges, the client MAY
   attempt to take an intersection of the ranges/subnet prefixes and to
   use that intersection. */
    assert(range->is_range);
    assert(sub->is_range);
    int subrange = 0;
    if (intersect) {
        *intersect = *sub; /* range: [ sub: ( ) ] */
    }
/*  If the resulting intersection is empty, the client MUST NOT accept the certificate. */
    for (unsigned i = 0; i < sizeof(ipv6_addr_t); i++) {
        if (range->address_or_range.u_range.max.u8[i] ^ sub->address_or_range.u_range.min.u8[i]) {
            if (sub->address_or_range.u_range.min.u8[i] > range->address_or_range.u_range.max.u8[i]) {
                return -1; /* range: [  ] sub: (  ) */
            }
            break;
        }
    }
    for (unsigned i = 0; i < sizeof(ipv6_addr_t); i++) {
        if (range->address_or_range.u_range.min.u8[i] ^ sub->address_or_range.u_range.max.u8[i]) {
            if (sub->address_or_range.u_range.max.u8[i] < range->address_or_range.u_range.min.u8[i]) {
                return -1; /* sub: (  ) range: [  ] */
            }
            break;
        }
    }
    for (unsigned i = 0; i < sizeof(ipv6_addr_t); i++) {
        if (range->address_or_range.u_range.min.u8[i] ^ sub->address_or_range.u_range.min.u8[i]) {
            if (sub->address_or_range.u_range.min.u8[i] < range->address_or_range.u_range.min.u8[i]) {
                if (intersect) {
                    memcpy(intersect->address_or_range.u_range.min.u8,
                           range->address_or_range.u_range.min.u8,
                           sizeof(range->address_or_range.u_range.min.u8));
                }
                subrange = 1; /* sub: ( range: [ ) ] */
            }
            break;
        }
    }
    for (unsigned i = 0; i < sizeof(ipv6_addr_t); i++) {
        if (range->address_or_range.u_range.max.u8[i] ^ sub->address_or_range.u_range.max.u8[i]) {
            if (sub->address_or_range.u_range.max.u8[i] > range->address_or_range.u_range.max.u8[i]) {
                if (intersect) {
                    memcpy(intersect->address_or_range.u_range.max.u8,
                           range->address_or_range.u_range.max.u8,
                           sizeof(range->address_or_range.u_range.max.u8));
                }
                subrange = 1; /* range: [ sub: ( ] ) */
            }
            break;
        }
    }
    return subrange;
}

int x509_parse_ip_address_block(x509_ip_address_block_t *ip_list, unsigned max,
                                const uint8_t *p, const uint8_t *end)
{
    int ret;
    size_t len;
    unsigned m = ip_list ? max : 0;
    if ((ret = mbedtls_asn1_get_tag((uint8_t **)&p, end, &len, ASN1_CONSTRUCTED_SEQUENCE)) != 0) {
        return ret;
    }
    while ((ret = mbedtls_asn1_get_tag((uint8_t **)&p, end, &len, ASN1_CONSTRUCTED_SEQUENCE)) == 0) {
        if ((ret = mbedtls_asn1_get_tag((uint8_t**)&p, end, &len, MBEDTLS_ASN1_OCTET_STRING)) != 0) {
            return ret;
        }
        /* The addressFamily element of the IPAddrBlocks sequence element MUST contain the IPv6
            Address Family Identifier (0002) [...]. */
        if (len != 2 || p[0] != 0x00 || p[1] != 0x02) {
            return -ENOTSUP;
        }
        p += len;
        if ((ret = mbedtls_asn1_get_tag((uint8_t **)&p, end, &len, MBEDTLS_ASN1_NULL)) == 0) {
            if (p + len != end) {
                return -EBADMSG;
            }
            if (m) {
                memset(ip_list, 0, sizeof(*ip_list));
            }
            return 1;
        }
        else if ((ret = mbedtls_asn1_get_tag((uint8_t **)&p, end, &len, ASN1_CONSTRUCTED_SEQUENCE)) == 0) {
            /* The X.509 IP address extension MAY contain additional IPv6 subnet prefixes, expressed
            as either an addressPrefix or an addressRange. */
            while (p != end) {
                if (m) {
                    memset(ip_list, 0, sizeof(*ip_list));
                }
                /* The X.509 IP address extension MUST contain at least one addressesOrRanges element. */
                if ((ret = mbedtls_asn1_get_tag((uint8_t **)&p, end, &len, MBEDTLS_ASN1_BIT_STRING)) == 0) {
                    /* This element MUST contain an addressPrefix element containing an IPv6 address prefix
                    for a prefix that the router or the intermediate entity is authorized to route. */
                    if (m) {
                        memcpy(&ip_list->address_or_range.u_prefix.addr, p + 1, len - 1);
                        ip_list->address_or_range.u_prefix.len = ((len - 1) * 8) - p[0];
                    }
                    p += len;
                }
                else if ((ret = mbedtls_asn1_get_tag((uint8_t **)&p, end, &len, ASN1_CONSTRUCTED_SEQUENCE)) == 0) {
                    /* Instead of an addressPrefix element, the addressesOrRange element MAY contain an
                    addressRange element for a range of subnet prefixes, if more than one prefix is
                    authorized. */
                    uint8_t unused;
                    if ((ret = mbedtls_asn1_get_tag((uint8_t **)&p, end, &len, MBEDTLS_ASN1_BIT_STRING)) != 0) {
                        return ret;
                    }
                    unused = p[0];
                    if (unused > 7) {
                        return -ENOTSUP;
                    }
                    if (m) {
                        ip_list->is_range = true;
                        memcpy(&ip_list->address_or_range.u_range.min, p + 1, len - 1);
                    }
                    p += len;
                    if ((ret = mbedtls_asn1_get_tag((uint8_t **)&p, end, &len, MBEDTLS_ASN1_BIT_STRING)) != 0) {
                        return ret;
                    }
                    unused = p[0];
                    if (unused > 7) {
                        return -ENOTSUP;
                    }
                    if (m) {
                        memset(&ip_list->address_or_range.u_range.max, 0xff, sizeof(ip_list->address_or_range.u_range.max));
                        memcpy(&ip_list->address_or_range.u_range.max, p + 1, len - 1);
                        ip_list->address_or_range.u_range.max.u8[len - 2] |= (~(((uint8_t)(0xff)) << unused));
                    }
                    p += len;
                }
                else {
                    return -EBADMSG;
                }
                if (m) {
                    m--;
                    ip_list++;
                }
            }
        }
        else {
            return -ENOENT;
        }
    }
    return max - m;
}
