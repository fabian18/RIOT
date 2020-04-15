/*
 * Copyright (C) 2020 Otto-von-Guericke Universität
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @defgroup    drivers_mtd_eeprom
 * @ingroup     drivers_storage
 * @brief       Driver interface for MTD EEPROM storage
 *
 * @{
 *
 * @file
 * @brief       MTD abstraction drivers for EEPROM storage
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef MTD_EEPROM_H
#define MTD_EEPROM_H

#include "mtd.h"
#include "at24cxxx.h"
#include "at25xxx.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief MTD EEPROM device
 *
 * Driver must be initialized before usage
 */
typedef struct {
    mtd_dev_t base; /**< MTD base device */
} mtd_eeprom_t;

/**
 * @brief Shortcut initializer for @ref mtd_eeprom_t
 */
#define MTD_EEPROM_PERIPH_INIT              \
(mtd_eeprom_t) {                            \
    .base = {.driver = &mtd_periph_driver}, \
}

/**
 * @brief MTD at24cxxx EEPROM device
 *
 * Driver and device pointer must be initialized before usage
 */
typedef struct {
    mtd_eeprom_t mtd_eeprom; /**< MTD EEPROM */
    at24cxxx_t *dev_ptr;     /**< at24cxxx device pointer */
} mtd_eeprom_at24cxxx_t;

/**
 * @brief Shortcut initializer for @ref mtd_eeprom_at24cxxx_t
 */
#define MTD_EEPROM_AT24CXXX_INIT(dev_p)     \
(mtd_eeprom_at24cxxx_t) {                   \
    .mtd_eeprom = {                         \
        .base = {                           \
            .driver = &mtd_at24cxxx_driver  \
        }                                   \
    },                                      \
    .dev_ptr = (dev_p)                      \
}

/**
 * @brief MTD at25xxx EEPROM device
 *
 * Driver and device pointer must be initialized before usage
 */
typedef struct {
    mtd_eeprom_t mtd_eeprom; /**< MTD EEPROM */
    at25xxx_t *dev_ptr;      /**< at25xxx device pointer */
} mtd_eeprom_at25xxx_t;

/**
 * @brief Shortcut initializer for @ref mtd_eeprom_at25xxx_t
 */
#define MTD_EEPROM_AT25XXX_INIT(dev_p)      \
(mtd_eeprom_at25xxx_t) {                    \
    .mtd_eeprom = {                         \
        .base = {                           \
            .driver = &mtd_at25xxx_driver   \
        }                                   \
    },                                      \
    .dev_ptr = (dev_p)                      \
}

/**
 * @brief MTD EEPROM driver for MCU integrated EEPROM
 */
extern const mtd_desc_t mtd_periph_driver;

/**
 * @brief MTD EEPROM driver for at24cxxx EEPROM
 */
extern const mtd_desc_t mtd_at24cxxx_driver;

/**
 * @brief MTD EEPROM driver for at25xxx EEPROM
 */
extern const mtd_desc_t mtd_at25xxx_driver;

#ifdef __cplusplus
}
#endif

#endif /* MTD_EEPROM_H */
/** @} */
