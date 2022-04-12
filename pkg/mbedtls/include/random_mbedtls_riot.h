/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    pkg_mbedtls_random Access API to Mbed TLS random module
 * @ingroup     pkg_mbedtls
 *
 * @{
 * @file
 * @brief       Internal wrapper around Mbed TLS API with internally allocated
 *              context to generate random data
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 */

#ifndef RANDOM_MBEDTLS_RIOT_H
#define RANDOM_MBEDTLS_RIOT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialize and seed the internal CTR DRBG context
 *
 * @return  0 on success
 */
int random_ctr_drbg_mbedtls_riot_init(void);

/**
 * @brief
 *
 * @param[out]  out     Output buffer containing random data
 * @param[in]   size    Number of random bytes to write to @p buf
 */
void random_ctr_drbg_mbedtls_get(void *out, size_t size);

#ifdef __cplusplus
}
#endif
/** @} */

#endif /* RANDOM_MBEDTLS_RIOT_H */
