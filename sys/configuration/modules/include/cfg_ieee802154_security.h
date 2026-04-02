/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    sys_configuration_cfg_ieee802154_security       IEEE 802.15.4 security configuration
 * @ingroup     sys_configuration_cfg_ieee802154_security
 * @{
 *
 * @file
 * @brief       IEEE 802.15.4 security configuration
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 *
 * @}
 */

#include "configuration.h"
#include "net/ieee802154_security.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef ieee802154_security_config_t cfg_ieee802154_security_t;

void cfg_ieee802154_security_instance(ieee802154_sec_context_t *instance, conf_sid_t sid_stride);

#ifdef __cplusplus
}
#endif
