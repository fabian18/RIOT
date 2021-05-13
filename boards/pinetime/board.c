/*
 * Copyright (C) 2017 Freie Universität Berlin
 *               2020 Inria
 *               2020 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_pinetime
 * @{
 *
 * @file
 * @brief       Board initialization for the PineTime
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */
#include "kernel_defines.h"
#include "cpu.h"
#include "board.h"
#include "periph/gpio.h"
#include "periph/spi.h"

#if IS_USED(MODULE_MTD)
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
#endif /* MODULE_MTD */

void board_init(void)
{
    /* initialize the CPU */
    cpu_init();

    /* initialize pins */
    gpio_init(VCC33, GPIO_OUT);
    gpio_init(BUTTON0_ENABLE, GPIO_OUT);
    gpio_init(BUTTON0, GPIO_IN);
    gpio_init(VIBRATOR, GPIO_OUT);
    gpio_init(LCD_BACKLIGHT_LOW, GPIO_OUT);
    gpio_init(LCD_BACKLIGHT_MID, GPIO_OUT);
    gpio_init(LCD_BACKLIGHT_HIGH, GPIO_OUT);

    gpio_set(VCC33);
    gpio_set(BUTTON0_ENABLE);
    gpio_set(VIBRATOR);
    gpio_set(LCD_BACKLIGHT_LOW);
    gpio_set(LCD_BACKLIGHT_MID);
    gpio_set(LCD_BACKLIGHT_HIGH);

#if IS_USED(MODULE_MTD)
#if !IS_USED(MODULE_AUTO_INIT_STORAGE_SPI_NOR)
    _mtd_spi_nor_devs[0].params = &mtd_spi_nor_params[0];
#else
    mtd0 = (mtd_dev_t *)mtd_spi_nor_devs();
#endif
#endif
}
