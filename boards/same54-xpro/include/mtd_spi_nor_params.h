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
        .pages_per_sector = 16,                         \
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
    .wait_chip_erase = 240 * US_PER_SEC,                \
    .wait_64k_erase = 700 * US_PER_MS,                  \
    .wait_sector_erase = 250 * US_PER_MS,               \
    .wait_chip_wake_up = 1 * US_PER_MS,                 \
    .clk  = MHZ(54),                                    \
    .flag = SPI_NOR_F_SECT_4K | SPI_NOR_F_SECT_64K,     \
    .spi  = SPI_DEV(2),                                 \
    .mode = SPI_MODE_0,                                 \
    .cs   = SAM0_QSPI_PIN_CS,                           \
    .wp   = SAM0_QSPI_PIN_DATA_2,                       \
    .hold = SAM0_QSPI_PIN_DATA_3,                       \
    .addr_width = 4,                                    \
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
