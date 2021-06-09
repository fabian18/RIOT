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
 * @brief       Implementation of the CPU clock configuration
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "vendor/RP2040.h"
#include "vendor/system_RP2040.h"
#include "reg_atomic.h"

/**
 * @brief   Configure the system clock to run from the reference clock,
 *          which is the default on boot
 *
 * @param   f_in        Input frequency of the reference clock
 * @param   f_out       Output frequency of the system clock
 * @param   source      Clock source
 */
void clock_sys_configure_source(uint32_t f_in, uint32_t f_out,
                                CLOCKS_CLK_SYS_CTRL_SRC_Enum source)
{
    assert(f_out <= f_in);
    assert(source != CLOCKS_CLK_SYS_CTRL_SRC_clksrc_clk_sys_aux);
    uint32_t div = (((uint64_t)f_in) << CLOCKS_CLK_SYS_DIV_INT_Pos) / f_out;
    /* switch the glitchless mux to clk_ref */
    reg_atomic_assign(&CLOCKS->CLK_SYS_CTRL,
                      source << CLOCKS_CLK_SYS_CTRL_SRC_Pos,
                      CLOCKS_CLK_SYS_CTRL_SRC_Msk);
    /* apply divider */
    reg_atomic_assign(&CLOCKS->CLK_SYS_DIV,
                      div,
                      CLOCKS_CLK_SYS_DIV_INT_Msk | CLOCKS_CLK_SYS_DIV_FRAC_Msk);
    /* poll SELECTED until the switch is completed */
    while (!(CLOCKS->CLK_SYS_SELECTED & (1U << source))) {
        ;
    }
    /* update SystemCoreClock variable */
    SystemCoreClockUpdate();
}
/**
 * @brief   Configure the system clock to run from an auxiliary clcok source,
 *          like PLL
 *
 * @note    The auxiliary must have been configured beforehand
 *
 * @param   f_in        Input frequency of the auxiliary clcok source
 * @param   f_out       Output frequency of the system clock
 * @param   aux         Which auxiliary clock source to use
 */
void clock_sys_configure_aux_source(uint32_t f_in, uint32_t f_out,
                                    CLOCKS_CLK_SYS_CTRL_AUXSRC_Enum aux)
{
    assert(f_out <= f_in);
    uint32_t div = (((uint64_t)f_in) << CLOCKS_CLK_SYS_DIV_INT_Pos) / f_out;
    /* switch the glitchless mux to a source that is not the aux mux */
    reg_atomic_assign(&CLOCKS->CLK_SYS_CTRL,
                      CLOCKS_CLK_SYS_CTRL_SRC_clk_ref << CLOCKS_CLK_SYS_CTRL_SRC_Pos,
                      CLOCKS_CLK_SYS_CTRL_SRC_Msk);
    /* poll SELECTED until the switch is completed */
    while (!(CLOCKS->CLK_SYS_SELECTED & (1U << CLOCKS_CLK_SYS_CTRL_SRC_clk_ref))) {
        ;
    }
    /* change the auxiliary mux */
    reg_atomic_assign(&CLOCKS->CLK_SYS_CTRL,
                      aux << CLOCKS_CLK_SYS_CTRL_AUXSRC_Pos,
                      CLOCKS_CLK_SYS_CTRL_AUXSRC_Msk);
    /* apply divider */
    reg_atomic_assign(&CLOCKS->CLK_SYS_DIV,
                      div,
                      CLOCKS_CLK_SYS_DIV_INT_Msk | CLOCKS_CLK_SYS_DIV_FRAC_Msk);
    /* switch the glitchless mux to clk_sys_aux */
    reg_atomic_assign(&CLOCKS->CLK_SYS_CTRL,
                      CLOCKS_CLK_SYS_CTRL_SRC_clksrc_clk_sys_aux << CLOCKS_CLK_SYS_CTRL_SRC_Pos,
                      CLOCKS_CLK_SYS_CTRL_SRC_Msk);
    /* poll SELECTED until the switch is completed */
    while (!(CLOCKS->CLK_SYS_SELECTED & (1U << CLOCKS_CLK_SYS_CTRL_SRC_clksrc_clk_sys_aux))) {
        ;
    }
    /* update SystemCoreClock variable */
    SystemCoreClockUpdate();
}
/**
 * @brief   Configure the reference clock to run from a clock source,
 *          which is either the ROSC or the XOSC
 *
 * @note    Make sure that ROSC or XOSC are properly set up
 *
 * @param   f_in        Input frequency of the reference clock
 * @param   f_out       Output frequency of the system clock
 * @param   source      Clock source
 */
void clcok_ref_configure_source(uint32_t f_in, uint32_t f_out,
                                CLOCKS_CLK_REF_CTRL_SRC_Enum source)
{
    assert(f_out <= f_in);
    assert(source != CLOCKS_CLK_REF_CTRL_SRC_clksrc_clk_ref_aux);
    uint32_t div = (((uint64_t)f_in) << CLOCKS_CLK_REF_DIV_INT_Pos) / f_out;
    /* switch the glitchless mux to clock source */
    reg_atomic_assign(&CLOCKS->CLK_REF_CTRL,
                      source << CLOCKS_CLK_REF_CTRL_SRC_Pos,
                      CLOCKS_CLK_REF_CTRL_SRC_Msk);
    /* apply divider */
    reg_atomic_assign(&CLOCKS->CLK_REF_DIV,
                      div,
                      CLOCKS_CLK_REF_DIV_INT_Msk);
    /* poll SELECTED until the switch is completed */
    while(!(CLOCKS->CLK_REF_SELECTED & (1U << source))) {
        ;
    }
}
/**
 * @brief   Configure the reference clock to run from an auxiliary clcok source,
 *          like PLL
 *
 * @note    The auxiliary must have been configured beforehand
 *
 * @param   f_in        Input frequency of the auxiliary clcok source
 * @param   f_out       Output frequency of the reference clock
 * @param   aux         Which auxiliary clock source to use
 */
void clcok_ref_configure_aux_source(uint32_t f_in, uint32_t f_out,
                                    CLOCKS_CLK_REF_CTRL_AUXSRC_Enum aux)
{
    assert(f_out <= f_in);
    uint32_t div = (((uint64_t)f_in) << CLOCKS_CLK_REF_DIV_INT_Pos) / f_out;
    /* switch the glitchless mux to a source that is not the aux mux */
    reg_atomic_assign(&CLOCKS->CLK_REF_CTRL,
                      CLOCKS_CLK_REF_CTRL_SRC_rosc_clksrc_ph << CLOCKS_CLK_REF_CTRL_SRC_Pos,
                      CLOCKS_CLK_REF_CTRL_SRC_Msk);
    /* poll SELECTED until the switch is completed */
    while (!(CLOCKS->CLK_REF_SELECTED & (1U << CLOCKS_CLK_REF_CTRL_SRC_rosc_clksrc_ph))) {
        ;
    }
    /* change the auxiliary mux */
    reg_atomic_assign(&CLOCKS->CLK_REF_CTRL,
                      aux << CLOCKS_CLK_REF_CTRL_AUXSRC_Pos,
                      CLOCKS_CLK_REF_CTRL_AUXSRC_Msk);
    /* apply divider */
    reg_atomic_assign(&CLOCKS->CLK_REF_DIV,
                      div,
                      CLOCKS_CLK_REF_DIV_INT_Msk);
    /* switch the glitchless mux to clk_ref_aux */
    reg_atomic_assign(&CLOCKS->CLK_REF_CTRL,
                      CLOCKS_CLK_REF_CTRL_SRC_clksrc_clk_ref_aux << CLOCKS_CLK_REF_CTRL_SRC_Pos,
                      CLOCKS_CLK_REF_CTRL_SRC_Msk);
    /* poll SELECTED until the switch is completed */
    while (!(CLOCKS->CLK_REF_SELECTED & (1U << CLOCKS_CLK_REF_CTRL_SRC_clksrc_clk_ref_aux))) {
        ;
    }
}
/**
 * @brief   Configure the peripheral clock to run from a dedicated auxiliary
 *          clock source
 *
 * @param   aux     Auxiliary clock source
 */
void clock_peri_configure(CLOCKS_CLK_PERI_CTRL_AUXSRC_Enum aux)
{
    reg_atomic_assign(&CLOCKS->CLK_PERI_CTRL,
                      aux << CLOCKS_CLK_PERI_CTRL_AUXSRC_Pos,
                      CLOCKS_CLK_PERI_CTRL_AUXSRC_Msk);
    reg_atomic_set(&CLOCKS->CLK_PERI_CTRL,
                   (1u << CLOCKS_CLK_ADC_CTRL_ENABLE_Pos));
}
/**
 * @brief   Configure gpio21 as clock output pin
 *
 * @details Can be used as an external clock source for another circuit or
 *          to check the expected signal with a logic analyzer
 *
 * @param   f_in        Input frequency
 * @param   f_out       Output frequency
 * @param   aux         Auxiliary clock source
 */
void clock_gpout0_configure(uint32_t f_in, uint32_t f_out,
                            CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_Enum aux)
{
    assert(f_out <= f_in);
    uint32_t div = (((uint64_t)f_in) << CLOCKS_CLK_REF_DIV_INT_Pos) / f_out;
    reg_atomic_assign(&CLOCKS->CLK_GPOUT0_CTRL,
                      aux << CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_Pos,
                      CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_Msk);
    reg_atomic_assign(&CLOCKS->CLK_GPOUT0_DIV,
                      div,
                      CLOCKS_CLK_GPOUT0_DIV_INT_Msk | CLOCKS_CLK_GPOUT0_DIV_FRAC_Msk);
    reg_atomic_set(&CLOCKS->CLK_GPOUT0_CTRL,
                   1U << CLOCKS_CLK_GPOUT0_CTRL_ENABLE_Pos);
    reg_atomic_assign(&PADS_BANK0->GPIO21,
                      1U << PADS_BANK0_GPIO21_IE_Pos,
                      PADS_BANK0_GPIO21_IE_Msk | PADS_BANK0_GPIO21_OD_Msk);
    reg_atomic_assign(&IO_BANK0->GPIO21_CTRL,
                      IO_BANK0_GPIO21_CTRL_FUNCSEL_clocks_gpout_0 << IO_BANK0_GPIO21_CTRL_FUNCSEL_Pos,
                      IO_BANK0_GPIO21_CTRL_FUNCSEL_Msk);
}
/**
 * @brief   Configure gpio23 as clock output pin
 *
 * @details Can be used as an external clock source for another circuit or
 *          to check the expected signal with a logic analyzer
 *
 * @param   f_in        Input frequency
 * @param   f_out       Output frequency
 * @param   aux         Auxiliary clock source
 */
void clock_gpout1_configure(uint32_t f_in, uint32_t f_out,
                            CLOCKS_CLK_GPOUT1_CTRL_AUXSRC_Enum aux)
{
    assert(f_out <= f_in);
    uint32_t div = (((uint64_t)f_in) << CLOCKS_CLK_REF_DIV_INT_Pos) / f_out;
    reg_atomic_assign(&CLOCKS->CLK_GPOUT1_CTRL,
                      aux << CLOCKS_CLK_GPOUT1_CTRL_AUXSRC_Pos,
                      CLOCKS_CLK_GPOUT1_CTRL_AUXSRC_Msk);
    reg_atomic_assign(&CLOCKS->CLK_GPOUT1_DIV,
                      div,
                      CLOCKS_CLK_GPOUT1_DIV_INT_Msk | CLOCKS_CLK_GPOUT1_DIV_FRAC_Msk);
    reg_atomic_set(&CLOCKS->CLK_GPOUT1_CTRL,
                   1U << CLOCKS_CLK_GPOUT1_CTRL_ENABLE_Pos);
    reg_atomic_assign(&PADS_BANK0->GPIO23,
                      1U << PADS_BANK0_GPIO23_IE_Pos,
                      PADS_BANK0_GPIO23_IE_Msk | PADS_BANK0_GPIO23_OD_Msk);
    reg_atomic_assign(&IO_BANK0->GPIO23_CTRL,
                      IO_BANK0_GPIO23_CTRL_FUNCSEL_clocks_gpout_1 << IO_BANK0_GPIO23_CTRL_FUNCSEL_Pos,
                      IO_BANK0_GPIO23_CTRL_FUNCSEL_Msk);
}
/**
 * @brief   Configure gpio24 as clock output pin
 *
 * @details Can be used as an external clock source for another circuit or
 *          to check the expected signal with a logic analyzer
 *
 * @param   f_in        Input frequency
 * @param   f_out       Output frequency
 * @param   aux         Auxiliary clock source
 */
void clock_gpout2_configure(uint32_t f_in, uint32_t f_out,
                            CLOCKS_CLK_GPOUT2_CTRL_AUXSRC_Enum aux)
{
    assert(f_out <= f_in);
    uint32_t div = (((uint64_t)f_in) << CLOCKS_CLK_REF_DIV_INT_Pos) / f_out;
    reg_atomic_assign(&CLOCKS->CLK_GPOUT2_CTRL,
                      aux << CLOCKS_CLK_GPOUT2_CTRL_AUXSRC_Pos,
                      CLOCKS_CLK_GPOUT2_CTRL_AUXSRC_Msk);
    reg_atomic_assign(&CLOCKS->CLK_GPOUT2_DIV,
                      div,
                      CLOCKS_CLK_GPOUT2_DIV_INT_Msk | CLOCKS_CLK_GPOUT2_DIV_FRAC_Msk);
    reg_atomic_set(&CLOCKS->CLK_GPOUT2_CTRL,
                   1U << CLOCKS_CLK_GPOUT2_CTRL_ENABLE_Pos);
    reg_atomic_assign(&PADS_BANK0->GPIO24,
                      1U << PADS_BANK0_GPIO24_IE_Pos,
                      PADS_BANK0_GPIO24_IE_Msk | PADS_BANK0_GPIO24_OD_Msk);
    reg_atomic_assign(&IO_BANK0->GPIO24_CTRL,
                      IO_BANK0_GPIO24_CTRL_FUNCSEL_clocks_gpout_2 << IO_BANK0_GPIO24_CTRL_FUNCSEL_Pos,
                      IO_BANK0_GPIO24_CTRL_FUNCSEL_Msk);
}
/**
 * @brief   Configure gpio25 as clock output pin
 *
 * @details Can be used as an external clock source for another circuit or
 *          to check the expected signal with a logic analyzer
 *
 * @param   f_in        Input frequency
 * @param   f_out       Output frequency
 * @param   aux         Auxiliary clock source
 */
void clock_gpout3_configure(uint32_t f_in, uint32_t f_out,
                            CLOCKS_CLK_GPOUT3_CTRL_AUXSRC_Enum aux)
{
    assert(f_out <= f_in);
    uint32_t div = (((uint64_t)f_in) << CLOCKS_CLK_REF_DIV_INT_Pos) / f_out;
    reg_atomic_assign(&CLOCKS->CLK_GPOUT3_CTRL,
                      aux << CLOCKS_CLK_GPOUT3_CTRL_AUXSRC_Pos,
                      CLOCKS_CLK_GPOUT3_CTRL_AUXSRC_Msk);
    reg_atomic_assign(&CLOCKS->CLK_GPOUT3_DIV,
                      div,
                      CLOCKS_CLK_GPOUT3_DIV_INT_Msk | CLOCKS_CLK_GPOUT3_DIV_FRAC_Msk);
    reg_atomic_set(&CLOCKS->CLK_GPOUT3_CTRL,
                   1U << CLOCKS_CLK_GPOUT2_CTRL_ENABLE_Pos);
    reg_atomic_assign(&PADS_BANK0->GPIO25,
                      1U << PADS_BANK0_GPIO25_IE_Pos,
                      PADS_BANK0_GPIO25_IE_Msk | PADS_BANK0_GPIO25_OD_Msk);
    reg_atomic_assign(&IO_BANK0->GPIO25_CTRL,
                      IO_BANK0_GPIO25_CTRL_FUNCSEL_clocks_gpout_3 << IO_BANK0_GPIO25_CTRL_FUNCSEL_Pos,
                      IO_BANK0_GPIO25_CTRL_FUNCSEL_Msk);
}
