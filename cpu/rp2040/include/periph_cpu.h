/*
 * Copyright (C) 2015-2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_rp2040
 * @{
 *
 * @file
 * @brief           RP2040 specific definitions for handling peripherals
 *
 * @author          Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 * @author          Fabian Hüßler <fabian.huessler@ovgu.de>
 */

#ifndef PERIPH_CPU_H
#define PERIPH_CPU_H

#include "cpu.h"
#include "vendor/RP2040.h"
#include "reg_atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   System core clock speed is fixed to 125MHz
 */
#define CLOCK_CORECLOCK     (125000000U)

/**
 * @brief   Peripherie blocks that can be reset
 */
#define RESETS_RESET_MASK               \
    (RESETS_RESET_usbctrl_Msk       |   \
     RESETS_RESET_uart1_Msk         |   \
     RESETS_RESET_uart0_Msk         |   \
     RESETS_RESET_timer_Msk         |   \
     RESETS_RESET_tbman_Msk         |   \
     RESETS_RESET_sysinfo_Msk       |   \
     RESETS_RESET_syscfg_Msk        |   \
     RESETS_RESET_spi1_Msk          |   \
     RESETS_RESET_spi0_Msk          |   \
     RESETS_RESET_rtc_Msk           |   \
     RESETS_RESET_pwm_Msk           |   \
     RESETS_RESET_pll_usb_Msk       |   \
     RESETS_RESET_pll_sys_Msk       |   \
     RESETS_RESET_pio1_Msk          |   \
     RESETS_RESET_pio0_Msk          |   \
     RESETS_RESET_pads_qspi_Msk     |   \
     RESETS_RESET_pads_bank0_Msk    |   \
     RESETS_RESET_jtag_Msk          |   \
     RESETS_RESET_io_qspi_Msk       |   \
     RESETS_RESET_io_bank0_Msk      |   \
     RESETS_RESET_i2c1_Msk          |   \
     RESETS_RESET_i2c0_Msk          |   \
     RESETS_RESET_dma_Msk           |   \
     RESETS_RESET_busctrl_Msk       |   \
     RESETS_RESET_adc_Msk)

/**
 * @brief   Reset hardware components
 *
 * @param   components bitmask of components to be reset,
 *          @see RESETS_RESET_MASK
 */
static inline void periph_reset(uint32_t components)
{
    reg_atomic_set(&RESETS->RESET, components);
}
/**
 * @brief   Waits until hardware components have been reset
 *
 * @param   components bitmask of components that must have reset,
 *          @see RESETS_RESET_MASK
 */
static inline void periph_reset_done(uint32_t components)
{
    reg_atomic_clear(&RESETS->RESET, components);
    while ((~RESETS->RESET_DONE.reg) & components) {
        ;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_H */
/** @} */
