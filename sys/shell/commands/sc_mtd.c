/*
 * Copyright (C) 2021 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_shell_commands
 * @{
 *
 * @file
 * @brief       Command to edit memory of MTDs
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "board.h"
#include "shell_commands.h"
#include "mtd.h"
#include "fmt.h"
#if IS_USED(MODULE_OD)
#include "od.h"
#endif

#ifndef SC_MTD_MAX_SIZE
#define SC_MTD_MAX_SIZE 16
#endif

#define MTD_SIZE(mtd)   ((mtd)->sector_count *      \
                         (mtd)->pages_per_sector *  \
                         (mtd)->page_size))

#define CSTRLEN(cs)     ((sizeof((const char[]){cs})) - 1)
#define CSTRCMP(p, cs)  strncmp(p, cs, CSTRLEN(cs))

#define PROG_EXIT_FAILURE(status, ...) {            \
    status = EXIT_FAILURE;                          \
    __VA_ARGS__;                                    \
    goto EXIT;                                      \
}

#define PROG_EXIT_SUCCESS(status, ...) {            \
    status = EXIT_SUCCESS;                          \
    __VA_ARGS__;                                    \
    goto EXIT;                                      \
}

static mtd_dev_t *_get_mtd_dev(unsigned idx)
{
    switch (idx) {
#ifdef MTD_0
    case 0: return MTD_0;
#endif
#ifdef MTD_1
    case 1: return MTD_1;
#endif
#ifdef MTD_2
    case 2: return MTD_2;
#endif
#ifdef MTD_3
    case 3: return MTD_3;
#endif
    }

    return NULL;
}

static void *_malloc(size_t size)
{
#if SC_MTD_MAX_SIZE > 0
    static uint8_t _buf[SC_MTD_MAX_SIZE];
    return size <= sizeof(_buf) ? _buf : NULL;
#else
    return malloc(size);
#endif
}

static void _free(void *buf)
{
#if SC_MTD_MAX_SIZE > 0
    (void)buf;
#else
    free(buf);
#endif
}

mtd_dev_t *_parse_dev(const char *sdev)
{
    errno = 0;
    long l = strtol(sdev, NULL, 0);
    if (errno || l < 0) {
        return NULL;
    }
    return _get_mtd_dev(l);
}

long _parse_address(const char *saddr)
{
    errno = 0;
    long addr = strtol(saddr, NULL, 0);
    if (errno || addr < 0) {
        return -ERANGE;
    }
    return addr;
}

long _parse_size(const char *ssize)
{
    errno = 0;
    long size = strtol(ssize, NULL, 0);
    if (errno || size < 0) {
        return -ERANGE;
    }
    return size;
}

long _parse_data(const char *sdata, void **data)
{
    size_t data_len = strlen(sdata);
    if (!CSTRCMP(sdata, "0x") || !CSTRCMP(sdata, "0X")) {
        if ((data_len - 2) & 1) {
            return -EINVAL;
        }
        uint8_t *d = (uint8_t *)sdata;
        const char *s = sdata + 2;
        data_len = fmt_hex_bytes(d, s);
    }
    *data = (void *)sdata;
    return data_len;
}

static long _read(int argc, char **argv, void **data)
{
    assert(argc >= 3);
    mtd_dev_t *mtd = _parse_dev(argv[0]);
    if (!mtd) {
        return -ENODEV;
    }
    long addr = _parse_address(argv[1]);
    if (addr < 0) {
        return -ERANGE;
    }
    long size = _parse_size((argv[2]));
    if (size < 0) {
        return -ERANGE;
    }
    if (!(*data = _malloc(size))) {
        return -ENOMEM;
    }
    int rd = mtd_read(mtd, *data, addr, size);
    if (rd < 0) {
        _free(*data);
    }
    return size;
}

static long _write(int argc, char **argv, void **data)
{
    assert(argc >= 3);
    mtd_dev_t *mtd = _parse_dev(argv[0]);
    if (!mtd) {
        return -ENODEV;
    }
    long addr = _parse_address(argv[1]);
    if (addr < 0) {
        return -ERANGE;
    }
    long size = _parse_data(argv[2], data);
    if (size < 0) {
        return -EINVAL;
    }
    int wr = mtd_write(mtd, *data, addr, size);
    if (wr < 0) {
        return wr;
    }
    return size;
}

static void _print_help(void)
{
    puts("usage:\n"
         "mtd [read | write]\n"
         "  <read <mtd_dev> <addr> <size>>\n"
         "  <write <mtd_dev> <addr> <data>>\n");
}

int _mtd(int argc, char **argv)
{
    long status = EXIT_FAILURE;

    if (argc < 5) {
        PROG_EXIT_FAILURE(status, _print_help());
    }

    void *data = NULL;
    if (!CSTRCMP(argv[1], "read")) {
        if ((status = _read(argc - 2, argv + 2, &data)) < 0) {
            PROG_EXIT_FAILURE(status, printf("read %ld\n", status));
        }
#if IS_USED(MODULE_OD)
        od_hex_dump(data, status, 8);
#endif
        _free(data);
    }
    else if (!CSTRCMP(argv[1], "write")) {
        if ((status = _write(argc - 2, argv + 2, &data)) < 0) {
            PROG_EXIT_FAILURE(status, printf("write %ld\n", status));
        }
    }
    else {
        PROG_EXIT_FAILURE(status, _print_help());
    }

    PROG_EXIT_SUCCESS(status);
EXIT:
    return status;
}
