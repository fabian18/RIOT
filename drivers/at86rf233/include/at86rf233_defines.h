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
 * @brief       Transceiver constants for at86rf233
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
#ifndef AT86RF233_DEFINES_H
#define AT86RF233_DEFINES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Constats for at86rf233
 * @{
 */
#define AT86RF233_PART_NUM                      (0x0B)
#define AT86RF233_VERSION_NUM_A                 (0x01)
#define AT86RF233_VERSION_NUM_B                 (0x02)
#define AT86RF233_MAN_ID_0                      (0x1F)
#define AT86RF233_MAN_ID_1                      (0x00)
#define AT86RF233_WAKEUP_DELAY_US               (360U)
#define AT86RF233_RESET_DELAY_US                (26U)
#define AT86RF233_RESET_PULSE_WIDTH_US          (1U)    /* 625 ns */
#define AT86RF233_SLP_TR_PULSE_WIDTH_US         (1U)    /* 62.5 ns */

#define AT86RF233_RX_SENSITIVITY_MIN_DBM        (-101)
#define AT86RF233_RX_SENSITIVITY_MAX_DBM        (-52)
#define AT86RF233_RX_SENSITIVITY_DEFAULT_DBM    (-101)
#define AT86RF233_TX_POWER_MIN_DBM              (-17)
#define AT86RF233_TX_POWER_MAX_DBM              (4)
#define AT86RF233_TX_POWER_DEFAULT_DBM          (4)
#define AT86RF233_CHANNEL_MIN                   (11)
#define AT86RF233_CHANNEL_DEFAULT               (11)
#define At86RF233_CHANNEL_MAX                   (26)
#define AT86RF233_RSSI_BASE_VAL                 (-94)
#define AT86RF233_MAX_FRAME_RETRIES_DEFAULT     (3)
#define AT86RF233_MAX_CSMA_RETRIES_DEFAULT      (4)
#define AT86RF233_CSMA_MIN_BE_DEFAULT           (3)
#define AT86RF233_CSMA_MAX_BE_DEFAULT           (5)
#define AT86RF233_PHY_DATA_RATE_KBPS_DEFAULT    (250)
#define AT86RF233_PAN_ID_DEFAULT                { 0xFF, 0xFF }
#define AT86RF233_SHORT_ADDR_DEFAULT            { 0xFF, 0xFF }
#define AT86RF233_LONG_ADDR_DEFAULT             { 0x00, 0x00,     \
                                                  0x00, 0x00,     \
                                                  0x00, 0x00,     \
                                                  0x00, 0x00 }
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AT86RF233_DEFINES_H */
/** @} */
