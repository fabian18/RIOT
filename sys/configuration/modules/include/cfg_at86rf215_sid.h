/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    sys_configuration_cfg_at86rf215_sid         AT86RF215 configuration SID definitions
 * @ingroup     sys_configuration
 * @{
 *
 * @file
 * @brief       AT86RF215 configuration SID definitions
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 *
 * @}
 */

#include <stddef.h>
#include <stdint.h>

#include "configuration.h"
#include "macros/utils.h"

#include "cfg_at86rf215.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_AT86RF215_LOWER_SID
#error "CONFIG_AT86RF215_LOWER_SID must be defined before including cfg_at86rf215.h"
#endif

#define CONFIG_AT86RF215_INDEX_LOWER_SID                                1

#  ifdef CONFIG_IEEE802154_LOWER_SID
#  undef CONFIG_IEEE802154_LOWER_SID
#  endif
#  define CONFIG_IEEE802154_LOWER_SID                                   (CONFIG_AT86RF215_LOWER_SID + \
                                                                        CONFIG_AT86RF215_INDEX_LOWER_SID + 1)

#  ifdef CONFIG_IEEE802154_NAME_PREFIX
#  undef CONFIG_IEEE802154_NAME_PREFIX
#  endif
#  define CONFIG_IEEE802154_NAME_PREFIX                                 CONCAT(CONFIG_AT86RF215_NAME_PREFIX, _at86rf215)

#  ifdef CONFIG_IEEE802154_DATA_LOC
#  undef CONFIG_IEEE802154_DATA_LOC
#  endif
#  define CONFIG_IEEE802154_DATA_LOC                                    (((uint8_t *)(CONFIG_AT86RF215_DATA_LOC)) + offsetof(struct cfg_at86rf215, ieee802154))

#  include "cfg_ieee802154_sid.h"

#define CONFIG_AT86RF215_INDEX_STRIDE                                  ((CONFIG_IEEE802154_UPPER_SID - CONFIG_IEEE802154_LOWER_SID) + 1)

#ifdef CONFIG_AT86RF215_UPPER_SID
#undef CONFIG_AT86RF215_UPPER_SID
#endif
#define CONFIG_AT86RF215_UPPER_SID                                      (CONFIG_IEEE802154_LOWER_SID + \
                                                                        CONFIG_AT86RF215_INDEX_LOWER_SID + \
                                                                        (CONFIG_AT86RF215_INDEX_STRIDE * CONFIG_AT86RF215_NUMOF) - 1)

/* define this if no instance should be created */
#ifndef _CFG_NO_INSTANCE

#ifndef CONFIG_AT86RF215_NAME_PREFIX
#error "CONFIG_AT86RF215_NAME_PREFIX must be defined before including cfg_at86rf215.h"
#endif

#ifndef CONFIG_AT86RF215_DATA_LOC
#error "CONFIG_AT86RF215_DATA_LOC must be defined before including cfg_at86rf215.h"
#endif

#ifndef CONFIG_AT86RF215_NUMOF
#error "CONFIG_AT86RF215_NUMOF must be defined before including cfg_at86rf215.h"
#endif

/* at86rf215 */

#define CONFIG_AT86RF215_INSTANCE_ARRAY_ID_NAME(prefix) \
    CONCAT(prefix, _at86rf215_handler_id)

static CONF_HANDLER_ARRAY_ID(CONFIG_AT86RF215_INSTANCE_ARRAY_ID_NAME(CONFIG_AT86RF215_NAME_PREFIX),
                             CONFIG_AT86RF215_LOWER_SID, CONFIG_AT86RF215_UPPER_SID, \
                             CONFIG_AT86RF215_INDEX_STRIDE, "at86rf215");

#define CONFIG_AT86RF215_INSTANCE_ARRAY_HANDLER_NAME(prefix) \
    CONCAT(prefix, _at86rf215_handler)

MAYBE_UNUSED
static CONF_ARRAY_HANDLER(CONFIG_AT86RF215_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_AT86RF215_NAME_PREFIX),
                          &CONFIG_AT86RF215_INSTANCE_ARRAY_ID_NAME(CONFIG_AT86RF215_NAME_PREFIX),
                          NULL,
                          &configuration_default_backend_ops,
                          &configuration_default_backend_data_ops,
                          0,
                          sizeof(cfg_at86rf215_t),
                          (CONFIG_AT86RF215_DATA_LOC),
                          CONFIG_AT86RF215_NUMOF);

#define CONFIG_AT86RF215_REGISTER_FUNCTION_NAME(prefix) \
    CONCAT(prefix, _at86rf215_register)

static inline void CONFIG_AT86RF215_REGISTER_FUNCTION_NAME(CONFIG_AT86RF215_NAME_PREFIX)(conf_handler_t *parent)
{
    if (parent) {
        configuration_register(parent, &CONFIG_AT86RF215_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_AT86RF215_NAME_PREFIX).handler);
    }
    CONFIG_IEEE802154_REGISTER_FUNCTION_NAME(CONFIG_IEEE802154_NAME_PREFIX)(
        &CONFIG_AT86RF215_INSTANCE_ARRAY_HANDLER_NAME(CONFIG_AT86RF215_NAME_PREFIX).handler);
}

#endif /* _CFG_NO_INSTANCE */

#ifdef __cplusplus
}
#endif
