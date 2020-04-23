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
 * @brief       Interface of at86rf2xx state mashine
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
#ifndef AT86RF2XX_COMMON_STATES_H
#define AT86RF2XX_COMMON_STATES_H

#include "at86rf2xx_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Flags for device internal states (see datasheet)
 * @{
 */
#define AT86RF2XX_STATE_P_ON            (0x00)  /**< initial power on */
#define AT86RF2XX_STATE_BUSY_RX         (0x01)  /**< busy receiving data (basic mode) */
#define AT86RF2XX_STATE_BUSY_TX         (0x02)  /**< busy transmitting data (basic mode) */
#define AT86RF2XX_STATE_FORCE_TRX_OFF   (0x03)  /**< force transition to idle */
#define AT86RF2XX_STATE_RX_ON           (0x06)  /**< listen mode (basic mode) */
#define AT86RF2XX_STATE_TRX_OFF         (0x08)  /**< idle */
#define AT86RF2XX_STATE_PLL_ON          (0x09)  /**< ready to transmit */
#define AT86RF2XX_STATE_SLEEP           (0x0f)  /**< sleep mode */
#define AT86RF2XX_STATE_PREP_DEEP_SLEEP (0x10)  /**< prepare deep sleep */
#define AT86RF2XX_STATE_BUSY_RX_AACK    (0x11)  /**< busy receiving data (extended mode) */
#define AT86RF2XX_STATE_BUSY_TX_ARET    (0x12)  /**< busy transmitting data (extended mode) */
#define AT86RF2XX_STATE_RX_AACK_ON      (0x16)  /**< wait for incoming data */
#define AT86RF2XX_STATE_TX_ARET_ON      (0x19)  /**< ready for sending data */
#define AT86RF2XX_STATE_IN_PROGRESS     (0x1f)  /**< ongoing state conversion */
/** @} */

/**
 * @brief   Express any receiver state
 *
 * @param[in]   state   State
 *
 * @return  Whether the transceiver is in a receiver state
 */
static inline
bool at86rf2xx_is_rx_state(uint8_t state)
{
    return state == AT86RF2XX_STATE_BUSY_RX         ||
           state == AT86RF2XX_STATE_RX_ON           ||
           state == AT86RF2XX_STATE_BUSY_RX_AACK    ||
           state == AT86RF2XX_STATE_RX_AACK_ON;
}

/**
 * @brief   Express any transmitter state
 *
 * @param[in]   state   State
 *
 * @return  Whether the transciever is in a transmitter state
 */
static inline
bool at86rf2xx_is_tx_state(uint8_t state)
{
    return state == AT86RF2XX_STATE_BUSY_TX         ||
           state == AT86RF2XX_STATE_PLL_ON          ||
           state == AT86RF2XX_STATE_BUSY_TX_ARET    ||
           state == AT86RF2XX_STATE_TX_ARET_ON;
}

/**
 * @brief   Express any busy state
 *
 * @param[in]   state   State
 *
 * @return  Whether the transciever is in a busy state
 */
static inline
bool at86rf2xx_is_busy_state(uint8_t state)
{
    return state == AT86RF2XX_STATE_BUSY_RX         ||
           state == AT86RF2XX_STATE_BUSY_RX_AACK    ||
           state == AT86RF2XX_STATE_BUSY_TX         ||
           state == AT86RF2XX_STATE_BUSY_TX_ARET;
}

/**
 * @brief   Perform initial state transition from P_ON to TRX_OFF
 *
 * @param[in]   dev         device to power on
 */
void at86rf2xx_power_on(at86rf2xx_t * dev);

/**
 * @brief   Convenience function for reading the status of the given device
 *
 * @param[in] dev       device to read the status from
 *
 * @return              internal status of the given device
 */
uint8_t at86rf2xx_get_state(const at86rf2xx_t *dev);

/**
 * @brief   Set the state of the given device (trigger a state change)
 *
 * @param[in,out] dev       device to change state of
 * @param[in] cmd           state change command
 *
 * @return                  the previous state before the new state was set
 */
uint8_t at86rf2xx_set_state(at86rf2xx_t *dev, uint8_t cmd);

#ifdef __cplusplus
}
#endif

#endif /* AT86RF2XX_COMMON_STATES_H */
/** @} */
