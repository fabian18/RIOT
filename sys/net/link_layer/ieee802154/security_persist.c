/*
 * Copyright (C) 2021 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author Fabian Hüßler <fabian.huessler@ovgu.de>
 * @}
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#include "kernel_defines.h"
#include "net/ieee802154_security.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#if IS_USED(MODULE_IEEE802154_SECURITY_PERSIST_BACKUP_RAM)
#ifndef CONFIG_IEEE802154_SEC_DEFAULT_DEV_NUMOF
/**
 * @brief   Maximum number of IEEE 802.15.4 interfaces
 */
#define CONFIG_IEEE802154_SEC_DEFAULT_DEV_NUMOF 1
#endif

typedef struct {
    ieee802154_sec_persist_t persist;
    atomic_bool commit;
} persist_data_backup_ram_t;

static persist_data_backup_ram_t
    _backup_ram[CONFIG_IEEE802154_SEC_DEFAULT_DEV_NUMOF] BACKUP_RAM;

int ieee802154_sec_persist_read(const ieee802154_sec_context_t *ctx,
                                void *dst,
                                size_t offset,
                                size_t size)
{
    assert(offset < sizeof(ieee802154_sec_persist_t));
    assert(size <= sizeof(ieee802154_sec_persist_t));
    assert(offset + size <= sizeof(ieee802154_sec_persist_t));
    uint8_t *src = (uint8_t *)&((persist_data_backup_ram_t *)(ctx->persist))->persist;
    memcpy(dst, src + offset, size);
    return 0;
}

int ieee802154_sec_persist_write(ieee802154_sec_context_t *ctx,
                                 const void *src,
                                 size_t offset,
                                 size_t size)
{
    assert(offset < sizeof(ieee802154_sec_persist_t));
    assert(size <= sizeof(ieee802154_sec_persist_t));
    assert(offset + size <= sizeof(ieee802154_sec_persist_t));
    uint8_t *dst = (uint8_t *)&((persist_data_backup_ram_t *)(ctx->persist))->persist;
    atomic_store(&(((persist_data_backup_ram_t *)ctx->persist)->commit), false);
    memcpy(dst + offset, src, size);
    atomic_store(&(((persist_data_backup_ram_t *)ctx->persist)->commit), true);
    return 0;
}

int ieee802154_sec_persist_init(ieee802154_sec_context_t *ctx)
{
    static atomic_uint _persist_counter;
    int ret = 0;
    if (!ctx->persist) {
        /* ieee802154_sec_init() is called in driver threads */
        unsigned index = _persist_counter;
        while (!atomic_compare_exchange_weak(&_persist_counter, &index, index + 1)) {}
        if (index >= CONFIG_IEEE802154_SEC_DEFAULT_DEV_NUMOF) {
            DEBUG("[ieee802154_security] "
                  "You need to increase CONFIG_IEEE802154_SEC_DEFAULT_DEV_NUMOF to at least %u\n",
                  index + 1);
            ret = -ENOBUFS;
        }
        else if (!_backup_ram[index].commit) {
            DEBUG("[ieee802154_security] "
                  "Persistent data of context %p is empty or has been corrupted "
                  "and must be (re)initialized\n",
                  (void *)ctx);
            _backup_ram[index].persist.fc = ctx->frame_counter;
            atomic_store(&_backup_ram[index].commit, true);
            ret = ENODATA;
        }
        ctx->persist = &_backup_ram[index].persist;
    }
    return ret;
}
#endif
