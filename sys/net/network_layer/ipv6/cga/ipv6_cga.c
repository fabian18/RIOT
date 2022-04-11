/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_ipv6_cga
 * @{
 *
 * @file
 * @brief       Cryptographically Generated Addresses (CGA) RFC3972
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 * @}
 */

#include <assert.h>
#include <errno.h>

#include "_cga_crypto.h"
#include "_cga_random.h"
#include "net/ipv6/cga.h"

typedef struct {
    uint8_t m[8];
} ipv6_cga_ces_mask_t;

typedef struct {
    uint8_t m[8];
} ipv6_cga_hash1_mask_t;

typedef struct {
    uint8_t m[14];
} ipv6_cga_hash2_mask_t;

#define IPV6_CGA_SEC_COLLISION_MAX      2

#define IPV6_CGA_SEC_MASK               {0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define IPV6_CGA_HASH1_MASK             {0x1c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}

typedef struct {
    uint8_t x[256U / 8];
    uint8_t y[256U / 8];
} ipv6_cga_pk_ec_256_t;

static inline void _inc_modifier(ipv6_cga_parameters_t *parameters)
{

    uint8_t modifier[16] __attribute__((aligned(4)));
    memcpy(modifier, parameters->modifier, sizeof(modifier));
    be_uint32_t *mod32 = (be_uint32_t *)modifier;
    if (mod32[3].u32 != 0xffffffff) {
        mod32[3] = byteorder_htonl(byteorder_ntohl(mod32[3]) + 1u);
        goto ret;
    }
    mod32[3].u32 = 0;
    if (mod32[2].u32 != 0xffffffff) {
        mod32[2] = byteorder_htonl(byteorder_ntohl(mod32[2]) + 1u);
        goto ret;
    }
    mod32[2].u32 = 0;
    if (mod32[1].u32 != 0xffffffff) {
        mod32[1] = byteorder_htonl(byteorder_ntohl(mod32[1]) + 1u);
        goto ret;
    }
    mod32[1].u32 = 0;
    if (mod32[0].u32 != 0xffffffff) {
        mod32[0] = byteorder_htonl(byteorder_ntohl(mod32[0]) + 1u);
        goto ret;
    }
    mod32[0].u32 = 0;
ret:
    memcpy(parameters->modifier, modifier, sizeof(modifier));
}

int ipv6_cga_generate(ipv6_addr_t *cga,
                      ipv6_cga_parameters_t *parameters,
                      const void *pk,
                      size_t pk_size,
                      uint8_t sec,
                      uint8_t collisions)
{
    assert(cga);
    assert(parameters);
    if (sec > IPV6_CGA_SEC_MAX) {
        return -EINVAL;
    }
    if (collisions > IPV6_CGA_SEC_COLLISION_MAX) {
        return -ECANCELED;
    }
    uint8_t digest[SHA1_DIGEST_LENGTH];
    cga_sha1_ctx_t sha1;
    if (collisions == 0) {
        memset(parameters, 0, sizeof(*parameters));
        cga_random_get(parameters->modifier, sizeof(parameters->modifier));
HASH2:
        cga_sha1_init(&sha1);
        cga_sha1_start(&sha1);
        cga_sha1_update(&sha1, parameters, sizeof(*parameters));
        cga_sha1_update(&sha1, pk, pk_size);
        /* 112 leftmost bits are HASH2 */
        cga_sha1_finish(&sha1, digest);
        for (unsigned i = 0; i < sec; i += 2) {
            if (digest[i] || digest[i + 1]) {
                _inc_modifier(parameters);
                goto HASH2;
            }
        }
    }
    const uint8_t *ip_prefix = cga->u8;
    memcpy(parameters->prefix, ip_prefix, sizeof(parameters->prefix));
    parameters->collision_count = collisions;
    cga_sha1_init(&sha1);
    cga_sha1_start(&sha1);
    cga_sha1_update(&sha1, parameters, sizeof(*parameters));
    cga_sha1_update(&sha1, pk, pk_size);
    cga_sha1_finish(&sha1, digest);
    memcpy(&cga->u8[8], digest, 8);
    /* IID: 3 most significant bits are set to 'sec' and U and G bits are set to 0 */
    cga->u8[8] = ((unsigned)sec << 5) | (digest[0] & 0x1c); /* 0x1c = 00011100 */
    memcpy(&cga->u8[0], ip_prefix, 8);
    return 0;
}

int ipv6_cga_verify(const ipv6_addr_t *cga,
                    const ipv6_cga_parameters_t *cga_parameters,
                    const void *pk,
                    size_t pk_size)
{
    assert(cga);
    assert(cga_parameters);

    if (cga_parameters->collision_count > IPV6_CGA_SEC_COLLISION_MAX) {
        return -1;
    }
    if (memcmp(&cga->u8[0], cga_parameters->prefix, sizeof(cga_parameters->prefix))) {
        return -1;
    }
    uint8_t digest[SHA1_DIGEST_LENGTH];
    cga_sha1_ctx_t sha1;
    cga_sha1_init(&sha1);
    cga_sha1_start(&sha1);
    cga_sha1_update(&sha1, cga_parameters, sizeof(*cga_parameters) + pk_size);
    cga_sha1_finish(&sha1, digest);
    if (((digest[0] & 0x1c) != (cga->u8[8] & 0x1c)) || memcmp(digest + 1, &cga->u8[9], 7)) {
        return -1;
    }
    uint8_t sec = (cga->u8[8] & 0xe0) >> 5;
    uint8_t zero = 0;
    cga_sha1_init(&sha1);
    cga_sha1_start(&sha1);
    cga_sha1_update(&sha1, cga_parameters->modifier, sizeof(cga_parameters->modifier));
    for (unsigned i = 0; i < 9; i++) {
        cga_sha1_update(&sha1, &zero, sizeof(zero));
    }
    cga_sha1_update(&sha1, pk, pk_size);
    cga_sha1_finish(&sha1, digest);
    for (unsigned i = 0; i < sec; i += 2) {
        if (digest[i] || digest[i + 1]) {
            return -1;
        }
    }
    return 0;
}
