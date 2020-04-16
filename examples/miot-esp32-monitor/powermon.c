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
 * @brief       Power monitoring with miot-esp32 board and two ina3221
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include "kernel_defines.h"
#include "xtimer.h"
#define ENABLE_DEBUG        (0)
#include "debug.h"

#include "include/ina3221_tables.h"
#include "include/powermon_params.h"

#define MAX_POLLS               (8)

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

struct powermon {
    ina3221_t dev;
    ina3221_channel_t channels;
    uint32_t update_interval_us;
    xtimer_t timeout;
    const powermon_info_t* info;
    on_measurement_cb on_measurement;
    on_config_cb on_config;
    on_error_cb on_error;
    void* user_data;
};

static powermon_t _mons[INA3221_NUM];
static uint8_t _num_mons;

static
uint32_t _get_update_interval_us(ina3221_enable_ch_t ech,
                                 ina3221_num_samples_t ns,
                                 ina3221_conv_time_bus_adc_t conv_badc,
                                 ina3221_conv_time_shunt_adc_t conv_sadc,
                                 ina3221_mode_t mode)
{
    int num_en_ch = ((ech & (INA3221_ENABLE_CH1)) ? 1 : 0) +
                    ((ech & (INA3221_ENABLE_CH2)) ? 1 : 0) +
                    ((ech & (INA3221_ENABLE_CH3)) ? 1 : 0);
    uint32_t conv;
    if (mode == INA3221_MODE_POWER_DOWN ||
        mode == INA3221_MODE_POWER_DOWN_) {
            return 0;
    }
    else if (mode == INA3221_MODE_TRIGGER_SHUNT_ONLY ||
             mode == INA3221_MODE_CONTINUOUS_SHUNT_ONLY) {
        conv = (ina3221_conv_time_sadc[conv_sadc / INA3221_CONV_TIME_SADC_DIFF]);
    }
    else if (mode == INA3221_MODE_TRIGGER_BUS_ONLY ||
             mode == INA3221_MODE_CONTINUOUS_BUS_ONLY) {
        conv = (ina3221_conv_time_badc[conv_badc / INA3221_CONV_TIME_BADC_DIFF]);
    }
    else {
        conv = (ina3221_conv_time_sadc[conv_sadc / INA3221_CONV_TIME_SADC_DIFF]) +
               (ina3221_conv_time_badc[conv_badc / INA3221_CONV_TIME_BADC_DIFF]);
    }
    return num_en_ch *
           ina3221_num_samples[(ns / INA3221_NUM_SAMPLES_DIFF)] *
           conv;
}

static
void _update(powermon_t* mon)
{
    ina3221_enable_ch_t ench;
    ina3221_num_samples_t ns;
    ina3221_conv_time_bus_adc_t badc;
    ina3221_conv_time_shunt_adc_t sadc;
    ina3221_mode_t mode;
    ina3221_get_config(&mon->dev, &ench, &ns, &badc, &sadc, &mode);
    ina3221_channel_t ch = (((ench & INA3221_ENABLE_CH1) ? INA3221_CH1 : 0) |
                            ((ench & INA3221_ENABLE_CH2) ? INA3221_CH2 : 0) |
                            ((ench & INA3221_ENABLE_CH3) ? INA3221_CH3 : 0));
    uint32_t interval_us = _get_update_interval_us(ench, ns, badc, sadc, mode);
    DEBUG("update interval: " "%" PRIu32 "\n", interval_us);
    mon->channels = ch;
    mon->update_interval_us = interval_us;
    xtimer_remove(&mon->timeout);
    if (interval_us) {
        xtimer_set(&mon->timeout, mon->update_interval_us);
    }
}

static
void _measurement(powermon_t* mon)
{
    /* This function is executed in interrupt context */

    uint16_t flags = 0;
    powermon_measurement_result_t mresult = {0};
    powermon_result_t result = {0};
    ina3221_mode_t mode;
    int sstatus;
    int bstatus;
    ina3221_get_mode(&mon->dev, &mode);
    /* The conversion should be ready right away.
       The poll is only to mitigate possible inaccruracies. */
    uint8_t polls = MAX_POLLS;
    while (polls-- && !(flags & INA3221_FLAG_CONV_READY)) {
        int fstatus;
        if ((fstatus = ina3221_read_flags(&mon->dev, &flags)) != INA3221_OK) {
            if (mon->on_error) {
                char msg[16];
                snprintf(msg, sizeof(msg), "powermon: %d\n", fstatus);
                mon->on_error(mon, msg, mon->user_data);
            }
            goto RESCHEDULE;
        }
    }
    assert(flags & INA3221_FLAG_CONV_READY);
    if (mode == INA3221_MODE_CONTINUOUS_SHUNT_ONLY ||
        mode == INA3221_MODE_TRIGGER_SHUNT_ONLY    ||
        mode == INA3221_MODE_CONTINUOUS_SHUNT_BUS) {
        sstatus = ina3221_read_shunt_uv(&mon->dev, mon->channels,
                                        mresult.shunt_uv, NULL);
        if (sstatus < 0) {
            if (mon->on_error) {
                char msg[16];
                snprintf(msg, sizeof(msg), "powermon: %d\n", sstatus);
                mon->on_error(mon, msg, mon->user_data);
            }
            goto RESCHEDULE;
        }
        ina3221_calculate_current_ua(&mon->dev, mon->channels,
                                     mresult.shunt_uv, result.current_ua);
    }
    if (mode == INA3221_MODE_CONTINUOUS_BUS_ONLY ||
        mode == INA3221_MODE_TRIGGER_BUS_ONLY    ||
        mode == INA3221_MODE_CONTINUOUS_SHUNT_BUS) {
        bstatus = ina3221_read_bus_mv(&mon->dev, mon->channels,
                                      mresult.bus_mv, NULL);
        if (bstatus <= 0) {
            if (mon->on_error) {
                char msg[16];
                snprintf(msg, sizeof(msg), "powermon: %d\n", bstatus);
                mon->on_error(mon, msg, mon->user_data);
            }
            goto RESCHEDULE;
        }
    }
    if (mode == INA3221_MODE_CONTINUOUS_SHUNT_BUS) {
        ina3221_calculate_power_uw(mresult.bus_mv, result.current_ua,
                                   bstatus, result.power_uw);
    }
    if (mon->on_measurement) {
        mon->on_measurement(mon, &mresult, &result, mon->user_data);
    }

RESCHEDULE: {
        if (mode == INA3221_MODE_CONTINUOUS_SHUNT_ONLY ||
            mode == INA3221_MODE_CONTINUOUS_BUS_ONLY ||
            mode == INA3221_MODE_CONTINUOUS_SHUNT_BUS) {
            xtimer_set(&mon->timeout, mon->update_interval_us);
        }
    }
}

int powermon_init(void)
{
    unsigned success = 0;
    for (unsigned num = 0; num < INA3221_NUM; num++) {
        int init = ina3221_init(&_mons[num].dev, &ina3221_params[num]);
        if (init == INA3221_OK) {
            if (success < num) {
                _mons[success].dev = _mons[num].dev;
            }
            _mons[success].info = &_handle_infos[num];
            _mons[success].timeout.callback =
                (xtimer_callback_t)_measurement;
            _mons[success].timeout.arg = &_mons[success];
            _mons[success].channels = 0;
            _mons[success].on_measurement = NULL;
            _mons[success].on_config = NULL;
            _mons[success].user_data = NULL;
            success++;
        }
    }
    for (unsigned s = 0; s < success; s++) {
        _update(&_mons[s]);
    }
    return (_num_mons = success);
}

int powermon_configure(powermon_t* mon,
                       const ina3221_enable_ch_t* channels,
                       const ina3221_num_samples_t* samples,
                       const ina3221_conv_time_bus_adc_t* bus_adc,
                       const ina3221_conv_time_shunt_adc_t* shunt_adc,
                       const ina3221_mode_t* mode)
{
    powermon_config_t old_cfg;
    ina3221_get_config(&mon->dev, &old_cfg.channels, &old_cfg.samples,
                       &old_cfg.bus_adc, &old_cfg.shunt_adc, &old_cfg.mode);
    powermon_config_t new_cfg = old_cfg;
    if (channels) {
        DEBUG("Configure channel: %x\n", *channels);
        assert(!((*channels) & ~(INA3221_ENABLE_CH1 |
                                 INA3221_ENABLE_CH2 |
                                 INA3221_ENABLE_CH3)));
        new_cfg.channels = *channels;
    }
    if (samples) {
        DEBUG("Configure samples: %x\n", *samples);
        assert(!(*samples % INA3221_NUM_SAMPLES_DIFF) &&
               (*samples >= INA3221_NUM_SAMPLES_1) &&
               (*samples <= INA3221_NUM_SAMPLES_1024));
        new_cfg.samples = *samples;
    }
    if (bus_adc) {
        DEBUG("Configure bus conversion time: %x\n", *bus_adc);
        assert(!(*bus_adc % INA3221_CONV_TIME_BADC_DIFF) &&
               (*bus_adc >= INA3221_CONV_TIME_BADC_140US) &&
               (*bus_adc <= INA3221_CONV_TIME_BADC_8244US));
        new_cfg.bus_adc = *bus_adc;
    }
    if (shunt_adc) {
        DEBUG("Configure shunt conversion time: %x\n", *shunt_adc);
        assert(!(*shunt_adc % INA3221_CONV_TIME_SADC_DIFF) &&
               (*shunt_adc >= INA3221_CONV_TIME_SADC_140US) &&
               (*shunt_adc <= INA3221_CONV_TIME_SADC_8244US));
        new_cfg.shunt_adc = *shunt_adc;
    }
    if (mode) {
        DEBUG("Configure mode: %x\n", *mode);
        assert((*mode >= INA3221_MODE_POWER_DOWN) &&
               (*mode <= INA3221_MODE_CONTINUOUS_SHUNT_BUS));
        new_cfg.mode = *mode;
    }
    int cfg = ina3221_set_config(&mon->dev, new_cfg.channels, new_cfg.samples,
                                 new_cfg.bus_adc, new_cfg.shunt_adc, new_cfg.mode);
    if (cfg != INA3221_OK) {
        if (mon->on_error) {
            char msg[16];
            snprintf(msg, sizeof(msg), "powermon: %d\n", cfg);
            mon->on_error(mon, msg, mon->user_data);
        }
        return cfg;
    }
    if (mon->on_config) {
        mon->on_config(mon, &new_cfg, &old_cfg, mon->user_data);
    }
    _update(mon);
    return 0;
}

uint8_t powermon_get_num_mons(void)
{ return _num_mons; }

powermon_t* powermon_get_mon(uint8_t i)
{
    if (i < _num_mons) {
        return &_mons[i];
    }
    return NULL;
}

void powermon_set_on_measurement(powermon_t* mon, on_measurement_cb cb)
{ mon->on_measurement = cb; }

void powermon_set_on_config(powermon_t* mon, on_config_cb cb)
{ mon->on_config = cb; }

void powermon_set_on_error(powermon_t* mon, on_error_cb cb)
{ mon->on_error = cb; }

void powermon_set_user_data(powermon_t* mon, void* data)
{ mon->user_data = data; }

const ina3221_t* powermon_get_dev(const powermon_t* mon)
{ return &mon->dev; }

ina3221_channel_t powermon_get_channels(const powermon_t* mon)
{ return mon->channels; }

const powermon_info_t* powermon_get_info(const powermon_t* mon)
{ return mon->info; }
