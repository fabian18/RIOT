/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * VFS organization:
 * ```
 *  send/
 *  |
 *  +- ta/ # Trust Anchor certificates for <T> Trust Anchors
 *  |   |
 *  |   +- <trust_anchor_1_fqdn>.der
 *  |   |
 *  |     ...
 *  |   |
 *  |   +- <trust_anchor_<T>_fqdn>.der
 *  |
 *  +- cp/ # Certificate Paths for <R> routers and own Certificate Path
 *  |   |
 *  |   +- <router_name_1>/ # router name is not important
 *  |   |   |
 *  |   |   +- <trust_anchor_1_fqdn>/ # for each router, multiple CP for different TAs 1 ... <t> exist
 *  |   |   |   |
 *  |   |   |   +- 1.der # A <c>-component CP consists of certificates named 1.der to <c>.der
 *  |   |   |   |
 *  |   |   |    ...
 *  |   |   |
 *  |   |    ...
 *  |   |   |
 *  |   |   +- <trust_anchor_<t>_fqdn>/
 *  |   |   |   |
 *  |   |   |   +- 1.der
 *  |   |   |   |
 *  |   |   |    ...
 *  |   |
 *  |   +- <router_name_<R>>/
 *  |   |   |
 *  |   |   +- <trust_anchor_1_fqdn>/
 *  |   |   |   |
 *  |   |   |   +- 1.der
 *  |   |   |   |
 *  |   |   |    ...
 *  |   |   |
 *  |   |    ...
 *  |   |
 *  |    ...
 *  |   |
 *  |   +- self/ # own Certificate Path
 *  |   |   |
 *  |   |   +- <trust_anchor_1_fqdn>/
 *  |   |   |   |
 *  |   |   |   +- pubkey.der # own public key
 *  |   |   |   |
 *  |   |   |   +- key.der # own private key
 *  |   |   |   |
 *  |   |   |   +- 1.der
 *  |   |   |   |
 *  |   |   |    ...
 *  |   |   |
 *  |   |    ...
 * ```
 * @}
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/errno.h>

#include "kernel_defines.h"
#include "mutex.h"
#include "send_internal.h"
#include "net/gnrc/send.h"

static uint8_t _pk_buf[CONFIG_GNRC_SEND_PK_BUF_SIZE];
static uint8_t _k_buf[CONFIG_GNRC_SEND_KEY_BUF_SIZE];
static gnrc_send_key_t _key_meta[1];

static uint8_t _ta_buf[CONFIG_GNRC_SEND_TA_BUF_SIZE];
static gnrc_send_ta_t _ta_meta[CONFIG_GNRC_SEND_TA_NUMOF];

static mutex_t _cert_mutex;
static uint8_t _cert_verify_buf[CONFIG_GNRC_SEND_CRT_BUF_SIZE];
static gnrc_send_x509_crt_t _cert_verify_chain[2];

static mutex_t _cp_mutex;
static gnrc_send_cp_t _cp_meta[CONFIG_GNRC_SEND_CP_NUMOF];

gnrc_send_ident_t gnrc_send_self_ident;

void gnrc_send_init_node(void)
{
    gnrc_send_load_ta(CONFIG_GNRC_SEND_TA_ROOT, _ta_meta, _ta_buf, sizeof(_ta_buf), ARRAY_SIZE(_ta_meta));
    gnrc_send_load_cp(CONFIG_GNRC_SEND_CP_ROOT, _cp_meta, ARRAY_SIZE(_cp_meta),
                      _key_meta, _k_buf, sizeof(_k_buf), _pk_buf, sizeof(_pk_buf));
    mutex_init(&_cert_mutex);
    (void)_cert_verify_buf;
    (void)_cert_verify_chain;
}

const struct gnrc_send_ident *gnrc_send_get_self_ident(void)
{
    return &gnrc_send_self_ident;
}

void *gnrc_send_get_pubkey_der(void)
{
    return _key_meta[0].pubkey;
}

size_t gnrc_send_get_pubkey_size_der(void)
{
    return _key_meta[0].pubkey_size;
}

void *gnrc_send_get_privkey_der(void)
{
    return _key_meta[0].key;
}

size_t gnrc_send_get_privkey_size_der(void)
{
    return _key_meta[0].key_size;
}

const gnrc_send_pk_t *gnrc_send_get_pk(void)
{
    return &_key_meta[0].pk;
}

const gnrc_send_ta_t *gnrc_send_get_ta_by_name(const void *name, size_t nsize)
{
    gnrc_send_ta_iter_t iter = gnrc_send_ta_acquire();
    while (gnrc_send_ta_iterator(&iter)) {
        if (nsize == iter.ta->name_size && !memcmp(name, iter.ta->name, nsize)) {
            gnrc_send_ta_release();
            return iter.ta;
        }
    }
    gnrc_send_ta_release();
    return NULL;
}

const gnrc_send_ta_t *gnrc_send_get_ta_by_fqdn(const char *fqdn, size_t len)
{
    gnrc_send_ta_iter_t iter = gnrc_send_ta_acquire();
    while (gnrc_send_ta_iterator(&iter)) {
        const char * extn = strrchr(iter.ta->fqdn, '.');
        size_t n = strlen(iter.ta->fqdn);
        if (extn && !strcmp(extn, ".der")) {
            n -= strlen(extn);
        }
        if (len == n && !strncmp(fqdn, iter.ta->fqdn, n)) {
            gnrc_send_ta_release();
            return iter.ta;
        }
    }
    gnrc_send_ta_release();
    return NULL;
}

const gnrc_send_ta_t *gnrc_send_get_ta_by_ident(const gnrc_send_ident_t *ident)
{
    gnrc_send_ta_iter_t iter = gnrc_send_ta_acquire();
    while (gnrc_send_ta_iterator(&iter)) {
        if (!memcmp(ident->hash, iter.ta->ta_ident.hash, sizeof(ident->hash))) {
            gnrc_send_ta_release();
            return iter.ta;
        }
    }
    gnrc_send_ta_release();
    return NULL;
}

const gnrc_send_ta_t *gnrc_send_ta_iterator(gnrc_send_ta_iter_t *iter)
{
    assert(iter);
    if (!iter->ta) {
        return (iter->ta = &_ta_meta[0]);
    }
    if (iter->ta + 1 >= &_ta_meta[CONFIG_GNRC_SEND_TA_NUMOF]) {
        return (iter->ta = NULL);
    }
    return (iter->ta = iter->ta + 1);
}

const gnrc_send_cp_t *gnrc_send_get_cp_by_ident(const gnrc_send_ident_t *ident,
                                                const gnrc_send_ta_t *ta)
{
    gnrc_send_cp_iter_t iter = gnrc_send_cp_acquire();
    while (gnrc_send_cp_iterator(&iter)) {
        if (!memcmp(ident->hash, iter.cp->cp_ident.hash, sizeof(ident->hash)) &&
            (!ta || iter.cp->ta == ta)) {
            gnrc_send_cp_release();
            return iter.cp;
        }
    }
    gnrc_send_cp_release();
    return NULL;
}

const gnrc_send_cp_t *gnrc_send_cp_iterator(gnrc_send_cp_iter_t *iter)
{
    assert(iter);
    if (!iter->cp) {
        return (iter->cp = &_cp_meta[0]);
    }
    if (iter->cp + 1 >= &_cp_meta[CONFIG_GNRC_SEND_CP_NUMOF]) {
        return (iter->cp = NULL);
    }
    return (iter->cp = iter->cp + 1);
}

gnrc_send_x509_crt_t *gnrc_send_cert_acquire(size_t *size, void **crt_buf)
{
    mutex_lock(&_cert_mutex);
    *crt_buf = _cert_verify_buf;
    *size = sizeof(_cert_verify_buf);
    for (unsigned i = 0; i < ARRAY_SIZE(_cert_verify_chain); i++) {
        mbedtls_x509_crt_init(&_cert_verify_chain[i].cert);
    }
    return _cert_verify_chain;
}

void gnrc_send_cert_release(void)
{
    for (unsigned i = 0; i < ARRAY_SIZE(_cert_verify_chain); i++) {
        mbedtls_x509_crt_free(&_cert_verify_chain[i].cert);
    }
    mutex_unlock(&_cert_mutex);
}

gnrc_send_ta_iter_t gnrc_send_ta_acquire(void)
{
    return (gnrc_send_ta_iter_t){ .ta = NULL };
}

void gnrc_send_ta_release(void)
{
}

gnrc_send_cp_iter_t gnrc_send_cp_acquire(void)
{
    mutex_lock(&_cp_mutex);
    return (gnrc_send_cp_iter_t){ .cp = NULL };
}

void gnrc_send_cp_release(void)
{
    mutex_unlock(&_cp_mutex);
}

int gnrc_send_cp_add(const gnrc_send_cp_t *cp)
{
    assert(cp->ta);
    for (unsigned i = 0; i < ARRAY_SIZE(_cp_meta); i++) {
        if (!_cp_meta[i].ta) {
            _cp_meta[i] = *cp;
            return 0;
        }
    }
    return -ENOBUFS;
}
