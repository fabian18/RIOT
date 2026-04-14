/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    sys_configuration_cfg_ieee802154   IEEE 802.15.4 configuration
 * @ingroup     sys_configuration
 * @{
 *
 * @file
 * @brief       IEEE 802.15.4 configuration
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 *
 * @}
 */

#include "configuration.h"
#include "cfg_ieee802154_security.h"
#include "net/ieee802154.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cfg_ieee802154 {
    cfg_ieee802154_security_t security;
} cfg_ieee802154_t;

void cfg_ieee802154_instance(netdev_ieee802154_t *instance, conf_sid_t sid_stride);

#ifdef __cplusplus
}
#endif
