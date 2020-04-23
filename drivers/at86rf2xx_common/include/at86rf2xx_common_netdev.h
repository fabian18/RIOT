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
 * @ingroup     drivers_at86rf2xx_common
 * @{
 *
 * @file
 * @brief       Common netdev interface of at86rf2xx transceivers
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
#ifndef AT86RF2XX_COMMON_NETDEV_H
#define AT86RF2XX_COMMON_NETDEV_H

#include "at86rf2xx_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Internal device option flags to control the netdev driver
 *          behaviour
 * @{
 */
#define AT86RF2XX_OPT_TELL_TX_START     (0x0001)    /**< notify MAC layer on TX start */
#define AT86RF2XX_OPT_TELL_TX_END       (0x0002)    /**< notify MAC layer on TX finished */
#define AT86RF2XX_OPT_TELL_RX_START     (0x0004)    /**< notify MAC layer on RX start */
#define AT86RF2XX_OPT_TELL_RX_END       (0x0008)    /**< notify MAC layer on RX finished */
#define AT86RF2XX_OPT_PRELOADING        (0x0010)    /**< preloading enabled pending */
#define AT86RF2XX_OPT_AUTOCCA           (0x0020)    /**< CSMA active (en. TX_ARET_ON) */
#define AT86RF2XX_OPT_AUTOACK           (0x0040)    /**< auto ACK active (en. RX_AACK_ON) */
#define AT86RF2XX_OPT_PROMISCUOUS       (0x0080)    /**< promiscuous mode is active */
/** @} */

/**
 * @name    Internal device flags
 * @{
 */
#define AT86RF2XX_FLG_PENDING_IRQ       (0x8000)    /**< IRQ is pending */
#define AT86RF2XX_FLG_DEEP_SLEEP        (0x4000)    /**< Device is in deep sleep state */
/** @} */

/**
 * @brief   Rx state: Either AT86RF2XX_STATE_RX_AACK_ON or AT86RF2XX_STATE_RX_ON
 */
#ifndef AT86RF2XX_PHY_STATE_RX
#define AT86RF2XX_PHY_STATE_RX                                  \
    ((dev->base.flags & AT86RF2XX_OPT_AUTOACK)                  \
        ? AT86RF2XX_STATE_RX_AACK_ON : AT86RF2XX_STATE_RX_ON)
#endif

/**
 * @brief Tx state: Either AT86RF2XX_STATE_TX_ARET_ON or AT86RF2XX_STATE_PLL_ON
 */
#ifndef AT86RF2XX_PHY_STATE_TX
#define AT86RF2XX_PHY_STATE_TX                                  \
    ((dev->base.flags & AT86RF2XX_OPT_AUTOCCA)                  \
        ? AT86RF2XX_STATE_TX_ARET_ON : AT86RF2XX_STATE_PLL_ON)
#endif

/**
 * @brief   Convert at86rf2xx state to netopt state
 *
 * @param[in] dev   Device
 * @param[in] state at86rf2xx state to be converted to netopt state
 *
 * @return  netopt state
 */
netopt_state_t at86rf2xx_state_to_netopt(const at86rf2xx_t *dev, uint8_t state);

/**
 * @brief   Convert netopt state state to at86rf2xx
 *
 * @param[in] dev   Device
 * @param[in] state  netopt state to be converted to at86rf2xx state
 *
 * @return  at86rf2xx state
 */
uint8_t at86rf2xx_netopt_to_state(const at86rf2xx_t *dev, netopt_state_t state);

/**
 * @brief   Get common at86rf2xx properties
 *
 * The calling function must wake up the transceiver, if neccessary for @p opt.
 *
 * @param[in] dev   at86rf2xx device
 * @param[in] opt   Netdev option
 * @param[out] val  Output value of type according to @p opt
 * @param[in] max_len   Maximim size of output value
 *
 * @return  Number of bytes written to @p val on success
 * @return  -ENOTSUP if the transceiver does not support getting @p opt
 * @return  Negative integer on error
 */
int at86rf2xx_netdev_get(at86rf2xx_t *dev, netopt_t opt, void *val, size_t max_len);

/**
 * @brief   Set common at86rf2xx properties
 *
 * The calling function must wake up the transceiver, if neccessary for @p opt.
 *
 * @param[in] dev   at86rf2xx device
 * @param[in] opt   Netdev option
 * @param[out] val  Input value of type according to @p opt
 * @param[in] len   Size of input value
 *
 * @return  @p len on success
 * @return  -ENOTSUP if the transceiver does not support setting @p opt
 * @return  Negative integer on error
 */
int at86rf2xx_netdev_set(at86rf2xx_t *dev, netopt_t opt, const void *val, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* AT86RF2XX_COMMON_NETDEV_H */
/** @} */
