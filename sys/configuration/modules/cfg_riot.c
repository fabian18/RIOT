/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     sys_configuration_cfg_riot
 * @{
 *
 * @file
 * @brief       Configuration
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 * @}
 */

#include "configuration.h"
#include "cfg_riot.h"

static cfg_riot_t cfg_riot;

#define CONFIG_RIOT_DATA_LOC       (&cfg_riot)

#include "cfg_riot_sid.h"

void auto_init_cfg_riot(void)
{
    cfg_riot_register(configuration_get_root());
}

AUTO_INIT_CONFIGURATION(auto_init_cfg_riot, AUTO_INIT_PRIO_MOD_CFG_RIOT);
