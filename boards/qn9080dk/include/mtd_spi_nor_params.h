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
#define MTD_SPI_NOR_DEVS                                                    \
{                                                                           \
    .base = {                                                               \
        .driver = &mtd_spi_nor_driver,                                      \
        .page_size = 256,                                                   \
        .pages_per_sector = 16,  /* 4 KiB sectors */                        \
        .sector_count = 64,                                                 \
    }                                                                       \
},
#endif

#ifndef MTD_SPI_NOR_PARAMS
/**
 * @brief   Array body of SPI NOR flash storage configuration
 */
#define MTD_SPI_NOR_PARAMS                                                  \
{                                                                           \
    .opcode = &mtd_spi_nor_opcode_default,                                  \
    .wait_chip_erase   = 15000LU * US_PER_MS,                               \
    .wait_64k_erase    = 3500LU * US_PER_MS,                                \
    .wait_32k_erase    = 1750LU * US_PER_MS,                                \
    .wait_sector_erase = 240LU * US_PER_MS,                                 \
    .wait_chip_wake_up = 1LU * US_PER_MS,                                   \
    .clk  = CLOCK_CORECLOCK,  /* Max fR and fC is 33 MHz, max core is 32 MHz. */ \
    .flag = SPI_NOR_F_SECT_4K | SPI_NOR_F_SECT_32K | SPI_NOR_F_SECT_64K,    \
    .spi  = SPI_DEV(0),                                                     \
    .mode = SPI_MODE_0,                                                     \
    .cs   = SPI_HWCS(0),  /* GPIO(PORT_A, 3) is used for HWCS(0) on FC2 */  \
    .wp   = GPIO_UNDEF,                                                     \
    .hold = GPIO_UNDEF,                                                     \
    .addr_width = 3,  /* 24-bit addresses */                                \
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
