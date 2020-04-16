/*
 * Copyright (C) 2020 Otto-von-Guericke-Universität Magdeburg
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
 * @brief       Shell application to configure miot-esp32 power monitoring
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#define ENABLE_DEBUG    (0)
#include "debug.h"

#include "include/ina3221_tables.h"
#include "include/powermon.h"

static const char* _modes[] = {
    "power_down",
    "trigger_shunt_only",
    "trigger_bus_only",
    "trigger_shunt_bus",
    "shunt_only",
    "bus_only",
    "shunt_bus"
};

static inline void _print_help(void)
{
    puts(
        "powermon <dev> [-c [... 1 2 3 ...]]\n"
        "               [-n <num_samples>]\n"
        "               [-s <shunt_conversion_time>]\n"
        "               [-b <bus_conversion_time>]\n"
        "               [-m <power_down |\n"
        "                    trigger_shunt_only |\n"
        "                    trigger_bus_only |\n"
        "                    trigger_shunt_bus |\n"
        "                    shunt_only |\n"
        "                    bus_only |\n"
        "                    shunt_bus>]\n"
    );
}

static
int _parse_channel(int* argc, char** arg, powermon_config_t* cfg)
{
    assert(argc && *argc); assert(arg && *arg); assert(cfg);
    assert(!strcmp(*arg, "-c"));
    DEBUG("%s: argc: %d\n", "_parse_channel", *argc);
    char** a = arg;
    int argi = *argc - 1; /* at lease 1 processed because of "-c" */
    int ch;
    if (*argc < 1) {
        *argc = 0;
        return -EINVAL;
    }
    while (argi && ++a) {
        DEBUG("arg: %s\n", *a);
        char* c = *a;
        if (isdigit(c[0]) && !c[1]) {
            ch = c[0] - '0';
            if (ch == 1) {
                cfg->channels |= INA3221_ENABLE_CH1;
            }
            else if (ch == 2) {
                cfg->channels |= INA3221_ENABLE_CH2;
            }
            else if (ch == 3) {
                cfg->channels |= INA3221_ENABLE_CH3;
            }
            else {
                *argc = *argc - argi;
                return -EINVAL;
            }
        }
        else {
            /* arg was next arg */
            *argc = *argc - argi;
            return 0;
        }
        argi--;
    }
    /* end of all args */
    *argc = *argc - argi;
    return 0;
}

static
int _parse_num(int* argc, char** arg, powermon_config_t* cfg)
{
    assert(argc && *argc); assert(arg && *arg); assert(cfg);
    assert(!strcmp(*arg, "-n"));
    DEBUG("%s: argc: %d\n", "_parse_num", *argc);
    char** a = arg;
    int argi = *argc - 1; /* at lease 1 processed because of "-n" */
    int num = 0;
    if (*argc < 2) {
        *argc = 0;
        return -EINVAL;
    }
    if (argi && ++a) {
        DEBUG("arg: %s\n", *a);
        char* c = *a;
        unsigned i;
        for (i = 0; c[i]; i++) {
            if (isdigit(c[i])) {
                num *= 10;
                num += (c[i] - '0');
            }
            else {
                *argc = *argc - argi;
                return -EINVAL;
            }
        }
        argi--;
        for (i = 0;
             i < ARRAY_SIZE(ina3221_num_samples) - 1 &&
             num > ina3221_num_samples[i];
             i++) {}
        cfg->samples = i * (INA3221_NUM_SAMPLES_DIFF);
    }
    *argc = *argc - argi;
    DEBUG("%s: num: %d\n", "_parse_num", num);
    return 0;
}

static
int _parse_sadc(int* argc, char** arg, powermon_config_t* cfg)
{
    assert(argc && *argc); assert(arg && *arg); assert(cfg);
    assert(!strcmp(*arg, "-s"));
    DEBUG("%s: argc: %d\n", "_parse_sadc", *argc);
    char** a = arg;
    int argi = *argc - 1; /* at lease 1 processed because of "-s" */
    int sadc = 0;
    if (*argc < 2) {
        *argc = 0;
        return -EINVAL;
    }
    if (argi && ++a) {
        char* c = *a;
        unsigned i;
        for (i = 0; c[i]; i++) {
            if (isdigit(c[i])) {
                sadc *= 10;
                sadc += (c[i] - '0');
            }
            else {
                *argc = *argc - argi;
                return -EINVAL;
            }
        }
        argi--;
        for (i = 0;
             i < ARRAY_SIZE(ina3221_conv_time_sadc) - 1 &&
             sadc > ina3221_conv_time_sadc[i];
             i++) {}
        cfg->shunt_adc = i * (INA3221_CONV_TIME_SADC_DIFF);
    }
    *argc = *argc - argi;
    DEBUG("%s: sadc: %d\n", "_parse_sadc", sadc);
    return 0;
}

static
int _parse_badc(int* argc, char** arg, powermon_config_t* cfg)
{
    assert(argc && *argc); assert(arg && *arg); assert(cfg);
    assert(!strcmp(*arg, "-b"));
    DEBUG("%s: argc: %d\n", "_parse_badc", *argc);
    char** a = arg;
    int argi = *argc - 1; /* at least 1 processed argument because of "-b" */
    int badc = 0;
    if (*argc < 2) {
        *argc = 0;
        return -EINVAL;
    }
    if (argi && ++a) {
        char* c = *a;
        unsigned i;
        for (i = 0; c[i]; i++) {
            if (isdigit(c[i])) {
                badc *= 10;
                badc += (c[i] - '0');
            }
            else {
                *argc = *argc - argi;
                return -EINVAL;
            }
        }
        argi--;
        for (i = 0;
             i < ARRAY_SIZE(ina3221_conv_time_badc) - 1 &&
             badc > ina3221_conv_time_badc[i];
             i++) {}
        cfg->bus_adc = i * (INA3221_CONV_TIME_BADC_DIFF);
    }
    *argc = *argc - argi;
    DEBUG("%s: badc: %d\n", "_parse_badc", badc);
    return 0;
}

static
int _parse_mode(int* argc, char** arg, powermon_config_t* cfg)
{
    assert(argc && *argc); assert(arg && *arg); assert(cfg);
    assert(!strcmp(*arg, "-m"));
    DEBUG("%s: argc: %d\n", "_parse_mode", *argc);
    char** a = arg;
    int argi = *argc - 1; /* at least 1 processed argument because of "-m" */
    if (*argc < 2) {
        *argc = 0;
        return -EINVAL;
    }
    if (argi && ++a) {
        char* c = *a;
        DEBUG("%s: mode: %s\n", "_parse_mode", c);
        if(!strcmp(c, _modes[0])) {
            cfg->mode = INA3221_MODE_POWER_DOWN;
        }
        else if (!strcmp(c, _modes[1])) {
            cfg->mode = INA3221_MODE_TRIGGER_SHUNT_ONLY;
        }
        else if (!strcmp(c, _modes[2])) {
            cfg->mode = INA3221_MODE_TRIGGER_BUS_ONLY;
        }
        else if (!strcmp(c, _modes[3])) {
            cfg->mode = INA3221_MODE_TRIGGER_SHUNT_BUS;
        }
        else if (!strcmp(c, _modes[4])) {
            cfg->mode = INA3221_MODE_CONTINUOUS_SHUNT_ONLY;
        }
        else if (!strcmp(c, _modes[5])) {
            cfg->mode = INA3221_MODE_CONTINUOUS_BUS_ONLY;
        }
        else if (!strcmp(c, _modes[6])) {
            cfg->mode = INA3221_MODE_CONTINUOUS_SHUNT_BUS;
        }
        else {
            *argc = *argc - argi;
            return -EINVAL;
        }
        argi--;
    }
    DEBUG("%s: mode: %s\n", "_parse_mode", *a);
    *argc = *argc - argi;
    return 0;
}

int sc_powermon(int argc, char* argv[])
{
    powermon_config_t pm_cfg = {0};
    ina3221_enable_ch_t* channels = NULL;
    ina3221_num_samples_t* samples = NULL;
    ina3221_conv_time_bus_adc_t* bus_adc = NULL;
    ina3221_conv_time_shunt_adc_t* shunt_adc = NULL;
    ina3221_mode_t* mode = NULL;
    char** arg_end = argv + argc;
    char** arg_ptr;
    int argi = 1;
    int dev_index;
    int err = 0;
    if (argc <= 1) {
        return -EINVAL;
    }
    else if ((dev_index = atoi(*(arg_ptr = argv + argi)))
            >= (int)powermon_get_num_mons() ||
            dev_index < 0) {
        return -EFAULT;
    }
    while (!err && ((arg_ptr += argi) < arg_end)) {
        argi = (arg_end - arg_ptr);
        if ((*arg_ptr)[0] == '-' && (*arg_ptr)[1] && !(*arg_ptr)[2]) {
            switch ((*arg_ptr)[1]) {
                case 'c':
                    channels = &pm_cfg.channels;
                    err = _parse_channel(&argi, arg_ptr, &pm_cfg);
                    break;
                case 'n':
                    samples = &pm_cfg.samples;
                    err = _parse_num(&argi, arg_ptr, &pm_cfg);
                    break;
                case 's':
                    shunt_adc = &pm_cfg.shunt_adc;
                    err = _parse_sadc(&argi, arg_ptr, &pm_cfg);
                    break;
                case 'b':
                    bus_adc = &pm_cfg.bus_adc;
                    err = _parse_badc(&argi, arg_ptr, &pm_cfg);
                    break;
                case 'm':
                    mode = &pm_cfg.mode;
                    err = _parse_mode(&argi, arg_ptr, &pm_cfg);
                    break;
                default:
                    //argi = 1; /* skip 1 arg ?*/
                    argi = 0;
                    err = -EINVAL;
                    DEBUG("Unknown argument %s\n", *arg_ptr);
            }
        }
        else {
            //argi = 1; /* skip 1 arg */
            argi = 0;
            err = -EINVAL;
            DEBUG("Not an argument %s\n", *arg_ptr);
        }
    }
    if (err) {
        assert(arg_ptr + argi < arg_end);
        DEBUG("Error at arument: %s\n", *(arg_ptr + argi));
        _print_help();
        return err;
    }
    if ((err = powermon_configure(powermon_get_mon(dev_index),
                                  channels, samples, bus_adc,
                                  shunt_adc, mode))) {
        DEBUG("Configuration failed\n");
        return err;
    }
    return 0;
}