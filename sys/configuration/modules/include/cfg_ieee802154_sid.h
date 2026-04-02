/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @defgroup    sys_configuration_cfg_ieee802154_sid    IEEE 802.15.4 configuration SID definitions
 * @ingroup     sys_configuration
 * @{
 *
 * @file
 * @brief       IEEE 802.15.4 configuration SID definitions
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 *
 * @}
 */

#include <stddef.h>

#include "configuration.h"
#include "macros/utils.h"

#include "cfg_ieee802154.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_IEEE802154_LOWER_SID
#error "CONFIG_IEEE802154_LOWER_SID must be defined before including cfg_ieee802154.h"
#endif

#  ifdef CONFIG_IEEE802154_SECURITY_LOWER_SID
#  undef CONFIG_IEEE802154_SECURITY_LOWER_SID
#  endif
#  define CONFIG_IEEE802154_SECURITY_LOWER_SID                  (CONFIG_IEEE802154_LOWER_SID + 1)

#  ifdef CONFIG_IEEE802154_SECURITY_NAME_PREFIX
#  undef CONFIG_IEEE802154_SECURITY_NAME_PREFIX
#  endif
#  define CONFIG_IEEE802154_SECURITY_NAME_PREFIX                CONCAT(CONFIG_IEEE802154_NAME_PREFIX, _ieee802154)

#  ifdef CONFIG_IEEE802154_SECURITY_DATA_LOC
#  undef CONFIG_IEEE802154_SECURITY_DATA_LOC
#  endif
#  define CONFIG_IEEE802154_SECURITY_DATA_LOC                   (((uint8_t *)(CONFIG_IEEE802154_DATA_LOC)) + offsetof(struct cfg_ieee802154, security))

#  include "cfg_ieee802154_security_sid.h"

#ifdef CONFIG_IEEE802154_UPPER_SID
#undef CONFIG_IEEE802154_UPPER_SID
#endif
#define CONFIG_IEEE802154_UPPER_SID                             (CONFIG_IEEE802154_SECURITY_UPPER_SID)

/* define this if no instance should be created */
#ifndef _CFG_NO_INSTANCE

#ifndef CONFIG_IEEE802154_NAME_PREFIX
#error "CONFIG_IEEE802154_NAME_PREFIX must be defined before including cfg_ieee802154.h"
#endif

#ifndef CONFIG_IEEE802154_DATA_LOC
#error "CONFIG_IEEE802154_DATA_LOC must be defined before including cfg_ieee802154.h"
#endif

/* ieee802154 */

#define CONFIG_IEEE802154_INSTANCE_HANDLER_NODE_ID_NAME(prefix) \
    CONCAT(prefix, _ieee802154_handler_id)

static CONF_HANDLER_NODE_ID(CONFIG_IEEE802154_INSTANCE_HANDLER_NODE_ID_NAME(CONFIG_IEEE802154_NAME_PREFIX),
                            CONFIG_IEEE802154_LOWER_SID, CONFIG_IEEE802154_UPPER_SID,
                            "ieee802154");

#define CONFIG_IEEE802154_INSTANCE_HANDLER_NAME(prefix) \
    CONCAT(prefix, _ieee802154_handler)

static CONF_HANDLER(CONFIG_IEEE802154_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_NAME_PREFIX),
                    &CONFIG_IEEE802154_INSTANCE_HANDLER_NODE_ID_NAME(CONFIG_IEEE802154_NAME_PREFIX),
                    NULL,
                    &configuration_default_backend_ops,
                    &configuration_default_backend_data_ops,
                    0,
                    sizeof(cfg_ieee802154_t),
                    (CONFIG_IEEE802154_DATA_LOC));

#define CONFIG_IEEE802154_REGISTER_FUNCTION_NAME(prefix) \
    CONCAT(prefix, _ieee802154_register)

static inline void CONFIG_IEEE802154_REGISTER_FUNCTION_NAME(CONFIG_IEEE802154_NAME_PREFIX)(conf_handler_t *parent)
{
    if (parent) {
        configuration_register(parent, &CONFIG_IEEE802154_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_NAME_PREFIX));
    }
    CONFIG_IEEE802154_SECURITY_REGISTER_FUNCTION_NAME(CONFIG_IEEE802154_SECURITY_NAME_PREFIX)(
        &CONFIG_IEEE802154_INSTANCE_HANDLER_NAME(CONFIG_IEEE802154_NAME_PREFIX));
}

#endif /* _CFG_NO_INSTANCE */

#ifdef __cplusplus
}
#endif
