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
 *
 * @file
 * @brief       Tables to lookup numerical sample properties of ina3221
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include <stdint.h>

/* difference between corresponding enums is 0x0200 */
const uint16_t ina3221_num_samples[8] = {
    1,
    4,
    16,
    64,
    128,
    256,
    512,
    1024
};

/* difference between corresponding enums is 0x0040 */
const uint16_t ina3221_conv_time_badc[8] = {
    140,
    204,
    332,
    588,
    1100,
    2116,
    4156,
    8244
};

/* difference between corresponding enums is 0x0008 */
const uint16_t ina3221_conv_time_sadc[8] = {
    140,
    204,
    332,
    588,
    1100,
    2116,
    4156,
    8244
};
