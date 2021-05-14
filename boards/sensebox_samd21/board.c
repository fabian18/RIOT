/*
 * Copyright (C) 2018 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_sensebox_samd21
 * @{
 *
 * @file
 * @brief       Board specific implementations for the SenseBox MCU with SAMD21
 * board.
 *
 * @author      Leandro Lanzieri <leandro.lanzieri@haw-hamburg.de>
 * @}
 */
#include "kernel_defines.h"
#include "cpu.h"
#include "board.h"
#include "periph/gpio.h"

#if IS_USED(MODULE_MTD_SDCARD)
#include "mtd_sdcard.h"
#if !IS_USED(MODULE_AUTO_INIT_STORAGE_SDCARD_SPI)
#include "sdcard_spi_params.h"
#include "mtd_sdcard_params.h"
static sdcard_spi_t sdcard_spi_devs[MTD_SDCARD_NUMOF];
static mtd_sdcard_t mtd_sdcard_devs[MTD_SDCARD_NUMOF] = {
    MTD_SDCARD_DEVS
};
mtd_dev_t *mtd0 = (mtd_dev_t *)&mtd_sdcard_devs[0];
#else
mtd_dev_t *mtd0;
#endif /* !IS_USED(MODULE_AUTO_INIT_STORAGE_SDCARD_SPI) */
#endif /* MODULE_MTD_SDCARD */

void board_init(void)
{
    /* initialize the CPU */
    cpu_init();

    /* initialize the on-board red LED on pin PA27 */
    gpio_init(LED0_PIN, GPIO_OUT);

    /* initialize the on-board green LED on pin PA28 */
    gpio_init(LED1_PIN, GPIO_OUT);

    /* initialize the on-board button on pin PA20 */
    gpio_init(BTN0_PIN, BTN0_MODE);

    /* initialize XBEE1 bus enable pin */
    gpio_init(XBEE1_EN_PIN, XBEE1_EN_MODE);

    /* enable XBEE1 bus */
    XBEE1_ENABLE;

    /* initialize XBEE2 bus enable pin */
    gpio_init(XBEE2_EN_PIN, XBEE2_EN_MODE);

    /* enable XBEE2 bus */
    XBEE2_ENABLE;

    /* initialize I2C bus enable pin */
    gpio_init(I2C_EN_PIN, I2C_EN_MODE);

    /* enable I2C bus */
    I2C_ENABLE;

    /* delay for SPI reset. Read README.md for more information. */
    for (unsigned i = 0; i < WAIT_FOR_SPI_RESET; i++) {
        __asm__("nop");
    }

#if IS_USED(MODULE_MTD_SDCARD)
#if !IS_USED(MODULE_AUTO_INIT_STORAGE_SDCARD_SPI)
    mtd_sdcard_devs[0].sd_card = &sdcard_spi_devs[0];
    mtd_sdcard_devs[0].params = &sdcard_spi_params[0];
#else
    mtd0 = (mtd_dev_t *)mtd_sdcard_devs();
#endif
#endif
}
