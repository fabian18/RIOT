/*
 * Copyright (C) 2015 Freie Universität Berlin
 *               2020 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    drivers_at86rf233 driver for at86rf233
 * @ingroup     drivers_netdev
 *
 * This module contains the netdev driver for Atmel's AT86RF233 transceiver.
 *
 * @{
 *
 * @file
 * @brief       Interface definition of AT86RF233 driver
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Daniel Krebs <github@daniel-krebs.net>
 * @author      Kévin Roussel <Kevin.Roussel@inria.fr>
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef AT86RF233_H
#define AT86RF233_H

#include "at86rf2xx_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief AT86RF233 params
 *
 * @extends at86rf2xx_params_t
 */
typedef struct at86rf233_params {
    at86rf2xx_params_t base_params;    /**< AT86RF2xx base params */
} at86rf233_params_t;

/**
 * @brief AT86RF233 device
 *
 * @extends at86rf2xx_base_t
 */
typedef struct at86rf233 {
    at86rf2xx_base_t base;                  /**< AT86RF2xx base */
    at86rf233_params_t params;              /**< AT86RF233 params */
} at86rf233_t;

/**
 * @brief   Initialize an at86rf233 device with initial parameters
 */
void at86rf233_setup(at86rf233_t *dev, const at86rf233_params_t *params);

#ifdef __cplusplus
}
#endif

#endif /* AT86RF233_H */
/** @} */
