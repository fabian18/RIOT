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
 * @brief       Randomness interface for Cryptographically Generated Addresses (CGA) RFC3972
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */

#ifndef PRIVATE_IPV6_CGA_RANDOM
#define PRIVATE_IPV6_CGA_RANDOM

#include "kernel_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

#if IS_USED(MODULE_IPV6_CGA_RANDOM_MBEDTLS) || defined(DOXYGEN)
#include "random_mbedtls_riot.h"

/**
 * @brief   Internal random bytes function
 *
 * @param[in]       out     Output buffer to write random bytes to
 * @param[in]       size    Number of random bytes requested
 */
static inline void cga_random_get(void *out, size_t size)
{
    random_ctr_drbg_mbedtls_get(out, size);
}

#else
#include "random.h"

static inline void cga_random_get(void *out, size_t size)
{
    random_bytes((uint8_t *)out, size);
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* PRIVATE_IPV6_CGA_RANDOM */
/** @} */
