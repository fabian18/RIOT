/*
 * Copyright (C) 2021 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */
/**
 * @ingroup     sys_auto_init_storage
 * @{
 *
 * @file
 * @brief       Auto initialization of at25xxx devices
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#include <assert.h>
#include "log.h"
#include "kernel_defines.h"
#include "mtd.h"
#include "at25xxx.h"
#include "at25xxx_params.h"
#include "at25xxx/mtd.h"
#include "mtd_at25xxx_params.h"

#define AT25XXX_NUMOF   ARRAY_SIZE(at25xxx_params)

static at25xxx_t _at25xxx_devs[AT25XXX_NUMOF];

#if MTD_AT25XXX_NUMOF > 0
static mtd_at25xxx_t _mtd_at25xxx_devs[MTD_AT25XXX_NUMOF];
#else
#define _mtd_at25xxx_devs           ((mtd_at25xxx_t *)NULL)
#endif

at25xxx_t *at25xxx_devs(void) {
    return _at25xxx_devs;
}
at25xxx_id_t at25xxx_numof(void) {
    return AT25XXX_NUMOF;
}

#if IS_USED(MODULE_MTD_AT25XXX) || defined(DOXYGEN)
mtd_at25xxx_t *mtd_at25xxx_devs(void) {
    return _mtd_at25xxx_devs;
}
at25xxx_id_t mtd_at25xxx_numof(void) {
    return MTD_AT25XXX_NUMOF;
}

static void _auto_init_mtd_at25xxx(mtd_at25xxx_t *mtd,
                                   at25xxx_t *devs,
                                   const at25xxx_params_t *params,
                                   at25xxx_id_t numof)
{
    for (at25xxx_id_t i = 0; i < numof; i++) {
        mtd[i].base.driver = &mtd_at25xxx_driver;
        mtd[i].at25xxx_eeprom = &devs[i];
        mtd[i].params = &params[i];
    }
}

/**
 * @internal    Additional setup at25xxx MTD,
 *              e.g. to be implemented by the board
 *
 * @note This function has __attribute__((weak)) so it can be overridden
 *
 * @param[in] at25xxx       Pointer to at25xxx device
 * @param[in] numof         Number of at25xxx devices
 */
void __attribute__((weak)) auto_init_mtd_at25xxx(mtd_at25xxx_t *at25xxx,
                                                 at25xxx_id_t numof)
{
    (void)at25xxx;
    (void)numof;
}
#else
#define _auto_init_mtd_at25xxx(...)
#define auto_init_mtd_at25xxx(...)
#endif

static void _auto_init_at25xxx(at25xxx_t *devs,
                               const at25xxx_params_t *params,
                               at25xxx_id_t numof)
{
    for (at25xxx_id_t i = 0; i < numof; i++) {
        int init;
        if ((init = at25xxx_init(&devs[i], &params[i]))) {
            LOG_DEBUG("[auto_init_at25xxx]: "
                      "failed to initialize device %d due to reason %d",
                      devs - _at25xxx_devs, init);
        }
    }
}

void auto_init_at25xxx(void)
{
    assert(AT25XXX_ID_MAX >= AT25XXX_NUMOF);
    assert(MTD_AT25XXX_NUMOF <= AT25XXX_NUMOF);

    const at25xxx_params_t *params = at25xxx_params;
    at25xxx_t *devs = _at25xxx_devs;

    _auto_init_mtd_at25xxx(_mtd_at25xxx_devs, devs, params, MTD_AT25XXX_NUMOF);
    _auto_init_at25xxx(devs + MTD_AT25XXX_NUMOF,
                       params + MTD_AT25XXX_NUMOF,
                       AT25XXX_NUMOF - MTD_AT25XXX_NUMOF);

    auto_init_mtd_at25xxx(_mtd_at25xxx_devs, MTD_AT25XXX_NUMOF);
}
