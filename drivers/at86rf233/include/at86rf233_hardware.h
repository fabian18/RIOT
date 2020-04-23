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
 * @brief       Hardware related functions for at86rf233
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
#ifndef AT86RF233_HARDWARE_H
#define AT86RF233_HARDWARE_H

#include "at86rf233.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Put at86rf233 to sleep
 *
 * @param[in] dev   Device to be put to sleep
 */
void at86rf233_sleep(at86rf233_t *dev);

/**
 * @brief   Put at86rf233 to deep sleep
 *
 * @param[in] dev   Device to be put to deep sleep
 */
void at86rf233_deep_sleep(at86rf233_t *dev);

/**
 * @brief   Requiere an at86rf233 to not be sleeping
 *
 * @param[in] dev   Device to be woken ip, if neccessary
 */
void at86rf233_assert_awake(at86rf233_t *dev);

/**
 * @brief   Trigger a hardware reset of an at86rf233
 *
 * @param[in] dev   Device to be reset
 */
void at86rf233_hardware_reset(at86rf233_t *dev);

/**
 * @brief   Trigger sending of a loaded framebuffer of an at86rf233
 *
 * @param[in] dev   Device to be triggered
 */
void at86rf233_trigger_send_gpio(at86rf233_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* AT86RF233_HARDWARE */
/** @} */