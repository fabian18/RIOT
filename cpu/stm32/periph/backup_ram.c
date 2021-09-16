/*
 * Copyright (C) 2021 Otto-von-Guericke Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_stm32
 * @{
 *
 * @file
 * @brief       Implementation of backup SRAM capabilities for stm32 MCUs
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 * @}
 */

#include "periph_cpu.h"
#include "stmclk.h"

#ifndef BKPRAM_CONFIG
/**
 * @brief   Configuration to retain backup RAM content during standby mode
 */
#if defined(CPU_FAM_STM32F7)
#define BKPRAM_CONFIG       (PWR_CSR1_BRE | PWR_CSR1_EIWUP)
#else
#define BKPRAM_CONFIG       (PWR_CSR_BRE)
#endif
#endif

#ifndef BKPRAM_READY
/**
 * @brief   Bits to check if the backup regulator is ready
 *          and backup RAM is retained
 */
#if defined(CPU_FAM_STM32F7)
#define BKPRAM_READY       (PWR_CSR1_BRR)
#else
#define BKPRAM_READY       (PWR_CSR_BRR)
#endif
#endif

#if defined(CPU_FAM_STM32F7)
#define PWR_BRE_REG    PWR->CSR1
#else
#define PWR_BRE_REG    PWR->CSR
#endif

void backup_ram_init(void)
{
    /* see reference manual 4.1.5 "Battery backup domain" */
    periph_clk_en(APB1, RCC_APB1ENR_PWREN);
    stmclk_dbp_unlock();
    periph_clk_en(AHB1, RCC_AHB1ENR_BKPSRAMEN);
}

bool backup_ram_is_retained(void)
{
    stmclk_dbp_unlock();
    return (PWR_BRE_REG & BKPRAM_CONFIG) == BKPRAM_CONFIG;
}

void backup_ram_sleep(void)
{
    stmclk_dbp_unlock();
    /* switch on regulator to retain backup SRAM content while in standby mode */
    if (!backup_ram_is_retained()) {
        PWR_BRE_REG |= BKPRAM_CONFIG;
        while (!(PWR_BRE_REG & BKPRAM_READY));
    }
    stmclk_dbp_lock();
}

void backup_ram_awake(void)
{
    stmclk_dbp_unlock();
    /* switch off regulator to save power */
    PWR_BRE_REG &= ~BKPRAM_CONFIG;
}
