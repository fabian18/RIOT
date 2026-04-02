/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    sys_configuration_cfg_riot         RIOT configuration
 * @ingroup     sys_configuration
 * @{
 *
 * @file
 * @brief       RIOT configuration
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 *
 * @}
 */

#include "auto_init_utils.h"
#include "configuration.h"
#include "cfg_at86rf215.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AUTO_INIT_PRIO_MOD_CFG_RIOT
#define AUTO_INIT_PRIO_MOD_CFG_RIOT                 AUTO_INIT_PRIORITY_AFTER( \
                                                        CONFIG_AUTO_INIT_CONFIGURATION_BACKEND_PRIO)
#endif

typedef struct cfg_riot {
    cfg_at86rf215_t at86rf215[AT86RF215_NUMOF];
} cfg_riot_t;

#ifdef __cplusplus
}
#endif
