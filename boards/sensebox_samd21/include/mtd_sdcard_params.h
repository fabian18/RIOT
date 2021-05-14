/*
 * Copyright (C) 2021 Otto-von-Guericke Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_mtd_sdcard
 * @{
 *
 * @file
 * @brief       Default parameters for sdcard_spi MTD driver
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */

#ifndef MTD_SDCARD_PARAMS_H
#define MTD_SDCARD_PARAMS_H

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MTD_SDCARD_DEVS
/**
 * @brief   Array body of sdcard MTD
 */
#define MTD_SDCARD_DEVS                                     \
{                                                           \
    .base = {                                               \
        .driver = &mtd_sdcard_driver,                       \
        .page_size = MTD_SD_CARD_PAGE_SIZE,                 \
        .pages_per_sector = MTD_SD_CARD_PAGES_PER_SECTOR,   \
        .sector_count = MTD_SD_CARD_SECTOR_COUNT            \
    },                                                      \
},
#endif

#ifndef MTD_SDCARD_NUMOF
/**
 * @brief   Number of sdcard MTD
 */
#define MTD_SDCARD_NUMOF        1
#endif

#ifdef __cplusplus
}
#endif

#endif /* MTD_SDCARD_PARAMS_H */
/** @} */
