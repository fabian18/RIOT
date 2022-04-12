/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     pkg_mbedtls_random
 *
 * @{
 * @file
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include <assert.h>
#include "entropy_mbedtls_riot.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

static mbedtls_entropy_context _entropy;
static mbedtls_ctr_drbg_context _rng;

int random_ctr_drbg_mbedtls_riot_init(void)
{
    mbedtls_entropy_init(&_entropy);
    mbedtls_ctr_drbg_init(&_rng);
    /* entropy sources are added by module mbedtls_entropy */
    int s = mbedtls_ctr_drbg_seed(&_rng,
                                  mbedtls_entropy_func,
                                  &_entropy,
                                  (const unsigned char *)"r10T-0$",
                                  7);

    return s;
}

void random_ctr_drbg_mbedtls_get(void *out, size_t size)
{
    mbedtls_ctr_drbg_random(&_rng, out, size);
}
