/*
 * Copyright (C) 2026 ML!PA Consulting Gmbh
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup    sys_configuration
 *
 * @{
 *
 * @file
 * @brief   Interface for EEPROM registry as a configuration backend
 *
 * @author  Fabian Hüßler <fabian.huessler@ml-pa.com>
 */

#ifndef CONFIGURATION_BACKEND_EEPREG_H
#define CONFIGURATION_BACKEND_EEPREG_H

#include "board.h"
#include "mtd.h"
#include "configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(CONFIGURATION_EEPREG_MTD) || defined(DOXYGEN)
/**
 * @brief   MTD device to use for EEPROM registry configuration backend
 */
#define CONFIGURATION_EEPREG_MTD        MTD_0
#endif

/**
 * @brief   Initialize the EEPROM registry backend
 *
 * @return  0 on success
 */
int configuration_backend_eepreg_init(void);

/**
 * @brief   EEPROM registry backend operations
 */
extern const conf_backend_ops_t conf_backend_eepreg_ops;

#ifdef __cplusplus
}
#endif

#endif
/** @} */
