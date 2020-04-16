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
 * @brief       Functions to format output in influx DB format
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "include/format_influx_db.h"

size_t format_influx_db_calculate_len(const char* channel_tag_value,
                                      int32_t shunt_uv,
                                      int16_t bus_mv,
                                      int32_t current_ua,
                                      int32_t power_uw)
{
    return strlen(MEASUREMENT",") +
           strlen(CHANNEL_TAG_KEY"=") +
           strlen(channel_tag_value) +
           1 +   /* white space */
           strlen(SHUNT_FIELD_KEY"=") +
           snprintf(NULL, 0, "%"PRId32, shunt_uv) +
           1 +   /* comma */
           strlen(BUS_FIELD_KEY"=") +
           snprintf(NULL, 0, "%"PRId16, bus_mv) +
           1 +   /* comma */
           strlen(CURRENT_FIELD_KEY"=") +
           snprintf(NULL, 0, "%"PRId32, current_ua) +
           1 +   /* comma */
           strlen(POWER_FIELD_KEY"=") +
           snprintf(NULL, 0, "%"PRId32, power_uw) +
           1     /* \0 */;
}

int format_influx_db_write(char* buffer,
                           size_t size,
                           const char* channel_tag_value,
                           int32_t shunt_uv,
                           int16_t bus_mv,
                           int32_t current_ua,
                           int32_t power_uw)
{
    return snprintf(buffer, size,
                    MEASUREMENT","              \
                    CHANNEL_TAG_KEY"=""%s"" "   \
                    SHUNT_FIELD_KEY"=""%d"","   \
                    BUS_FIELD_KEY"=""%d"","     \
                    CURRENT_FIELD_KEY"=""%d"","    \
                    POWER_FIELD_KEY"=""%d",
                    channel_tag_value,
                    shunt_uv,
                    bus_mv,
                    current_ua,
                    power_uw);
}
