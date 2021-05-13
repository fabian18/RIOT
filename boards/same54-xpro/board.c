/*
 * Copyright (C) 2019 ML!PA Consulting GmbH
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_same54-xpro
 * @{
 *
 * @file
 * @brief       Board specific implementations for the Microchip SAM E54 Xplained
 *              Pro board
 *
 * @author      Benjamin Valentin <benjamin.valentin@ml-pa.com>
 * @}
 */
#include "kernel_defines.h"
#include "board.h"
#include "periph/gpio.h"

#if IS_USED(MODULE_MTD)
#include "mtd_spi_nor.h"
#if !IS_USED(MODULE_AUTO_INIT_STORAGE_SPI_NOR)
#include "mtd_spi_nor_params.h"
/* N25Q256A */
static mtd_spi_nor_t _mtd_spi_nor_devs[] = {
    MTD_SPI_NOR_DEVS
};
mtd_dev_t *mtd0 = (mtd_dev_t *)&_mtd_spi_nor_devs;
#else
mtd_dev_t *mtd0;
#endif

#include "mtd_at24cxxx.h"
#include "at24cxxx_params.h"
static at24cxxx_t at24cxxx_dev;
static mtd_at24cxxx_t at24mac_dev = {
    .base = {
        .driver = &mtd_at24cxxx_driver,
    },
    .at24cxxx_eeprom = &at24cxxx_dev,
    .params = at24cxxx_params,
};
mtd_dev_t *mtd1 = (mtd_dev_t *)&at24mac_dev;
#endif /* MODULE_MTD */

void board_init(void)
{
    /* initialize the on-board LED */
    gpio_init(LED0_PIN, GPIO_OUT);
    LED0_OFF;

    /* initialize the on-board button */
    gpio_init(BTN0_PIN, BTN0_MODE);

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
