/* Copyright (C) 2013 Alaeddine Weslati <alaeddine.weslati@inria.fr>
 *               2015 Freie Universität Berlin
 *               2015 Kaspar Schleiser <kaspar@schleiser.de>
 *               2015 Hauke Petersen <hauke.petersen@fu-berlin.de>
 *               2017 HAW Hamburg
 *               2020 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     drivers_at86rf2xx_common
 * @{
 *
 * @file
 * @brief       Internal interface for at86rf2xx drivers
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

#ifndef AT86RF2XX_COMMON_INTERNAL_H
#define AT86RF2XX_COMMON_INTERNAL_H

#include "at86rf2xx_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Common CSMA/CA constants
 * @{
 */
#define AT86RF2XX_MAX_CSMA_RETRIES          (5)
#define AT86RF2XX_MAX_FRAME_RETRIES         (7)
#define AT86RF2XX_MAX_BE                    (8)
#define AT86RF2XX_MIN_BE                    (0)
/** @} */

/**
 * @name    Timing values
 * @{
 */
#define AT86RF2XX_TIMING__VCC_TO_P_ON                           (330)
#define AT86RF2XX_TIMING__SLEEP_TO_TRX_OFF                      (380)
#define AT86RF2XX_TIMING__TRX_OFF_TO_PLL_ON                     (110)
#define AT86RF2XX_TIMING__TRX_OFF_TO_RX_ON                      (110)
#define AT86RF2XX_TIMING__PLL_ON_TO_BUSY_TX                     (16)
#define AT86RF2XX_TIMING__RESET                                 (100)
#define AT86RF2XX_TIMING__RESET_TO_TRX_OFF                      (37)
/** @} */

/**
 * @brief   Get the short address of the given device
 *
 * @param[in]   dev         device to read from
 * @param[out]  addr        the short address will be stored here
 *
 * @return                  the currently set (2-byte) short address
 */
void at86rf2xx_get_addr_short(const at86rf2xx_t *dev, network_uint16_t *addr);

/**
 * @brief   Set the short address of the given device
 *
 * @param[in,out] dev       device to write to
 * @param[in] addr          (2-byte) short address to set
 */
void at86rf2xx_set_addr_short(at86rf2xx_t *dev, const network_uint16_t *addr);

/**
 * @brief   Get the configured long address of the given device
 *
 * @param[in]   dev         device to read from
 * @param[out]  addr        the long address will be stored here
 *
 * @return                  the currently set (8-byte) long address
 */
void at86rf2xx_get_addr_long(const at86rf2xx_t *dev, eui64_t *addr);

/**
 * @brief   Set the long address of the given device
 *
 * @param[in,out] dev       device to write to
 * @param[in] addr          (8-byte) long address to set
 */
void at86rf2xx_set_addr_long(at86rf2xx_t *dev, const eui64_t *addr);

/**
 * @brief   Auto init short and long addresses
 *
 * @param[in] dev   device to write addresses to
 */
void at86rf2xx_address_init_auto(at86rf2xx_t *dev);

/**
 * @brief   Get the configured PAN ID of the given device
 *
 * @param[in] dev           device to read from
 *
 * @return                  the currently set PAN ID
 */
uint16_t at86rf2xx_get_pan(const at86rf2xx_t *dev);

/**
 * @brief   Set the PAN ID of the given device
 *
 * @param[in,out] dev       device to write to
 * @param[in] pan           PAN ID to set
 */
void at86rf2xx_set_pan(at86rf2xx_t *dev, uint16_t pan);

/**
 * @brief   Get the configured channel number of the given device
 *
 * @param[in] dev           device to read from
 *
 * @return                  the currently set channel number
 */
uint8_t at86rf2xx_get_channel(const at86rf2xx_t *dev);

/**
 * @brief   Set PHY chennel
 *
 * @param[in] dev       device
 * @param[in] channel   channel
 */
void at86rf2xx_set_channel(at86rf2xx_t *dev, uint8_t channel);

/**
 * @brief   Read frame retransmission counter
 *
 * @param[in] dev           device from which to read retransmission counter
 *
 * @return                  retransmission counter
 */
uint8_t at86rf2xx_get_frame_retries(const at86rf2xx_t *dev);

/**
 * @brief   Set maximum number of frame retransmissions
 *
 * @param[in]   dev         device
 * @param[in]   retries     number of retransmissions
 */
void at86rf2xx_set_frame_max_retries(at86rf2xx_t *dev, uint8_t retries);

/**
 * @brief   Read channel access retries
 *
 * @param[in] dev           device from which to read CSMA retries
 *
 * @return                  CSMA retries
 */
uint8_t at86rf2xx_get_csma_retries(const at86rf2xx_t *dev);

/**
 * @brief   Set the maximum number of channel access attempts per frame (CSMA)
 *
 * This setting specifies the number of attempts to access the channel to
 * transmit a frame. If the channel is busy @p retries times, then frame
 * transmission fails.
 * Valid values: 0 to 5, -1 means CSMA disabled
 * If @p retries > 5, the maximum value 5 is taken.
 *
 * @param[in] dev           device to write to
 * @param[in] retries       the maximum number of retries
 */
void at86rf2xx_set_csma_max_retries(const at86rf2xx_t *dev, int8_t retries);

/**
 * @brief   Get the maximum number of channel access attempts per frame (CSMA)
 *
 * @param[in] dev           device to read from
 *
 * @return                  configured number of retries
 */
uint8_t at86rf2xx_get_csma_max_retries(const at86rf2xx_t *dev);

/**
 * @brief   Retrieve maximum and minimum CSMA/CA backoff exponent
 *
 * @param[in] dev   device to read from
 * @param[out] min  minimum BE
 * @param[out] max  maximum BE
 */
void at86rf2xx_get_csma_backoff_exp(const at86rf2xx_t *dev,
                                    uint8_t *min, uint8_t *max);

/**
 * @brief   Set the min and max backoff exponent for CSMA/CA
 *
 * - Maximum BE: 0 - 8
 * - Minimum BE: 0 - [max]
 *
 * @param[in] dev           device to write to
 * @param[in] min           the minimum BE
 * @param[in] max           the maximum BE
 */
void at86rf2xx_set_csma_backoff_exp(const at86rf2xx_t *dev,
                                    uint8_t min, uint8_t max);

/**
 * @brief   Set seed for CSMA random backoff
 *
 * @param[in] dev           device to write to
 * @param[in] entropy       11 bit of entropy as seed for random backoff
 */
void at86rf2xx_set_csma_seed(const at86rf2xx_t *dev, const uint8_t entropy[2]);

/**
 * @brief   Perform one manual channel clear assessment (CCA)
 *
 * The CCA mode and threshold level depends on the current transceiver settings.
 *
 * @param[in]  dev          device to use
 *
 * @return                  true if channel is determined clear
 * @return                  false if channel is determined busy
 */
bool at86rf2xx_cca(at86rf2xx_t *dev);

/**
 * @brief   Trigger a transmission while writing TX_START to TRX_CMD
 *
 * @param[in] dev   Device to be trigegred
 */
void at86rf2xx_trigger_send(at86rf2xx_t *dev);

/**
 * @brief   Read random data from the RNG
 *
 * @note    According to the data sheet this function only works properly in
 *          Basic Operation Mode. However, sporadic testing has shown that even
 *          in Extended Operation Mode this returns random enough data to be
 *          used as a seed for @ref sys_random if no cryptographically secure
 *          randomness is required.
 *          Any further use-case needs to be evaluated, especially if
 *          crypto-relevant randomness is required.
 *
 * @param[in] dev       device to configure
 * @param[out] data     buffer to copy the random data to
 * @param[in]  len      number of random bytes to store in data
 */
void at86rf2xx_get_random(const at86rf2xx_t *dev, uint8_t *data, size_t len);

#endif /* AT86RF2XX_COMMON_INTERNAL_H */
/** @} */
