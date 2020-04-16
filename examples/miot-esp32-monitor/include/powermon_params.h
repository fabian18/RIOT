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
 * @brief       Power monitoring wrapper around ina3221 parameters
 *
 * @author      Fabian Hüßler <afabian.huessler@ovgu.de>
 */

#ifndef POWERMON_PARAMS_H
#define POWERMON_PARAMS_H

#include "powermon.h"
#include "ina3221_params.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INA3221_NUM                     ARRAY_SIZE(ina3221_params)

#ifndef POWERMON_0_CH1_NAME
#define POWERMON_0_CH1_NAME             "0_ch1"
#endif

#ifndef POWERMON_0_CH2_NAME
#define POWERMON_0_CH2_NAME             "0_ch2"
#endif

#ifndef POWERMON_0_CH3_NAME
#define POWERMON_0_CH3_NAME             "CC1101"
#endif

#ifndef POWERMON_1_CH1_NAME
#define POWERMON_1_CH1_NAME             "1_ch1"
#endif

#ifndef POWERMON_1_CH2_NAME
#define POWERMON_1_CH2_NAME             "NRF24L01+"
#endif

#ifndef POWERMON_1_CH3_NAME
#define POWERMON_1_CH3_NAME             "AT86RF233"
#endif

static const powermon_info_t _handle_infos[INA3221_NUM] = {
#ifdef POWERMON_0
    {.ch_names = {
        POWERMON_0_CH1_NAME,
        POWERMON_0_CH2_NAME,
        POWERMON_0_CH3_NAME }
    },
#endif
#ifdef POWERMON_1
    {.ch_names = {
        POWERMON_1_CH1_NAME,
        POWERMON_1_CH2_NAME,
        POWERMON_1_CH3_NAME }
    },
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* POWERMON_PARAMS_H */
/** @} */
