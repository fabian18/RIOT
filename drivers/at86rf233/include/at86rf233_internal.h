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
 *
 * @file
 * @brief       Internal functions of the at86rf233 transceiver driver
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
#ifndef AT86RF233_INTERNAL_H
#define AT86RF233_INTERNAL_H

#include "at86rf233_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Adjust transmitter power of an at86rf233
 *
 * @param[in] dev       Device to be configured
 * @param[in] tx_power  Power in dBm
 */
void at86rf233_set_tx_power(at86rf233_t *dev, int8_t tx_power);

/**
 * @brief   Read the curently configured transmitter power
 *          of an at86rf233
 *
 * @param[in] dev       Device to read from
 *
 * @return  Transmitetr power in dBm
 */
int8_t at86rf233_get_tx_power(const at86rf233_t *dev);

/**
 * @brief   Adjust receiver sensibility of an at86rf233
 *
 * @param[in] dev       Device to be configured
 * @param[in] rx_sens   Sensibility in dBm
 */
void at86rf233_set_rx_sensibility(at86rf233_t *dev, int8_t rx_sens);

/**
 * @brief   Read the currently configured receiver sensibility
 *          of an at86rf233
 *
 * @param[in] dev       Device to read from
 *
 * @return  Sensibility in dBm
 */
int8_t at86rf233_get_rx_sensesibility(const at86rf233_t *dev);

/**
 * @brief   Read the currently configured CCA threshold of an at86rf233,
 *          indicating a busy channel
 *
 * @param[in] dev       Device to read from
 *
 * @return  CCA threshold in dBm
 */
int8_t at86rf233_get_cca_threshold(const at86rf233_t *dev);

/**
 * @brief   Configure the CCA threshold of an at86rf233,
 *          indicating a busy channel
 *
 * @param[in] dev       Device to be configured
 * @param[in] thresh    threshold value in dBm
 */
void at86rf233_set_cca_threshold(const at86rf233_t *dev, int8_t thresh);

/**
 * @brief   Configure physical layer modulation of an at86rf233
 *
 * @param[in] dev       Device to be configured
 * @param[in] mde       PHY mode
 */
void at86rf233_configure_phy(at86rf233_t *dev, at86rf233_phy_mode_t mode);

/**
 * @brief       Read PHR from the frame buffer of an at86rf233
 *
 * @param[in] dev       Device to read from
 *
 * @return      PHR
 */
uint8_t at86rf233_fb_read_phr(const at86rf233_t *dev);

/**
 * @brief   Read the content of the frame buffer of an at86rf233
 *
 * @param[in] dev       Device to read from
 * @param[out] fb       Parsed framebuffer content
 * @param[out] buf      Buffer to write frame buffer content to
 * @param[in] buf_size  Buffer size
 *
 * @return  Number of bytes written to @p buf
 */
ssize_t at86rf233_fb_read(const at86rf233_t *dev, at86rf233_fb_t *fb,
                          void *buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif /* AT86RF233_INTERNAL_H */
/** @} */
