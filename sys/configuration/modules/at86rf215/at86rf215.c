/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     sys_configuration_cfg_at86rf215
 * @{
 *
 * @file
 * @brief       AT86RF215 configuration
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 * @}
 */

#include "at86rf215.h"

#define _CFG_NO_INSTANCE
#include "cfg_riot_sid.h"

void cfg_at86rf215_instance(at86rf215_t *instance, conf_sid_t sid_stride)
{
    (void)instance;
    (void)sid_stride;
#if MODULE_CFG_IEEE802154
    cfg_ieee802154_instance(&instance->netdev, sid_stride);
#endif
}
