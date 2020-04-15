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
 * @brief       MTD EEPROM driver implementation for MCU integrated EEPROM
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */
#include <errno.h>
#include <assert.h>

#include "periph_cpu.h"

/* Do not fail compilation if CPU does not provide EEPROM_SIZE */
#ifndef EEPROM_SIZE
#define EEPROM_SIZE (0)
#endif

#include "mtd_eeprom.h"
#include "periph/eeprom.h"

static int _mtd_periph_init(mtd_dev_t *mtd)
{
    assert(mtd);
    assert(mtd->driver == &mtd_periph_driver);
    mtd->page_size = EEPROM_SIZE;
    mtd->pages_per_sector = 1;
    mtd->sector_count = 1;
    return 0;
}

static int _mtd_periph_read(mtd_dev_t *mtd, void *dest, uint32_t addr,
                            uint32_t size)
{
    assert(mtd);
    assert(dest);
    if (size > EEPROM_SIZE || EEPROM_SIZE - size < addr) {
        return -EFAULT;
    }
    return eeprom_read(addr, dest, size);
}

static int _mtd_periph_write(mtd_dev_t *mtd, const void *src, uint32_t addr,
                             uint32_t size)
{
    assert(mtd);
    assert(src);
    if (size > EEPROM_SIZE || EEPROM_SIZE - size < addr) {
        return -EFAULT;
    }
    return eeprom_write(addr, src, size);
}

static int _mtd_periph_erase(mtd_dev_t *mtd, uint32_t addr, uint32_t size)
{
    assert(mtd);
    if (size > EEPROM_SIZE || EEPROM_SIZE - size < addr) {
        return -EFAULT;
    }
    return eeprom_clear(addr, size);
}

static int _mtd_periph_power(mtd_dev_t *mtd, enum mtd_power_state power)
{
    (void)mtd;
    (void)power;
    return -ENOTSUP;
}

const mtd_desc_t mtd_periph_driver = {
    .init = _mtd_periph_init,
    .read = _mtd_periph_read,
    .write = _mtd_periph_write,
    .erase = _mtd_periph_erase,
    .power = _mtd_periph_power
};
