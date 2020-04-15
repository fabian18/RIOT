/*
 * Copyright (C) 2020 Otto-von-Guericke Universität
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */
/**
 * @ingroup     drivers_mtd_spi_nor
 * @{
 *
 * @file
 * @brief       MTD EEPROM driver implementation for at24cxxx EEPROM
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */
#include <errno.h>
#include <assert.h>

#include "mtd_eeprom.h"
#include "at24cxxx.h"

#define DEV(mtd_ptr)     (((mtd_eeprom_at24cxxx_t *)(mtd_ptr))->dev_ptr)

static int _mtd_at24cxxx_init(mtd_dev_t *mtd)
{
    assert(mtd);
    assert(mtd->driver == &mtd_at24cxxx_driver);
    assert(DEV(mtd));
    mtd->page_size = DEV(mtd)->params.page_size;
    mtd->pages_per_sector = 1;
    mtd->sector_count = DEV(mtd)->params.eeprom_size /
                        DEV(mtd)->params.page_size;
    return 0;
}

static int _mtd_at24cxxx_read(mtd_dev_t *mtd, void *dest, uint32_t addr,
                              uint32_t size)
{
    assert(mtd);
    assert(dest);
    int r = at24cxxx_read(DEV(mtd), addr, dest, size);
    return (r == AT24CXXX_OK) ? (int)size : r;
}

static int _mtd_at24cxxx_write(mtd_dev_t *mtd, const void *src, uint32_t addr,
                               uint32_t size)
{
    assert(mtd);
    assert(src);
    int w = at24cxxx_write(DEV(mtd), addr, src, size);
    return (w == AT24CXXX_OK) ? (int)size : w;
}

static int _mtd_at24cxxx_erase(mtd_dev_t *mtd, uint32_t addr, uint32_t size)
{
    assert(mtd);
    int w = at24cxxx_clear(DEV(mtd), addr, size);
    return (w == AT24CXXX_OK) ? (int)size : w;
}

static int _mtd_at24cxxx_power(mtd_dev_t *mtd, enum mtd_power_state power)
{
    (void)mtd;
    (void)power;
    return -ENOTSUP;
}

const mtd_desc_t mtd_at24cxxx_driver = {
    .init = _mtd_at24cxxx_init,
    .read = _mtd_at24cxxx_read,
    .write = _mtd_at24cxxx_write,
    .erase = _mtd_at24cxxx_erase,
    .power = _mtd_at24cxxx_power
};
