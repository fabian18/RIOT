/*
 * Copyright (C) 2026 ML!PA Consulting Gmbh
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     sys_configuration
 * @{
 *
 * @file
 * @brief       Implementation of the EEPROM registry configuration backend
 *
 * @author      Fabian Hüßler <fabian.huessler@ml-pa.com>
 *
 * @}
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "configuration.h"
#include "eepreg.h"
#include "fmt.h"
#include "mtd.h"

#include "configuration_backend_eepreg.h"

static int _be_eepreg_init(void)
{
    return mtd_init(CONFIGURATION_EEPREG_MTD);
}

static int _be_eepreg_reset(const struct conf_backend *be)
{
    (void)be;
    return eepreg_reset(CONFIGURATION_EEPREG_MTD);
}

static int _be_eepreg_load(const struct conf_backend *be,
                             conf_key_buf_t *key, void *val, size_t *size,
                             size_t offset, conf_backend_flags_t *flg)
{
    (void)be;
    (void)flg;
    if (offset) {
        /* reading partial records is not supported */
        return -ENOTSUP;
    }
    uint32_t pos;
    char sid_str[21];
    sid_str[fmt_u64_dec(sid_str, key->sid)] = '\0';
    int ret = eepreg_read(CONFIGURATION_EEPREG_MTD, &pos, sid_str);
    if (ret < 0) {
        return -EIO;
    }
    ret = mtd_read(CONFIGURATION_EEPREG_MTD, val, pos, *size);
    if (ret < 0) {
        return -EIO;
    }
    return 0;
}

static int _be_eepreg_store(const struct conf_backend *be,
                              conf_key_buf_t *key, const void *val, size_t *size,
                              size_t offset, conf_backend_flags_t *flg)
{
    (void)be;
    (void)flg;
    if (offset) {
        /* writing partial records is not supported */
        return -ENOTSUP;
    }
    uint32_t pos;
    char sid_str[21];
    sid_str[fmt_u64_dec(sid_str, key->sid)] = '\0';
    int ret = eepreg_add(CONFIGURATION_EEPREG_MTD, &pos, sid_str, *size);
    if (ret < 0) {
        return -EIO;
    }
    ret = mtd_write(CONFIGURATION_EEPREG_MTD, val, pos, *size);
    if (ret < 0) {
        return -EIO;
    }
    return 0;
}

static int _be_eepreg_delete(const struct conf_backend *be, conf_key_buf_t *key)
{
    (void)be;
    char sid_str[21];
    sid_str[fmt_u64_dec(sid_str, key->sid)] = '\0';
    int ret = eepreg_rm(CONFIGURATION_EEPREG_MTD, sid_str);
    if (ret < 0) {
        return -EIO;
    }
    return 0;
}

const conf_backend_ops_t conf_backend_eepreg_ops = {
    .be_load = _be_eepreg_load,
    .be_store = _be_eepreg_store,
    .be_delete = _be_eepreg_delete,
    .be_reset = _be_eepreg_reset,
};

int configuration_backend_eepreg_init(void)
{
    return _be_eepreg_init();
}

void auto_init_configuration_backend_eepreg(void)
{
    configuration_backend_eepreg_init();
}

#ifndef AUTO_INIT_PRIO_MOD_CONFIGURATION_BACKEND_EEPREG
#define AUTO_INIT_PRIO_MOD_CONFIGURATION_BACKEND_EEPREG     CONFIG_AUTO_INIT_CONFIGURATION_BACKEND_PRIO
#endif

AUTO_INIT_CONFIGURATION(auto_init_configuration_backend_eepreg,
                        AUTO_INIT_PRIO_MOD_CONFIGURATION_BACKEND_EEPREG);
