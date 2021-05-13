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
 * @brief       Auto initialization of SPI NOR flash storage
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#include <assert.h>
#include "kernel_defines.h"
#include "board.h"
#include "mtd_spi_nor.h"
#if IS_USED(MODULE_MTD_SPI_NOR)
#include "mtd_spi_nor_params.h"
#else
#define MTD_SPI_NOR_NUMOF 0
#endif

#if MTD_SPI_NOR_NUMOF > 0
static mtd_spi_nor_t _mtd_spi_nor_devs[MTD_SPI_NOR_NUMOF] = {
    MTD_SPI_NOR_DEVS
};
#else
#define _mtd_spi_nor_devs   ((mtd_spi_nor_t *)NULL)
#define mtd_spi_nor_params  ((mtd_spi_nor_params_t *)NULL)
#endif

#if IS_USED(MODULE_MTD_SPI_NOR) || defined(DOXYGEN)
mtd_spi_nor_t *mtd_spi_nor_devs(void)
{
    return _mtd_spi_nor_devs;
}

spi_nor_id_t mtd_spi_nor_numof(void)
{
    return MTD_SPI_NOR_NUMOF;
}

static void _auto_init_mtd_spi_nor(mtd_spi_nor_t *mtd,
                                   const mtd_spi_nor_params_t *params,
                                   spi_nor_id_t numof)
{
    for (spi_nor_id_t i = 0; i < numof; i++) {
        mtd[i].base.driver = &mtd_spi_nor_driver;
        mtd[i].params = &params[i];
    }
}

/**
 * @internal    Additional setup of MTD SPI NOR chips
 *
 * @note This function has __attribute__((weak))
 *       so it can be overridden
 *
 * @param[in] spi_nor       Pointer to SPI NOR device
 * @param[in] numof         Number of SPI NOR devices
 */
void __attribute__((weak)) auto_init_mtd_spi_nor(mtd_spi_nor_t *spi_nor,
                                                 spi_nor_id_t numof)
{
    (void)spi_nor;
    (void)numof;
}
#else
#define _auto_init_mtd_spi_nor(...)
#define auto_init_mtd_spi_nor(...)
#endif

void auto_init_spi_nor(void)
{
    assert(SPI_NOR_ID_MAX >= MTD_SPI_NOR_NUMOF);

    _auto_init_mtd_spi_nor(_mtd_spi_nor_devs,
                           mtd_spi_nor_params,
                           MTD_SPI_NOR_NUMOF);
    auto_init_mtd_spi_nor(_mtd_spi_nor_devs, MTD_SPI_NOR_NUMOF);
}
