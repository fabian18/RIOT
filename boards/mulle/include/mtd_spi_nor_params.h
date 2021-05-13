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
#define MTD_SPI_NOR_DEVS                                \
{                                                       \
    .base = {                                           \
        .driver = &mtd_spi_nor_driver,                  \
        .page_size = 256,                               \
        .pages_per_sector = 256,                        \
        .sector_count = 32,                             \
    }                                                   \
},
#endif

#ifndef MTD_SPI_NOR_PARAMS
/**
 * @brief   Array body of SPI NOR flash storage configuration
 */
#define MTD_SPI_NOR_PARAMS                              \
{                                                       \
    .opcode = &mtd_spi_nor_opcode_default,              \
    .wait_chip_erase = 16LU * US_PER_SEC,               \
    .wait_sector_erase = 10LU * US_PER_MS,              \
    .wait_32k_erase = 20LU * US_PER_MS,                 \
    .wait_chip_wake_up = 1LU * US_PER_MS,               \
    .spi = MULLE_NOR_SPI_DEV,                           \
    .addr_width = 3,                                    \
    .mode = SPI_MODE_3,                                 \
    .cs = MULLE_NOR_SPI_CS,                             \
    .wp = GPIO_UNDEF,                                   \
    .hold = GPIO_UNDEF,                                 \
    .clk = SPI_CLK_10MHZ,                               \
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
