/*
 * Copyright (C) 2021 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_mtd_spi_nor
 * @{
 *
 * @file
 * @brief       Default configuration for MTD SPI NOR flash storage
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef MTD_SPI_NOR_PARAMS_H
#define MTD_SPI_NOR_PARAMS_H

#include "board.h"
#include "mtd_spi_nor.h"

#ifndef MTD_SPI_NOR_DEVS
/**
 * @brief   Array body of SPI NOR MTD
 */
#define MTD_SPI_NOR_DEVS                                        \
{                                                               \
    .base = {                                                   \
        .driver = &mtd_spi_nor_driver,                          \
        .page_size = IKEA_TRADFRI_NOR_PAGE_SIZE,                \
        .pages_per_sector = IKEA_TRADFRI_NOR_PAGES_PER_SECTOR,  \
        .sector_count = IKEA_TRADFRI_NOR_SECTOR_COUNT,          \
    }                                                           \
},
#endif

#ifndef MTD_SPI_NOR_PARAMS
/**
 * @brief   Array body of SPI NOR flash storage configuration
 */
#define MTD_SPI_NOR_PARAMS                                      \
{                                                               \
    .opcode = &mtd_spi_nor_opcode_default,                      \
    .wait_chip_erase = 2LU * US_PER_SEC,                        \
    .wait_32k_erase = 500LU *US_PER_MS,                         \
    .wait_sector_erase = 300LU * US_PER_MS,                     \
    .wait_chip_wake_up = 1LU * US_PER_MS,                       \
    .clk = IKEA_TRADFRI_NOR_SPI_CLK,                            \
    .flag = IKEA_TRADFRI_NOR_FLAGS,                             \
    .spi = IKEA_TRADFRI_NOR_SPI_DEV,                            \
    .mode = IKEA_TRADFRI_NOR_SPI_MODE,                          \
    .cs = IKEA_TRADFRI_NOR_SPI_CS,                              \
    .wp = GPIO_UNDEF,                                           \
    .hold = GPIO_UNDEF,                                         \
    .addr_width = 3,                                            \
},
#endif

#ifndef MTD_SPI_NOR_NUMOF
/**
 * @brief   Number of MTD SPI NOR flash chips
 */
#define MTD_SPI_NOR_NUMOF     (1)
#endif

#if (MTD_SPI_NOR_NUMOF > 0) || defined(DOXYGEN)
/**
 * @brief   Array of MTD SPI NOR flash configuration parameters
 */
static const mtd_spi_nor_params_t mtd_spi_nor_params[MTD_SPI_NOR_NUMOF] = {
    MTD_SPI_NOR_PARAMS
};
#endif

#endif /* MTD_SPI_NOR_PARAMS_H */
/** @} */
