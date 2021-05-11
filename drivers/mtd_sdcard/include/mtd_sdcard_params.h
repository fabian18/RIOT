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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MTD_SDCARD_DEVS
/**
 * @brief   Array body of sdcard MTD
 */
#define MTD_SDCARD_DEVS
#endif

#ifndef MTD_SDCARD_NUMOF
/**
 * @brief   Number of sdcard MTD
 */
#define MTD_SDCARD_NUMOF        0
#endif

#ifdef __cplusplus
}
#endif

#endif /* MTD_SDCARD_PARAMS_H */
/** @} */
