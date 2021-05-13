/*
 * Copyright (C) 2019 Benjamin Valentin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_common_weact-f4x1cx
 * @{
 *
 * @file
 * @brief       Board initialization code for the WeAct-F4x1Cx board.
 *
 * @author      Benjamin Valentin <benpicco@googlemail.com>
 *
 * @}
 */
#include "kernel_defines.h"
#include "board.h"
#include "cpu.h"
#include "periph/gpio.h"

#if IS_USED(MODULE_MTD)
#include "mtd_spi_nor.h"
#if !IS_USED(MODULE_AUTO_INIT_STORAGE_SPI_NOR)
#include "mtd_spi_nor_params.h"
/* AT25SF041 */
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
    cpu_init();

    gpio_init(LED0_PIN, GPIO_OUT);
    LED0_OFF;

#if IS_USED(MODULE_MTD)
#if !IS_USED(MODULE_AUTO_INIT_STORAGE_SPI_NOR)
    _mtd_spi_nor_devs[0].params = &mtd_spi_nor_params[0];
#else
    mtd0 = (mtd_dev_t *)mtd_spi_nor_devs();
#endif
#endif
}
