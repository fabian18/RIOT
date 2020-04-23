/*
 * Copyright (C) 2015 Freie Universität Berlin
 *               2020 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    drivers_at86rf2xx_common common core of at86rf2xx drivers
 * @ingroup     drivers_netdev
 *
 * This module contains common driver code for radio devices in Atmel's AT86RF2xx series.
 *
 * @{
 *
 * @file
 * @brief       Common type definitions for AT86RF2xx based drivers
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Daniel Krebs <github@daniel-krebs.net>
 * @author      Kévin Roussel <Kevin.Roussel@inria.fr>
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */

#ifndef AT86RF2XX_COMMON_H
#define AT86RF2XX_COMMON_H

#include <stdint.h>

#include "kernel_defines.h"
#include "net/netdev.h"
#include "net/netdev/ieee802154.h"
#include "net/gnrc/nettype.h"
#if IS_USED(MODULE_AT86RF2XX_SPI)
#include "periph/gpio.h"
#include "periph/gpio_util.h"
#include "periph/spi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   struct holding all params needed for device initialization
 */
typedef struct at86rf2xx_params {
#if IS_USED(MODULE_AT86RF2XX_SPI)
    spi_t spi;              /**< SPI bus the device is connected to */
    spi_clk_t spi_clk;      /**< SPI clock speed to use */
    spi_cs_t cs_pin;        /**< GPIO pin connected to chip select */
    gpio_t int_pin;         /**< GPIO pin connected to the interrupt pin */
    gpio_t sleep_pin;       /**< GPIO pin connected to the sleep pin */
    gpio_t reset_pin;       /**< GPIO pin connected to the reset pin */
#else
    char dummy;
#endif
} at86rf2xx_params_t;

/**
 * @brief   Device descriptor for AT86RF2XX radio devices
 *
 * @extends netdev_ieee802154_t
 */
typedef struct at86rf2xx_base {
    netdev_ieee802154_t netdev;             /**< netdev parent struct */
    uint16_t flags;                         /**< Device specific flags */
    uint8_t state;                          /**< current state of the radio */
    uint8_t tx_frame_len;                   /**< length of the current TX frame */
    uint8_t idle_state;                     /**< state to return to after sending */
    uint8_t pending_tx;                     /**< keep track of pending TX calls
                                                 this is required to know when to
                                                 return to idle_state */
} at86rf2xx_base_t;

/**
 * @brief A pointer to an instance of an actual AT86RF2XX
 *        must safely be castable to a pointer of this type
 *
 * @extends     at86rf2xx_base_t
 */
typedef struct at86rf2xx {
    at86rf2xx_base_t base;                  /**< AT86RF2xx base */
    at86rf2xx_params_t params;              /**< AT86RF2xx params */
} at86rf2xx_t;

#ifdef __cplusplus
}
#endif

#endif /* AT86RF2XX_COMMON_H */
/** @} */
