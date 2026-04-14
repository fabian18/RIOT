/*
 * SPDX-FileCopyrightText: 2026 ML!PA Consulting GmbH.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    sys_configuration_cfg_at86rf215         AT86RF215 configuration
 * @ingroup     sys_configuration
 * @{
 *
 * @file
 * @brief       AT86RF215 configuration
 *
 * @author      Fabian Hüßler <fabian.huessler@mlpa.com>
 *
 * @}
 */

#include "at86rf215.h"
#include "at86rf215_params.h"
#include "configuration.h"
#include "cfg_ieee802154.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AT86RF215_BANDS     (IS_USED(MODULE_AT86RF215_SUBGHZ) + IS_USED(MODULE_AT86RF215_24GHZ))
#define AT86RF215_NUMOF     (ARRAY_SIZE(at86rf215_params) * AT86RF215_BANDS)

typedef struct cfg_at86rf215 {
    cfg_ieee802154_t ieee802154;
} cfg_at86rf215_t;

void cfg_at86rf215_instance(at86rf215_t *instance, conf_sid_t sid_stride);

#ifdef __cplusplus
}
#endif
