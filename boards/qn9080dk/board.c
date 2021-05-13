/*
 * Copyright (C) 2020 iosabi
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_qn9080dk
 * @{
 *
 * @file
 * @brief       Board specific implementations for the QN9080DK base board
 *
 * @author      iosabi <iosabi@protonmail.com>
 *
 * @}
 */
#include "kernel_defines.h"
#include "cpu.h"
#include "board.h"
#include "periph/gpio.h"

#if IS_USED(MODULE_MTD)
#include "mtd_spi_nor.h"
#if !IS_USED(MODULE_AUTO_INIT_STORAGE_SPI_NOR)
#include "mtd_spi_nor_params.h"
/* MX25R2035F */
static mtd_spi_nor_t _mtd_spi_nor_devs[] = {
    MTD_SPI_NOR_DEVS
};
mtd_dev_t *mtd0 = (mtd_dev_t *)&_mtd_spi_nor_devs[0];
#else
mtd_dev_t *mtd0;
#endif /* !IS_USED(MODULE_AUTO_INIT_STORAGE_SPI_NOR) */
#endif /* MODULE_MTD */

void board_init(void)
{
    /* Initialize LEDs and Buttons. */
    gpio_init(LED_RED_PIN, GPIO_OUT);
    gpio_init(LED_GREEN_PIN, GPIO_OUT);
    gpio_init(LED_BLUE_PIN, GPIO_OUT);
    gpio_init(BTN1_PIN, BTN1_MODE);
    gpio_init(BTN2_PIN, BTN2_MODE);

    /* initialize the CPU */
    cpu_init();

#if IS_USED(MODULE_MTD)
#if !IS_USED(MODULE_AUTO_INIT_STORAGE_SPI_NOR)
    _mtd_spi_nor_devs[0].params = &mtd_spi_nor_params[0];
#else
    mtd0 = (mtd_dev_t *)mtd_spi_nor_devs();
#endif
#endif
}
