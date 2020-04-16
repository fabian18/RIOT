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
 * @brief       Interface for power monitoring with miot-esp32 and two ina3221
 *
 * @author      Fabian Hüßler <afabian.huessler@ovgu.de>
 */

#ifndef POWERMON_H
#define POWERMON_H

#include <stdint.h>

#include "ina3221.h"

#ifdef __cplusplus
extern "C" {
#endif

struct powermon;
typedef struct powermon powermon_t;

struct powermon_info {
    union {
        struct {
            const char* ch1_name;
            const char* ch2_name;
            const char* ch3_name;
        };
        const char* ch_names[3];
    };
};
typedef struct powermon_info powermon_info_t;

struct powermon_measurement_result {
    int32_t shunt_uv[INA3221_NUM_CH];
    int16_t bus_mv[INA3221_NUM_CH];
};
typedef struct powermon_measurement_result powermon_measurement_result_t;

struct powermon_result {
    int32_t current_ua[INA3221_NUM_CH];
    int32_t power_uw[INA3221_NUM_CH];
};
typedef struct powermon_result powermon_result_t;

struct powermon_config {
    ina3221_enable_ch_t channels;
    ina3221_num_samples_t samples;
    ina3221_conv_time_bus_adc_t bus_adc;
    ina3221_conv_time_shunt_adc_t shunt_adc;
    ina3221_mode_t mode;
};
typedef struct powermon_config powermon_config_t;

typedef
void (*on_measurement_cb)(powermon_t* mon,
                          const powermon_measurement_result_t* mres,
                          const powermon_result_t* res,
                          void* user_data);

typedef
void (*on_config_cb)(powermon_t* mon,
                     powermon_config_t* cfg_new,
                     powermon_config_t* cfg_old,
                     void* user_data);

typedef
void (*on_error_cb)(powermon_t* mon, char* msg, void* user_data);

int powermon_init(void);
uint8_t powermon_get_num_mons(void);
powermon_t* powermon_get_mon(uint8_t i);
int powermon_configure(powermon_t* mon,
                       const ina3221_enable_ch_t* channels,
                       const ina3221_num_samples_t* samples,
                       const ina3221_conv_time_bus_adc_t* bus_adc,
                       const ina3221_conv_time_shunt_adc_t* shunt_adc,
                       const ina3221_mode_t* mode);
void powermon_set_on_measurement(powermon_t* mon, on_measurement_cb cb);
void powermon_set_on_config(powermon_t* mon, on_config_cb cb);
void powermon_set_on_error(powermon_t* mon, on_error_cb cb);
void powermon_set_user_data(powermon_t* mon, void* data);
const ina3221_t* powermon_get_dev(const powermon_t* mon);
ina3221_channel_t powermon_get_channels(const powermon_t* mon);
const powermon_info_t* powermon_get_info(const powermon_t* mon);

#ifdef __cplusplus
}
#endif

#endif /* POWERMON_H */
/** @} */
