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

#ifndef MTD_SPI_NOR_DEVS
/**
 * @brief   Array body of SPI NOR MTD
 */
#define MTD_SPI_NOR_DEVS
#endif

#ifndef MTD_SPI_NOR_PARAMS
/**
 * @brief   Array body of SPI NOR flash storage configuration
 */
#define MTD_SPI_NOR_PARAMS
#endif

#ifndef MTD_SPI_NOR_NUMOF
/**
 * @brief   Number of MTD SPI NOR flash chips
 */
#define MTD_SPI_NOR_NUMOF     0
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
