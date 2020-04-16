/*
 * Copyright (C) 2018 Acutam Automation, LLC
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   sys_eepreg
 * @{
 *
 * @file
 * @brief   eepreg implementation
 *
 * @author  Matthew Blue <matthew.blue.neuro@gmail.com>
 * @}
 */

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "eepreg.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/* EEPREG magic number */
static const char eepreg_magic[] = "RIOTREG";

typedef struct {
    mtd_eeprom_t *eeprom;   /**< MTD EEPROM pointer */
    uint32_t size;          /**< EEPROM size */
    uint8_t ptr_len;        /**< length of pointer to address EEPROM */
} eepreg_t;

static eepreg_t _reg;       /**< eepreg singleton */

#define EEPREG_EEPROM_DEV   (_reg.eeprom->base)
#define EEPREG_EEPROM_SIZE  (_reg.size)
#define EEPREG_PTR_LEN      (_reg.ptr_len)

/* constant lengths */
#define MAGIC_SIZE          (sizeof(eepreg_magic) - 1)  /* -1 to remove null */
#define ENT_LEN_SIZ         (1U)

/* constant locations */
#define REG_START           (EEPROM_RESERV_CPU_LOW + EEPROM_RESERV_BOARD_LOW)
#define REG_MAGIC_LOC       (REG_START)
#define REG_END_PTR_LOC     (REG_MAGIC_LOC + MAGIC_SIZE)
#define REG_ENT1_LOC        (REG_END_PTR_LOC + EEPREG_PTR_LEN)
#define DAT_START           (EEPREG_EEPROM_SIZE - EEPROM_RESERV_CPU_HI \
                            - EEPROM_RESERV_BOARD_HI - 1)

#if EEPROM_CHECK_ERRORS
#define CHECK(exp, res, todo)       \
do {                                \
    int check = (exp);              \
    if (check != (res)) {           \
        todo;                       \
    }                               \
} while(0)
#else
#define CHECK(exp, res, todo) (exp)
#endif

static inline int _eeprom_read(void* dst, uint32_t loc, uint32_t len)
{
    return EEPREG_EEPROM_DEV.driver->read(&EEPREG_EEPROM_DEV, dst, loc, len);
}

static inline int _eeprom_write(const void* src, uint32_t loc, uint32_t len)
{
    return EEPREG_EEPROM_DEV.driver->write(&EEPREG_EEPROM_DEV, src, loc, len);
}

static inline int _read_meta_uint(uint32_t loc, uint32_t *val)
{
    uint8_t data[4];
    uint32_t ret;

    CHECK(_eeprom_read(data, loc, EEPREG_PTR_LEN), EEPREG_PTR_LEN,
        return check);

    /* unused array members will be discarded */
    ret = ((uint32_t)data[0] << 24)
          | ((uint32_t)data[1] << 16)
          | ((uint32_t)data[2] << 8)
          | ((uint32_t)data[3]);

    /* bit shift to discard unused array members */
    ret >>= 8 * (4 - EEPREG_PTR_LEN);
    *val = ret;

    return 0;
}

static inline int _write_meta_uint(uint32_t loc, uint32_t val)
{
    uint8_t data[4];

    val <<= 8 * (4 - EEPREG_PTR_LEN);

    data[0] = (uint8_t)(val >> 24);
    data[1] = (uint8_t)(val >> 16);
    data[2] = (uint8_t)(val >> 8);
    data[3] = (uint8_t)val;

    CHECK(_eeprom_write(data, loc, EEPREG_PTR_LEN), EEPREG_PTR_LEN,
        return check);

    return 0;
}

static inline int _get_reg_end(uint32_t *reg_end)
{
    return _read_meta_uint(REG_END_PTR_LOC, reg_end);
}

static inline int _set_reg_end(uint32_t reg_end)
{
    return _write_meta_uint(REG_END_PTR_LOC, reg_end);
}

static inline int _get_last_loc(uint32_t reg_end, uint32_t *last_loc)
{
    if (reg_end == REG_ENT1_LOC) {
        /* no entries yet */
        *last_loc = DAT_START;
        return 0;
    }

    return _read_meta_uint(reg_end - EEPREG_PTR_LEN, last_loc);
}

static inline uint32_t _calc_free_space(uint32_t reg_end, uint32_t last_loc)
{
    return last_loc - reg_end;
}

static inline int _get_meta_len(uint32_t meta_loc, uint8_t *meta_len)
{
    CHECK(_eeprom_read(meta_len, meta_loc, 1), 1, return check);
    return 0;
}

static inline int _set_meta_len(uint32_t meta_loc, uint8_t meta_len)
{
    CHECK(_eeprom_write(&meta_len, meta_loc, 1), 1, return check);
    return 0;
}

static inline int _get_data_loc(uint32_t meta_loc, uint8_t meta_len,
                                uint32_t *data_loc)
{
    /* data location is at the end of meta-data */
    return _read_meta_uint(meta_loc + meta_len - EEPREG_PTR_LEN, data_loc);
}

static inline int _set_data_loc(uint32_t meta_loc, uint8_t meta_len,
                                 uint32_t data_loc)
{
    /* data location is at the end of meta-data */
    return _write_meta_uint(meta_loc + meta_len - EEPREG_PTR_LEN, data_loc);
}

static inline uint8_t _calc_name_len(uint8_t meta_len)
{
    /* entry contents: meta-data length, name, data pointer */
    return meta_len - ENT_LEN_SIZ - EEPREG_PTR_LEN;
}

static inline int _get_name(uint32_t meta_loc, char *name, uint8_t meta_len)
{
    /* name is after entry length */
    uint8_t name_len = _calc_name_len(meta_len);
    CHECK(_eeprom_read(name, meta_loc + ENT_LEN_SIZ, name_len), name_len,
        return check);
    return name_len;
}

static inline int _cmp_name(uint32_t meta_loc, const char *name,
                            uint8_t meta_len)
{
    /* name is after entry length */
    uint32_t loc = meta_loc + ENT_LEN_SIZ;

    uint8_t len = _calc_name_len(meta_len);

    uint8_t offset;
    uint8_t c = 0;
    for (offset = 0; offset < len; offset++) {
        if (name[offset] == '\0') {
            /* entry name is longer than name */
            return (int)offset + 1;
        }
        CHECK(_eeprom_read(&c, loc + offset, 1), 1, return check);
        if (c != (uint8_t)name[offset]) {
            /* non-matching character */
            return (int)offset + 1;
        }
    }

    if (name[offset] == '\0') {
        /* entry name is the same length as name */
        return 0;
    }

    /* entry name is shorter than name */
    return (int)offset + 1;
}

static inline int _get_meta_loc(const char *name, uint32_t *meta_loc)
{
    uint32_t loc = REG_ENT1_LOC;
    uint32_t reg_end;
    CHECK(_get_reg_end(&reg_end), 0, return check);

    while (loc < reg_end) {
        uint8_t meta_len;
        CHECK(_get_meta_len(loc, &meta_len), 0, return check);

        if (_cmp_name(loc, name, meta_len) == 0) {
            *meta_loc = loc;
            return 0;
        }

        loc += meta_len;
    }

    /* no meta-data found */
    *meta_loc = (uint32_t)UINT_MAX;
    return 0;
}

static inline int _get_data_len(uint32_t meta_loc, uint32_t data_loc,
                                uint32_t *data_len)
{
    uint32_t prev_loc;
    if (meta_loc == REG_ENT1_LOC) {
        prev_loc = DAT_START;
    }
    else {
        /* previous entry data pointer is just before this entry */
        CHECK(_read_meta_uint(meta_loc - EEPREG_PTR_LEN, &prev_loc), 0,
            return check);
    }

    *data_len = prev_loc - data_loc;
    return 0;
}

static inline int _new_entry(const char *name, uint32_t data_len)
{
    uint32_t reg_end;
    uint32_t last_loc;
    CHECK(_get_reg_end(&reg_end), 0, return check);
    CHECK(_get_last_loc(reg_end, &last_loc), 0, return check);
    uint32_t free_space = _calc_free_space(reg_end, last_loc);

    uint8_t name_len = (uint8_t)strlen(name);
    uint8_t meta_len = ENT_LEN_SIZ + name_len + EEPREG_PTR_LEN;

    /* check to see if there is enough room */
    if (free_space < meta_len + data_len) {
        return -ENOSPC;
    }

    /* set the length of the meta-data */
    CHECK(_set_meta_len(reg_end, meta_len), 0, return check);

    /* write name of entry */
    CHECK(_eeprom_write(name, reg_end + ENT_LEN_SIZ, name_len), name_len,
        return check);

    /* set the location of the data */
    CHECK(_set_data_loc(reg_end, meta_len, last_loc - data_len), 0,
        return check);

    /* update end of the registry */
    CHECK(_set_reg_end(reg_end + meta_len), 0,
        return check);

    return 0;
}

static inline int _move_data(uint32_t oldpos, uint32_t newpos, uint32_t len)
{
    for (uint32_t count = 0; count < len; count++) {
        uint32_t offset;

        if (newpos < oldpos) {
            /* move from beginning of data */
            offset = count;
        }
        else {
            /* move from end of data */
            offset = len - count;
        }

        uint8_t byte;
        CHECK(_eeprom_read(&byte, oldpos + offset, 1), 1, return check);
        CHECK(_eeprom_write(&byte, newpos + offset, 1), 1, return check);
    }
    return 0;
}

int eepreg_init(mtd_eeprom_t *mtd)
{
    int init = mtd_init((mtd_dev_t *)mtd);
    if (init != 0) {
        return init;
    }
    _reg.eeprom = mtd;
    _reg.size = mtd->base.page_size *
                mtd->base.pages_per_sector *
                mtd->base.sector_count;
    if (_reg.size > 0x1000000) {
        _reg.ptr_len = 4U;
    }
    else if (_reg.size > 0x10000) {
        _reg.ptr_len = 3U;
    }
    else if (_reg.size > 0x100) {
        _reg.ptr_len = 2U;
    }
    else {
        _reg.ptr_len = 1U;
    }
    return 0;
}

int eepreg_add(uint32_t *pos, const char *name, uint32_t len)
{
    int ret = eepreg_check();
    if (ret == -ENOENT) {
        /* reg does not exist, so make a new one */
        CHECK(eepreg_reset(), 0, return -EIO);
    }
    else if (ret < 0) {
        DEBUG("[eepreg_add] eepreg_check failed\n");
        return ret;
    }

    uint32_t reg_end;
    CHECK(_get_reg_end(&reg_end), 0, return -EIO);

    uint32_t meta_loc;
    CHECK(_get_meta_loc(name, &meta_loc), 0, return -EIO);

    if (meta_loc == (uint32_t)UINT_MAX) {
        /* entry does not exist, so make a new one */

        /* location of the new data */
        CHECK(_get_last_loc(reg_end, pos), 0, return -EIO);
        *pos -= len;

        ret = _new_entry(name, len);
        if (ret != 0) {
            if (ret == -ENOSPC) {
                DEBUG("[eepreg_add] not enough space for %s\n", name);
            }
            return ret;
        }

        return 0;
    }
    uint8_t meta_len;
    uint32_t data_len;
    CHECK(_get_meta_len(meta_loc, &meta_len), 0, return -EIO);
    CHECK(_get_data_loc(meta_loc, meta_len, pos), 0, return -EIO);
    CHECK(_get_data_len(meta_loc, *pos, &data_len), 0, return -EIO);
    if (len != data_len) {
        DEBUG("[eepreg_add] %s already exists with different length\n", name);
        return -EADDRINUSE;
    }

    return 0;
}

int eepreg_read(uint32_t *pos, const char *name)
{
    int ret = eepreg_check();
    if (ret < 0) {
        DEBUG("[eepreg_read] eepreg_check failed\n");
        return ret;
    }

    uint32_t meta_loc;
    CHECK(_get_meta_loc(name, &meta_loc), 0, return -EIO);

    if (meta_loc == (uint32_t)UINT_MAX) {
        DEBUG("[eepreg_read] no entry for %s\n", name);
        return -ENOENT;
    }

    uint8_t meta_len;
    CHECK(_get_meta_len(meta_loc, &meta_len), 0, return -EIO);
    CHECK(_get_data_loc(meta_loc, meta_len, pos), 0, return -EIO);

    return 0;
}

int eepreg_write(uint32_t *pos, const char *name, uint32_t len)
{
    uint32_t reg_end;
    CHECK(_get_reg_end(&reg_end), 0, return -EIO);

    int ret = eepreg_check();
    if (ret == -ENOENT) {
        /* reg does not exist, so make a new one */
        CHECK(eepreg_reset(), 0, return -EIO);
    }
    else if (ret < 0) {
        DEBUG("[eepreg_add] eepreg_check failed\n");
        return ret;
    }

    /* location of the new data */
    CHECK(_get_last_loc(reg_end, pos), 0, return -EIO);
    *pos -= len;

    ret = _new_entry(name, len);
    if (ret != 0) {
        if (ret == -ENOSPC) {
            DEBUG("[eepreg_add] not enough space for %s\n", name);
        }
        return ret;
    }

    return 0;
}

int eepreg_rm(const char *name)
{
    int ret = eepreg_check();
    if (ret < 0) {
        DEBUG("[eepreg_rm] eepreg_check failed\n");
        return ret;
    }

    uint32_t meta_loc;
    CHECK(_get_meta_loc(name, &meta_loc), 0, return -EIO);

    if (meta_loc == (uint32_t)UINT_MAX) {
        DEBUG("[eepreg_rm] no entry for %s\n", name);
        return -ENOENT;
    }

    uint32_t reg_end;
    uint32_t last_loc;
    uint8_t meta_len;
    CHECK(_get_reg_end(&reg_end), 0, return -EIO);
    CHECK(_get_last_loc(reg_end, &last_loc), 0, return -EIO);
    CHECK(_get_meta_len(meta_loc, &meta_len), 0, return -EIO);
    uint32_t tot_meta_len = reg_end - meta_loc;

    uint32_t data_loc;
    uint32_t data_len;
    CHECK(_get_data_loc(meta_loc, meta_len, &data_loc), 0, return -EIO);
    CHECK(_get_data_len(meta_loc, data_loc, &data_len), 0, return -EIO);

    /* data_loc is above last_loc due to descending order */
    uint32_t tot_data_len = data_loc - last_loc;

    CHECK(_move_data(meta_loc + meta_len, meta_loc, tot_meta_len), 0,
        return -EIO);

    CHECK(_move_data(last_loc, last_loc + data_len, tot_data_len), 0,
        return -EIO);

    reg_end -= meta_len;
    CHECK(_set_reg_end(reg_end), 0, return -EIO);

    /* update data locations */
    while (meta_loc < reg_end) {
        CHECK(_get_meta_len(meta_loc, &meta_len), 0, return -EIO);
        CHECK(_get_data_loc(meta_loc, meta_len, &data_loc), 0, return -EIO);

        /* addition due to descending order */
        CHECK(_set_data_loc(meta_loc, meta_len, data_loc + data_len), 0,
            return -EIO);

        meta_loc += meta_len;
    }

    return 0;
}

int eepreg_iter(eepreg_iter_cb_t cb, void *arg)
{
    int ret = eepreg_check();
    if (ret < 0) {
        DEBUG("[eepreg_len] eepreg_check failed\n");
        return ret;
    }

    uint32_t reg_end;
    CHECK(_get_reg_end(&reg_end), 0, return -EIO);

    uint32_t meta_loc = REG_ENT1_LOC;
    uint8_t meta_len;
    while (meta_loc < reg_end) {
        CHECK(_get_meta_len(meta_loc, &meta_len), 0, return -EIO);

        /* size of memory allocation */
        uint8_t name_len = _calc_name_len(meta_len);

        char name[name_len + 1];

        /* terminate string */
        name[name_len] = '\0';

        CHECK(_get_name(meta_loc, name, meta_len), name_len, return -EIO);

        /* execute callback */
        ret = cb(name, arg);

        if (ret < 0) {
            DEBUG("[eepreg_iter] callback reports failure\n");
            return ret;
        }

        /* only advance if cb didn't delete entry */
        if (_cmp_name(meta_loc, name, meta_len) == 0) {
            meta_loc += meta_len;
        }
    }

    return 0;
}

int eepreg_check(void)
{
    char magic[MAGIC_SIZE];

    /* get magic number from EEPROM */
    if (_eeprom_read(magic, REG_MAGIC_LOC, MAGIC_SIZE) != MAGIC_SIZE) {
        DEBUG("[eepreg_check] EEPROM read error\n");
        return -EIO;
    }

    /* check to see if magic number is the same */
    if (strncmp(magic, eepreg_magic, MAGIC_SIZE) != 0) {
        DEBUG("[eepreg_check] No registry detected\n");
        return -ENOENT;
    }

    return 0;
}

int eepreg_reset(void)
{
    /* write new registry magic number */
    if (_eeprom_write(eepreg_magic, REG_MAGIC_LOC, MAGIC_SIZE) != MAGIC_SIZE) {
        DEBUG("[eepreg_reset] EEPROM write error\n");
        return -EIO;
    }

    /* new registry has no entries */
    CHECK(_set_reg_end(REG_ENT1_LOC), 0, return -EIO);

    return 0;
}

int eepreg_len(uint32_t *len, const char *name)
{
    int ret = eepreg_check();
    if (ret < 0) {
        DEBUG("[eepreg_len] eepreg_check failed\n");
        return ret;
    }

    uint32_t meta_loc;
    CHECK(_get_meta_loc(name, &meta_loc), 0, return -EIO);

    if (meta_loc == (uint32_t)UINT_MAX) {
        DEBUG("[eepreg_len] no entry for %s\n", name);
        return -ENOENT;
    }

    uint8_t meta_len;
    uint32_t data_loc;
    CHECK(_get_meta_len(meta_loc, &meta_len), 0, return -EIO);
    CHECK(_get_data_loc(meta_loc, meta_len, &data_loc), 0, return -EIO);
    CHECK(_get_data_len(meta_loc, data_loc, len), 0, return -EIO);

    return 0;
}

int eepreg_free(uint32_t *len)
{
    int ret = eepreg_check();
    if (ret < 0) {
        DEBUG("[eepreg_free] eepreg_check failed\n");
        return ret;
    }

    uint32_t reg_end;
    uint32_t last_loc;
    CHECK(_get_reg_end(&reg_end), 0, return -EIO);
    CHECK(_get_last_loc(reg_end, &last_loc), 0, return -EIO);
    *len = _calc_free_space(reg_end, last_loc);

    return 0;
}
