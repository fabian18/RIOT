/*
 * Copyright (C) 2021 Otto-von-Guericke Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_rp2040
 * @{
 *
 * @file
 * @brief       Implementation of the crystal oscillator (XOSC)
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include <assert.h>

#include "macros/units.h"
#include "vendor/RP2040.h"
#include "reg_atomic.h"

static inline uint32_t _xosc_conf_sartup_delay(uint32_t f_crystal_mhz, uint32_t t_stable_ms)
{
    return (((f_crystal_mhz / 1000) * t_stable_ms) + 128) / 256;
}

/**
 * @brief   Configues the Crstal to run.
 *          Should be configured to 12 MHz which is the default as
 *          described in the hardware manual.
 *          The minimum is 1 MHz and the maximum is 15 MHz.
 *
 * @param   f_ref       Desired frequency
 */
void xosc_start(uint32_t f_ref)
{
    assert(f_ref == MHZ(12));
    uint32_t delay = _xosc_conf_sartup_delay(f_ref, 1);
    reg_atomic_assign(&XOSC->STARTUP,
                      delay << XOSC_STARTUP_DELAY_Pos,
                      XOSC_STARTUP_DELAY_Msk);
    reg_atomic_assign(&XOSC->CTRL,
                      XOSC_CTRL_ENABLE_ENABLE << XOSC_CTRL_ENABLE_Pos,
                      XOSC_CTRL_ENABLE_Msk);
    while (!XOSC->STATUS.bit.STABLE) {
        ;
    }
}
/**
 * @brief   Stop the crytal.
 */
void xosc_stop(void)
{
    reg_atomic_assign(&XOSC->CTRL,
                      XOSC_CTRL_ENABLE_DISABLE << XOSC_CTRL_ENABLE_Pos,
                      XOSC_CTRL_ENABLE_Msk);
}
