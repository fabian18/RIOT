/*
 * Copyright (C) 2018 Acutam Automation, LLC
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       eepreg test application
 *
 * @author      Matthew Blue <matthew.blue.neuro@gmail.com>
 * @}
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "kernel_defines.h"
#include "eepreg.h"

#if IS_USED(MODULE_AT24CXXX)
#include "at24cxxx_params.h"
#elif IS_USED(MODULE_AT25XXX)
#include "at25xxx_params.h"
#endif

#define MTD_PTR(mtd)    ((mtd_dev_t *)(mtd))
#define EEPROM_PTR(mtd) ((mtd_eeprom_t *)(mtd))
#define ENT1_NAME       "foo"
#define ENT1_SIZE       (12U)
#define ENT2_NAME       "bar"
#define ENT2_SIZE       (34U)
#define DATA            "spam and eggs"

static inline uint32_t _eeprom_size(const mtd_eeprom_t *eeprom)
{
    return eeprom->base.sector_count *
           eeprom->base.pages_per_sector *
           eeprom->base.page_size;
}

static inline uint32_t _data_start(const mtd_eeprom_t *eeprom)
{
    return _eeprom_size(eeprom) -
           EEPROM_RESERV_CPU_HI -
           EEPROM_RESERV_BOARD_HI -
           1;
}

static inline uint8_t _ptr_len(const mtd_eeprom_t *eeprom) {
    uint32_t size = _eeprom_size(eeprom);
    if (size > 0x1000000) {
        return 4U;
    }
    else if (size > 0x10000) {
        return 3U;
    }
    else if (size > 0x100) {
        return 2U;
    }
    else {
        return 1U;
    }
}

int eepreg_iter_cb(char *name, void *arg)
{
    (void)arg;

    printf("%s ", name);

    return 0;
}

int main(void)
{
    int ret;
    uint32_t tmp1, tmp2, tmp3;
    char data[sizeof(DATA)];

#if IS_USED(MODULE_AT24CXXX)
    at24cxxx_t dev;
    if ((ret = at24cxxx_init(&dev, &at24cxxx_params[0])) != 0) {
        return ret;
    }
    mtd_eeprom_at24cxxx_t mtd = MTD_EEPROM_AT24CXXX_INIT(&dev);
#elif IS_USED(MODULE_AT25XXX)
    at25xxx_t dev;
    if ((ret = at25xxx_init(&dev, &at25xxx_params[0])) != 0) {
        return ret;
    }
    mtd_eeprom_at25xxx_t mtd = MTD_EEPROM_AT25XXX_INIT(&dev);
#else
    mtd_eeprom_t mtd = MTD_EEPROM_PERIPH_INIT;
#endif
    if ((ret = eepreg_init(EEPROM_PTR(&mtd)))) {
        puts("Initialization of EEPROM registry failed");
        return ret;
    }

    puts("EEPROM registry (eepreg) test routine");

    printf("Testing new registry creation: ");

    printf("reset ");
    ret = eepreg_reset();
    if (ret < 0) {
        puts("[FAILED]");
        return 1;
    }

    printf("check ");
    ret = eepreg_check();
    if (ret < 0) {
        puts("[FAILED]");
        return 1;
    }

    puts("[SUCCESS]");

    printf("Testing writing and reading entries: ");

    printf("add ");
    ret = eepreg_add(&tmp1, ENT1_NAME, ENT1_SIZE);
    if (ret < 0 || tmp1 != _data_start(EEPROM_PTR(&mtd)) - ENT1_SIZE) {
        puts("[FAILED]");
        return 1;
    }

    printf("write ");
    ret = eepreg_write(&tmp2, ENT2_NAME, ENT2_SIZE);
    if (ret < 0 ||
        tmp2 != _data_start(EEPROM_PTR(&mtd)) - ENT1_SIZE - ENT2_SIZE) {
        puts("[FAILED]");
        return 1;
    }

    /* read via add */
    printf("add ");
    ret = eepreg_add(&tmp3, ENT1_NAME, ENT1_SIZE);
    if (ret < 0 || tmp1 != tmp3) {
        puts("[FAILED]");
        return 1;
    }

    printf("read ");
    ret = eepreg_read(&tmp1, ENT2_NAME);
    if (ret < 0 || tmp1 != tmp2) {
        puts("[FAILED]");
        return 1;
    }

    puts("[SUCCESS]");

    printf("Testing detection of conflicting size: ");

    printf("add ");
    ret = eepreg_add(&tmp1, ENT1_NAME, ENT1_SIZE + 1);
    if (ret != -EADDRINUSE) {
        puts("[FAILED]");
        return 1;
    }

    puts("[SUCCESS]");

    printf("Testing calculation of lengths: ");

    printf("len ");
    ret = eepreg_len(&tmp1, ENT1_NAME);
    if (ret < 0 || tmp1 != ENT1_SIZE) {
        puts("[FAILED]");
        return 1;
    }

    printf("len ");
    ret = eepreg_len(&tmp2, ENT2_NAME);
    if (ret < 0 || tmp2 != ENT2_SIZE) {
        puts("[FAILED]");
        return 1;
    }

    puts("[SUCCESS]");

    printf("Testing of successful data move after rm: ");

    printf("rm ");
    eepreg_read(&tmp1, ENT1_NAME);
    eepreg_read(&tmp2, ENT2_NAME);
    MTD_PTR(&mtd)->driver->write(MTD_PTR(&mtd), DATA, tmp2, sizeof(DATA));
    ret = eepreg_rm(ENT1_NAME);
    if (ret < 0) {
        puts("[FAILED]");
        return 1;
    }

    printf("read ");
    ret = eepreg_read(&tmp3, ENT2_NAME);
    if (ret < 0 || tmp3 != (_data_start(EEPROM_PTR(&mtd)) - ENT2_SIZE)) {
        puts("[FAILED]");
        return 1;
    }

    printf("data ");
    MTD_PTR(&mtd)->driver->read(MTD_PTR(&mtd), data, tmp3, sizeof(DATA));
    if (strcmp(data, DATA) != 0) {
        puts("[FAILED]");
        return 1;
    }

    puts("[SUCCESS]");

    printf("Testing of free space change after write: ");

    printf("free ");
    ret = eepreg_free(&tmp1);
    if (ret < 0) {
        puts("[FAILED]");
        return 1;
    }

    printf("add ");
    ret = eepreg_add(&tmp3, ENT1_NAME, ENT1_SIZE);
    if (ret < 0) {
        puts("[FAILED]");
        return 1;
    }

    printf("free ");
    ret = eepreg_free(&tmp2);
    if (ret < 0 ||
        tmp1 !=
        (tmp2 + ENT1_SIZE + sizeof(ENT1_NAME) + _ptr_len(EEPROM_PTR(&mtd)))) {

        puts("[FAILED]");
        return 1;
    }

    puts("[SUCCESS]");

    printf("Testing of iteration over registry: ");

    printf("iter ");
    ret = eepreg_iter(eepreg_iter_cb, NULL);
    if (ret < 0) {
        puts("[FAILED]");
        return 1;
    }

    puts("[SUCCESS]");

    puts("Tests complete!");

    return 0;
}
