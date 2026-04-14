/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     sys_configuration_cfg_ieee802154
 * @{
 *
 * @file
 * @brief       IEEE 802.15.4 configuration
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 * @}
 */

#include "configuration.h"

#define _CFG_NO_INSTANCE
#include "cfg_riot_sid.h"

void cfg_ieee802154_instance(netdev_ieee802154_t *instance, conf_sid_t sid_stride)
{
    (void)instance;
    (void)sid_stride;
#if MODULE_CFG_IEEE802154_SECURITY
    cfg_ieee802154_security_instance(&instance->sec_ctx, sid_stride);
#endif
}
