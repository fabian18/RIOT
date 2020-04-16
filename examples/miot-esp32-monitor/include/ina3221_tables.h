/*
 * Copyright (C) 2020 OvGU Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @ingroup     examples
 * @{
 * @file
 * @brief       Tables to lookup numerical sample properties of ina3221
 *
 * @author      Fabian Hüßler <afabian.huessler@ovgu.de>
 */

#ifndef INA3221_TABLES_H
#define INA3221_TABLES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INA3221_NUM_SAMPLES_DIFF        (0x0200)
#define INA3221_CONV_TIME_BADC_DIFF     (0x0040)
#define INA3221_CONV_TIME_SADC_DIFF     (0x0008)

extern const uint16_t ina3221_num_samples[8];
extern const uint16_t ina3221_conv_time_badc[8];
extern const uint16_t ina3221_conv_time_sadc[8];

#ifdef __cplusplus
}
#endif

#endif /* INA3221_TABLES_H */
/** @} */
