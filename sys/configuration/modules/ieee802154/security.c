/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     sys_configuration_cfg_ieee802154_security
 * @{
 *
 * @file
 * @brief       IEEE 802.15.4 security configuration
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 * @}
 */

#define _CFG_NO_INSTANCE
#include "cfg_riot_sid.h"

void cfg_ieee802154_security_instance(ieee802154_sec_context_t *instance, conf_sid_t sid_stride)
{
    instance->sid = sid_stride + CONFIG_IEEE802154_SECURITY_LOWER_SID;
}
