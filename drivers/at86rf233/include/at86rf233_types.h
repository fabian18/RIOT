/* Copyright (C) 2013 Alaeddine Weslati <alaeddine.weslati@inria.fr>
 *               2015 Freie Universität Berlin
 *               2015 Kaspar Schleiser <kaspar@schleiser.de>
 *               2015 Hauke Petersen <hauke.petersen@fu-berlin.de>
 *               2017 HAW Hamburg
 *               2020 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_at86rf233
 * @{
 * @file
 * @brief       Additional types for the at86rf233 driver
 *
 * @author      Alaeddine Weslati <alaeddine.weslati@inria.fr>
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Martine Lenders <mlenders@inf.fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Sebastian Meiling <s@mlng.net>
 * @author      Baptiste Clenet <bapclenet@gmail.com>
 * @author      Daniel Krebs <github@daniel-krebs.net>
 * @author      Kévin Roussel <Kevin.Roussel@inria.fr>
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 * @author      Josua Arndt <jarndt@ias.rwth-aachen.de>
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef AT86RF233_TYPES_H
#define AT86RF233_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief at86rf233 phy mode
 */
typedef enum at86rf233_phy_mode {
    AT86RF233_OQPSK_250_SCR_ON      = (0b00100000),
    AT86RF233_OQPSK_500_SCR_ON      = (0b00100001),
    AT86RF233_OQPSK_1000_SCR_ON     = (0b00100010),
    AT86RF233_OQPSK_2000_SCR_ON     = (0b00100011),
    AT86RF233_OQPSK_250_SCR_OFF     = (0b00000000),
    AT86RF233_OQPSK_500_SCR_OFF     = (0b00000001),
    AT86RF233_OQPSK_1000_SCR_OFF    = (0b00000010),
    AT86RF233_OQPSK_2000_SCR_OFF    = (0b00000011)
} at86rf233_phy_mode_t;

/**
 * @brief Frame buffer structure of at86rf233
 */
typedef struct at86rf233_fb {
    uint8_t phr;        /**< PHR */
    uint8_t *payload;   /**< Payload pointer */
    uint8_t fcs[2];     /**< checksum */
    uint8_t lqi;        /**< LQI */
    uint8_t ed;         /**< ED */
    uint8_t rx_status;  /**< Rx status */
} at86rf233_fb_t;

#ifdef __cplusplus
}
#endif

#endif /* AT86RF233_TYPES_H */
/** @} */
