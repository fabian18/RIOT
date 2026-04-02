/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @defgroup    sys_configuration_cfg_riot_sid   RIOT configuration SID definitions
 * @ingroup     sys_configuration
 * @{
 *
 * @file
 * @brief       RIOT configuration SID definitions
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 *
 * @}
 */

#include "configuration.h"
#include "macros/utils.h"

#include "cfg_riot.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_RIOT_KEY_BUF_MAX
#define CONFIG_RIOT_KEY_BUF_MAX                     80
#endif

#ifndef CONFIG_RIOT_LOWER_SID
#define CONFIG_RIOT_LOWER_SID                       1
#endif

#ifndef CONFIG_RIOT_NAME_PREFIX
#define CONFIG_RIOT_NAME_PREFIX
#endif

#  ifdef CONFIG_AT86RF215_LOWER_SID
#  undef CONFIG_AT86RF215_LOWER_SID
#  endif
#  define CONFIG_AT86RF215_LOWER_SID                (CONFIG_RIOT_LOWER_SID + 1)

#  ifdef CONFIG_AT86RF215_NAME_PREFIX
#  undef CONFIG_AT86RF215_NAME_PREFIX
#  endif
#  define CONFIG_AT86RF215_NAME_PREFIX              CONCAT(CONFIG_RIOT_NAME_PREFIX, _riot)

#  ifdef CONFIG_AT86RF215_DATA_LOC
#  undef CONFIG_AT86RF215_DATA_LOC
#  endif
#  define CONFIG_AT86RF215_DATA_LOC                 (((uint8_t *)(CONFIG_RIOT_DATA_LOC)) + offsetof(struct cfg_riot, at86rf215))

#  ifdef CONFIG_AT86RF215_NUMOF
#  undef CONFIG_AT86RF215_NUMOF
#  endif
#  define CONFIG_AT86RF215_NUMOF                    AT86RF215_NUMOF

#  include "cfg_at86rf215_sid.h"

#ifdef CONFIG_RIOT_UPPER_SID
#undef CONFIG_RIOT_UPPER_SID
#endif
#define CONFIG_RIOT_UPPER_SID                       (CONFIG_AT86RF215_UPPER_SID)

#ifndef _CFG_NO_INSTANCE

#ifndef CONFIG_RIOT_DATA_LOC
#error "CONFIG_RIOT_DATA_LOC must be defined before including cfg_riot.h"
#endif

/* riot */

#define CONFIG_RIOT_INSTANCE_HANDLER_NODE_ID_NAME(prefix) \
    CONCAT(prefix, riot_handler_id)

static CONF_HANDLER_ID(CONFIG_RIOT_INSTANCE_HANDLER_NODE_ID_NAME(CONFIG_RIOT_NAME_PREFIX),
                       CONFIG_RIOT_LOWER_SID,
                       "riot");

#define CONFIG_RIOT_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, riot_handler)

static CONF_HANDLER(CONFIG_RIOT_INSTANCE_HANDLER_NAME(CONFIG_RIOT_NAME_PREFIX),
                    &CONFIG_RIOT_INSTANCE_HANDLER_NODE_ID_NAME(CONFIG_RIOT_NAME_PREFIX),
                    NULL,
                    &configuration_default_backend_ops,
                    &configuration_default_backend_data_ops,
                    0,
                    sizeof(cfg_riot_t),
                    (CONFIG_RIOT_DATA_LOC));

static inline void cfg_riot_register(conf_handler_t *parent)
{
    if (parent) {
        configuration_register(parent, &CONFIG_RIOT_INSTANCE_HANDLER_NAME(CONFIG_RIOT_NAME_PREFIX));
    }
    CONFIG_AT86RF215_REGISTER_FUNCTION_NAME(CONFIG_AT86RF215_NAME_PREFIX)(
        &CONFIG_RIOT_INSTANCE_HANDLER_NAME(CONFIG_RIOT_NAME_PREFIX));
}

#endif /* _CFG_NO_INSTANCE */

#ifdef __cplusplus
}
#endif
