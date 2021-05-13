/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_common_nrf52
 * @{
 *
 * @file
 * @brief       Board initialization for the nRF52xxx DK
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */
#include "kernel_defines.h"
#include "cpu.h"
#include "board.h"

#ifdef MTD_0 /* nrf52840dk */
#if IS_USED(MODULE_MTD_SPI_NOR)
#include "mtd_spi_nor.h"
#if !IS_USED(MODULE_AUTO_INIT_STORAGE_SPI_NOR)
#include "mtd_spi_nor_params.h"
static mtd_spi_nor_t _mtd_spi_nor_devs[] = {
    MTD_SPI_NOR_DEVS
};
mtd_dev_t *mtd0 = (mtd_dev_t *)&_mtd_spi_nor_devs[0];
#else
mtd_dev_t *mtd0;
#endif /* !IS_USED(MODULE_AUTO_INIT_STORAGE_SPI_NOR) */
#endif /* IS_USED(MODULE_MTD_SPI_NOR) */
#endif /* MTD_0 */

void board_init(void)
{
    /* initialize the boards LEDs */
    LED_PORT->DIRSET = (LED_MASK);
    LED_PORT->OUTSET = (LED_MASK);

    /* initialize the CPU */
    cpu_init();

#ifdef MTD_0
#if IS_USED(MODULE_MTD_SPI_NOR)
#if !IS_USED(MODULE_AUTO_INIT_STORAGE_SPI_NOR)
    _mtd_spi_nor_devs[0].params = &mtd_spi_nor_params[0];
#else
    mtd0 = (mtd_dev_t *)mtd_spi_nor_devs();
#endif
#endif
#endif
}
