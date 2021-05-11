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
 * @brief       Auto initialization of SPI sdcards
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#include <assert.h>
#include "log.h"
#include "kernel_defines.h"
#include "sdcard_spi.h"
#include "sdcard_spi_params.h"
#include "mtd_sdcard.h"
#if IS_USED(MODULE_MTD_SDCARD)
#include "mtd_sdcard_params.h"
#else
#define MTD_SDCARD_NUMOF 0
#endif

#define SDCARD_SPI_NUMOF            ARRAY_SIZE(sdcard_spi_params)

static sdcard_spi_t _sdcard_spi_devs[SDCARD_SPI_NUMOF];

#if MTD_SDCARD_NUMOF > 0
static mtd_sdcard_t _mtd_sdcard_devs[MTD_SDCARD_NUMOF] = {
    MTD_SDCARD_DEVS
};
#else
#define _mtd_sdcard_devs          ((mtd_sdcard_t *)NULL)
#endif

sdcard_spi_t *sdcard_spi_devs(void) {
    return _sdcard_spi_devs;
}
sdcard_spi_id_t sdcard_spi_numof(void) {
    return SDCARD_SPI_NUMOF;
}

#if IS_USED(MODULE_MTD_SDCARD) || defined(DOXYGEN)
mtd_sdcard_t *mtd_sdcard_devs(void) {
    return _mtd_sdcard_devs;
}
sdcard_spi_id_t mtd_sdcard_numof(void) {
    return MTD_SDCARD_NUMOF;
}

static void _auto_init_mtd_sdcard(mtd_sdcard_t *mtd,
                                  sdcard_spi_t *devs,
                                  const sdcard_spi_params_t *params,
                                  sdcard_spi_id_t numof)
{
    for (sdcard_spi_id_t i = 0; i < numof; i++) {
        mtd[i].base.driver = &mtd_sdcard_driver;
        mtd[i].sd_card = &devs[i];
        mtd[i].params = &params[i];
    }
}

/**
 * @internal    Additional setup sdcard MTD,
 *              e.g. to be implemented by the board
 *
 * @note This function has __attribute__((weak)) so it can be overridden
 *
 * @param[in] sdcard        Pointer to sdcard devices
 * @param[in] numof         Number of sdcard devices
 */
void __attribute__((weak)) auto_init_mtd_sdcard(mtd_sdcard_t *sdcard,
                                                sdcard_spi_id_t numof)
{
    (void)sdcard;
    (void)numof;
}
#else
#define _auto_init_mtd_sdcard(...)
#define auto_init_mtd_sdcard(...)
#endif

static void _auto_init_sdcard_spi(sdcard_spi_t *devs,
                                  const sdcard_spi_params_t *params,
                                  sdcard_spi_id_t numof)
{
    int init;
    for (sdcard_spi_id_t i = 0; i < numof; i++) {
        if ((init = sdcard_spi_init(&devs[i], &params[i]))) {
            LOG_DEBUG("[auto_init_sdcard_spi]: "
                      "failed to initialize device %d due to reason %d",
                      devs - _sdcard_spi_devs, init);
        }
    }
}

void auto_init_sdcard_spi(void)
{
    assert(SDCARD_SPI_ID_MAX >= SDCARD_SPI_NUMOF);
    assert(MTD_SDCARD_NUMOF <= SDCARD_SPI_NUMOF);

    const sdcard_spi_params_t *params = sdcard_spi_params;
    sdcard_spi_t *devs = _sdcard_spi_devs;

    _auto_init_mtd_sdcard(_mtd_sdcard_devs, devs, params, MTD_SDCARD_NUMOF);
    _auto_init_sdcard_spi(devs + MTD_SDCARD_NUMOF,
                          params + MTD_SDCARD_NUMOF,
                          SDCARD_SPI_NUMOF - MTD_SDCARD_NUMOF);

    auto_init_mtd_sdcard(_mtd_sdcard_devs, MTD_SDCARD_NUMOF);
}
