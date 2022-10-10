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
 * @brief       Cryptography interface for Cryptographically Generated Addresses (CGA) RFC3972
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */

#ifndef PRIVATE_IPV6_CGA_CRYPTO_H
#define PRIVATE_IPV6_CGA_CRYPTO_H

#include "kernel_defines.h"
#include "hashes/sha1.h"

#ifdef __cplusplus
extern "C" {
#endif

#if IS_USED(MODULE_IPV6_CGA_CRYPTO_MBEDTLS) || defined(DOXYGEN)
#include "mbedtls/sha1.h"

/**
 * @brief   Internal CGA hash context
 */
typedef mbedtls_sha1_context cga_sha1_ctx_t;
/**
 * @brief   Initialize a CGA hash context
 *
 * @param[out]      ctx     Internal CGA hash context
 */
static inline void cga_sha1_init(cga_sha1_ctx_t *ctx)
{
    mbedtls_sha1_init(ctx);
}
/**
 * @brief   Start a hash computation using a CGA hash context
 *
 * @param[in, out]  ctx     Internal CGA hash context
 */
static inline void cga_sha1_start(cga_sha1_ctx_t *ctx)
{
    mbedtls_sha1_starts(ctx);
}
/**
 * @brief   Add input data to the hash computation using a CGA hash context
 *
 * @param[in, out]  ctx     Internal CGA hash context
 * @param[in]       in      Input data pointer
 * @param[in]       size    Size of new input data
 */
static inline void cga_sha1_update(cga_sha1_ctx_t *ctx, const void *in, size_t size)
{
    mbedtls_sha1_update(ctx, (const unsigned char *)in, size);
}
/**
 * @brief   Finish the hash computation using a CGA hash context and write the result to @p out
 *
 * @param[in, out]  ctx     Internal CGA hash context
 * @param[out]      out     Output data pointer
 */
static inline void cga_sha1_finish(cga_sha1_ctx_t *ctx, void *out)
{
    mbedtls_sha1_finish(ctx, (unsigned char *)out);
}
#else

typedef sha1_context cga_sha1_ctx_t;

static inline void cga_sha1_init(cga_sha1_ctx_t *ctx)
{
    sha1_init(ctx);
}
static inline void cga_sha1_start(cga_sha1_ctx_t *ctx)
{
    (void)ctx;
}
static inline void cga_sha1_update(cga_sha1_ctx_t *ctx, const void *in, size_t size)
{
    sha1_update(ctx, in, size);
}
static inline void cga_sha1_finish(cga_sha1_ctx_t *ctx, void *out)
{
    sha1_final(ctx, out);
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* PRIVATE_IPV6_CGA_CRYPTO_H */
/** @} */
