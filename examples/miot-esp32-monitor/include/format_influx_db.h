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
 * @brief       Functions to format output in influx DB format
 *
 * @author      Fabian Hüßler <afabian.huessler@ovgu.de>
 */

#ifndef FORMAT_INFLUX_DB_H
#define FORMAT_INFLUX_DB_H

#include "powermon.h"

#ifdef __cplusplus
extern "C" {
#endif

/* influx DB one line protocol:
 * powermon,channel=<ch_name> shunt_uv=<shunt_value> bus_mv=<bus_value> current_ua=<current_value> power_uw=<power_value>
 *
 * <ch_name>        is a descriptive name of the device which is measured
 * <shunt_value>    is shunt voltage drop in uV
 * <bus_value>      is bus voltage in mV
 * <current_ua>     is current in uA
 * <power_uw>       is power in uW
 */

#define MEASUREMENT                     "powermon"
#define CHANNEL_TAG_KEY                 "channel"
#define SHUNT_FIELD_KEY                 "shunt_uv"
#define BUS_FIELD_KEY                   "bus_mv"
#define CURRENT_FIELD_KEY               "current_ua"
#define POWER_FIELD_KEY                 "power_uw"

size_t format_influx_db_calculate_len(const char* channel_tag_value,
                                      int32_t shunt_uv,
                                      int16_t bus_mv,
                                      int32_t current_ua,
                                      int32_t power_uw);

int format_influx_db_write(char* buffer,
                           size_t size,
                           const char* channel_tag_value,
                           int32_t shunt_uv,
                           int16_t bus_mv,
                           int32_t current_ua,
                           int32_t power_uw);

#ifdef __cplusplus
}
#endif

#endif /* INFLUX_DB_FORMAT_H */
/** @} */
