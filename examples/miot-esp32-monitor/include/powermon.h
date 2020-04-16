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

#include "sched.h"
#include "ina3221.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Powermon forward declaration to hide implementation details
 */
struct powermon;

/**
 * @brief   Powermon type
 */
typedef struct powermon powermon_t;

/**
 * @brief   Numeric powermon ID
 */
typedef int8_t powermon_id_t;

/**
 * @brief   Powermon meta information
 */
typedef struct powermon_info {
    const char* ch_names[3];                    /**< Channel labels */
} powermon_info_t;

/**
 * @brief   Type which represent powermon measurement metrics
 */
typedef struct powermon_measurement_result {
    int32_t shunt_uv[INA3221_NUM_CH];           /**< Shunt voltage drops */
    int16_t bus_mv[INA3221_NUM_CH];             /**< Bus voltages */
} powermon_measurement_result_t;

/**
 * @brief   Type which represents powermon calculated metrics
 */
typedef struct powermon_result {
    int32_t current_ua[INA3221_NUM_CH];         /**< Current values */
    int32_t power_uw[INA3221_NUM_CH];           /**< Power values */
} powermon_result_t;

/**
 * @brief   Type to represent powermon configuration
 */
typedef struct powermon_config {
    uint16_t channels;                          /**< Channels to be used */
    ina3221_num_samples_t samples;              /**< Number of samples to compute average */
    ina3221_conv_time_bus_adc_t bus_adc;        /**< Bus ADC conversion time */
    ina3221_conv_time_shunt_adc_t shunt_adc;    /**< Shunt ADC conversion time */
    ina3221_mode_t mode;                        /**< Operation mode */
} powermon_config_t;

/**
 * @brief   Callback on new measurements
 */
typedef void (*powermon_on_measurement_cb)(const powermon_t* mon,
                                           const powermon_measurement_result_t* mres,
                                           const powermon_result_t* res,
                                           void* user_data);

/**
 * @brief   Callback on configuration
 */
typedef void (*powermon_on_config_cb)(const powermon_t* mon,
                                      powermon_config_t* cfg_new,
                                      powermon_config_t* cfg_old,
                                      void* user_data);

/**
 * @brief   Callback on error
 */
typedef void (*powermon_on_error_cb)(const powermon_t* mon,
                                     char* msg,
                                     void* user_data);

/**
 * @brief   Initialize power monitoring
 */
int powermon_init(void);

/**
 * @brief   Create power monitoring thread
 *
 * @param[in]       stack       Thread stack
 * @param[in]       stacksize   Size of @p stack
 * @param[in]       priority    Thread priority
 *
 * @return  PID
 */
kernel_pid_t powermon_start(char *stack, size_t stacksize, uint8_t priority);

/**
 * @brief   Get number of successfully initialized monitors
 */
powermon_id_t powermon_get_num_mons(void);

/**
 * @brief   Get monitor handle
 *
 * @param[in]       id          Monitor ID
 *
 * @return  Monitor handle
 */
powermon_t *powermon_get_mon(powermon_id_t id);

/**
 * @brief   Configure power monitor device
 *
 * @param[in]       mon         Monitor handle
 * @param[in]       channels    Channels
 * @param[in]       samples     Number of samples to compute average
 * @param[in]       bus_adc     Bus ADC conversion time
 * @param[in]       shunt_adc   Shunt ADC conversion time
 * @param[in]       mode        Operation mode
 *
 * @return 0 on success
 */
int powermon_configure(powermon_t* mon,
                       const uint16_t* channels,
                       const ina3221_num_samples_t* samples,
                       const ina3221_conv_time_bus_adc_t* bus_adc,
                       const ina3221_conv_time_shunt_adc_t* shunt_adc,
                       const ina3221_mode_t* mode);

/**
 * @brief   Set measurement callback
 *
 * @param[in]       mon         Monitor handle
 * @param[in]       cb          Monitor callback
 */
void powermon_set_on_measurement(powermon_t* mon, powermon_on_measurement_cb cb);

/**
 * @brief   Set configuration callback
 *
 * @param[in]       mon         Monitor handle
 * @param[in]       cb          Monitor callback
 */
void powermon_set_on_config(powermon_t* mon, powermon_on_config_cb cb);

/**
 * @brief   Set error callback
 *
 * @param[in]       mon         Monitor handle
 * @param[in]       cb          Monitor callback
 */
void powermon_set_on_error(powermon_t* mon, powermon_on_error_cb cb);

/**
 * @brief   Set arbitrary user data
 *
 * @param[in]       mon         Monitor device
 * @param[in]       data        User data
 */
void powermon_set_user_data(powermon_t* mon, void* data);

/**
 * @brief   Get under-the-hood ina3221 device handle
 *
 * @param[in]       mon         Monitor device
 *
 * @return ina3221 device
 */
const ina3221_t* powermon_get_dev(const powermon_t* mon);

/**
 * @brief   Get enabled channels
 *
 * @param[in]       mon         Monitor device
 *
 * @return  Enabled channels
 */
ina3221_channel_t powermon_get_channels(const powermon_t* mon);

/**
 * @brief   Get monitor meta information
 *
 * @param[in]       mon         Monitor device
 *
 * @return  Device information
 */
const powermon_info_t* powermon_get_info(const powermon_t* mon);

#ifdef __cplusplus
}
#endif

#endif /* POWERMON_H */
/** @} */
