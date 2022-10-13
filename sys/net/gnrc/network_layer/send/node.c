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
 * @brief   Organization of a SEND node, provisioned with Trust Anchors,
 *          Certificate Paths, and public and private keys
 *
 * VFS organization:
 * ```
 *  send/
 *  |
 *  +- cp/ # Certificate Paths of routers and own Certificate Path
 *  |   |
 *  |   +- _self.der # A filename with a starting underscore denotes own certificates.
 *  |   |
 *  |   ...
 *  |
 *  +- crt/ # All intermediate certificates
 *  |   |
 *  |   ...
 *  |
 *  +- key/ # keys to use for signature and CGA generation
 *  |   |
 +  |   +- key.der # private key in DER for signature creation
 +  |   |
 *  |   +- pubker.der # public key in DER for CGA generation
 *  |
 *  +- ta/ # Trust Anchor certificates
 *  |   |
 *  |   ...
 * ```
 *
 * @author  Fabian Hüßler <fabian.huessler@ovgu.de>
 * @}
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "kernel_defines.h"
#include "mutex.h"
#include "send_internal.h"
#include "net/gnrc/send.h"

#ifdef TEST_SUITES
/* In unittests node variables are initialized by hand and not loaded from VFS */
#define _STATIC
#else
#define _STATIC             static
#endif

_STATIC uint8_t _k_buf[GNRC_SEND_KEY_BUF_SIZE];
_STATIC gnrc_send_key_t _key_meta[1];

static unsigned _crt_numof;
_STATIC gnrc_send_crt_t _crt_meta[GNRC_SEND_CRT_NUMOF];

static unsigned _ta_numof;
_STATIC gnrc_send_ta_t _ta_meta[GNRC_SEND_TA_NUMOF];

static mutex_t _cp_mutex = MUTEX_INIT;
static unsigned _cp_numof;
_STATIC gnrc_send_cp_t _cp_meta[GNRC_SEND_CP_NUMOF];

static mutex_t _cert_mutex = MUTEX_INIT;
static uint8_t _cert_verify_buf[GNRC_SEND_CRT_BUF_SIZE];
static gnrc_send_x509_crt_t _cert_verify_chain[2];

_STATIC gnrc_send_ident_t _gnrc_send_self_ident;

void gnrc_send_init_node(void)
{
    if (IS_USED(MODULE_GNRC_SEND_CLEAR_CP)) {
        gnrc_send_clear_cp_vfs();
    }
    int ret;
    ret = gnrc_send_load_ta(_crt_meta, ARRAY_SIZE(_crt_meta),
                            _ta_meta, ARRAY_SIZE(_ta_meta));
    if (ret > 0) {
        _ta_numof = ret;
        _crt_numof += _ta_numof;
    }
    ret = gnrc_send_load_crt(_crt_meta + _crt_numof, ARRAY_SIZE(_crt_meta) - _crt_numof);
    if (ret > 0) {
        _crt_numof += ret;
    }
    ret = gnrc_send_load_cp(_crt_meta + _crt_numof, ARRAY_SIZE(_crt_meta) - _crt_numof,
                            _cp_meta, ARRAY_SIZE(_cp_meta));
    if (ret > 0) {
        _cp_numof = ret;
        _crt_numof += _cp_numof;
    }
    gnrc_send_load_keys(_key_meta, _k_buf, sizeof(_k_buf));
}

const struct gnrc_send_ident *gnrc_send_get_self_ident(void)
{
    return &_gnrc_send_self_ident;
}

const void *gnrc_send_get_pubkey_der(void)
{
    return _key_meta[0].pubkey;
}

size_t gnrc_send_get_pubkey_size_der(void)
{
    return _key_meta[0].pubkey_size;
}

const void *gnrc_send_get_privkey_der(void)
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

const gnrc_send_ta_t *gnrc_send_ta_iterator(gnrc_send_ta_iter_t *iter)
{
    assert(iter);
    if (!iter->ta) {
        iter->ta = &_ta_meta[0];
    }
    else if (iter->ta + 1 >= &_ta_meta[GNRC_SEND_TA_NUMOF]) {
        return (iter->ta = NULL);
    }
    else {
        iter->ta = iter->ta + 1;
    }
    return (iter->ta = ((iter->ta->name_size &&
                         iter->ta->pk_size &&
                         iter->ta->crt &&
                         (iter->ta->crt->rel_path[0] || iter->ta->crt->crt_inmem.size))
                        ? iter->ta : NULL));
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
    (void)fqdn; (void)len;
    return NULL;
}

const gnrc_send_ta_t *gnrc_send_get_ta_by_subject_ident(const gnrc_send_ident_t *ident)
{
    gnrc_send_ta_iter_t iter = gnrc_send_ta_acquire();
    while (gnrc_send_ta_iterator(&iter)) {
        if (!memcmp(ident->hash, iter.ta->crt->subject_ident.hash, sizeof(ident->hash))) {
            gnrc_send_ta_release();
            return iter.ta;
        }
    }
    gnrc_send_ta_release();
    return NULL;
}

const gnrc_send_crt_t *gnrc_send_crt_iterator(gnrc_send_crt_iter_t *iter)
{
    assert(iter);
    if (!iter->crt) {
        iter->crt = &_crt_meta[0];
    }
    else if (iter->crt + 1 >= &_crt_meta[GNRC_SEND_CRT_NUMOF]) {
        return (iter->crt = NULL);
    }
    else {
        iter->crt = iter->crt + 1;
    }
    return (iter->crt = ((iter->crt->rel_path[0] || iter->crt->crt_inmem.size)
                         ? iter->crt : NULL));
}

gnrc_send_crt_t *gnrc_send_get_crt_by_subject_ident(const gnrc_send_ident_t *ident)
{
    gnrc_send_crt_iter_t iter = gnrc_send_crt_acquire();
    while (gnrc_send_crt_iterator(&iter)) {
        if (!memcmp(ident->hash, iter.crt->subject_ident.hash, sizeof(ident->hash))) {
            gnrc_send_crt_release();
            return (gnrc_send_crt_t *)iter.crt;
        }
    }
    gnrc_send_crt_release();
    return NULL;
}

gnrc_send_crt_t *gnrc_send_get_crt_by_pk_ident(const gnrc_send_ident_t *ident)
{
    gnrc_send_crt_iter_t iter = gnrc_send_crt_acquire();
    while (gnrc_send_crt_iterator(&iter)) {
        if (!memcmp(ident->hash, iter.crt->pk_ident.hash, sizeof(ident->hash))) {
            gnrc_send_crt_release();
            return (gnrc_send_crt_t *)iter.crt;
        }
    }
    gnrc_send_crt_release();
    return NULL;
}

const gnrc_send_cp_t *gnrc_send_cp_iterator(gnrc_send_cp_iter_t *iter)
{
    assert(iter);
    if (!iter->cp) {
        iter->cp = &_cp_meta[0];
    }
    else if (iter->cp + 1 >= &_cp_meta[GNRC_SEND_CP_NUMOF]) {
        return (iter->cp = NULL);
    }
    else {
        iter->cp = iter->cp + 1;
    }
    return (iter->cp = ((iter->cp->num_comp &&
                         iter->cp->ta &&
                         (iter->cp->crt->rel_path[0] || iter->cp->crt->crt_inmem.size))
                        ? iter->cp : NULL));
}

const gnrc_send_cp_t *gnrc_send_get_cp_by_pk_ident(const gnrc_send_ident_t *ident,
                                                   const gnrc_send_ta_t *ta)
{
    gnrc_send_cp_iter_t iter = gnrc_send_cp_acquire();
    while (gnrc_send_cp_iterator(&iter)) {
        if (!memcmp(ident->hash, iter.cp->crt->pk_ident.hash, sizeof(ident->hash)) &&
            (!ta || iter.cp->ta == ta)) {
            gnrc_send_cp_release();
            return iter.cp;
        }
    }
    gnrc_send_cp_release();
    return NULL;
}

gnrc_send_x509_crt_t *gnrc_send_x509_acquire(size_t *size, void **crt_buf)
{
    mutex_lock(&_cert_mutex);
    *crt_buf = _cert_verify_buf;
    *size = sizeof(_cert_verify_buf);
    for (unsigned i = 0; i < ARRAY_SIZE(_cert_verify_chain); i++) {
        mbedtls_x509_crt_init(&_cert_verify_chain[i].cert);
    }
    return _cert_verify_chain;
}

void gnrc_send_x509_release(void)
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

gnrc_send_crt_iter_t gnrc_send_crt_acquire(void)
{
    mutex_lock(&_cp_mutex);
    return (gnrc_send_crt_iter_t){ .crt = NULL };
}

void gnrc_send_crt_release(void)
{
    mutex_unlock(&_cp_mutex);
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

const gnrc_send_crt_t *gnrc_send_crt_add(const gnrc_send_crt_t *crt)
{
    gnrc_send_crt_t *out;
    if (_crt_numof < ARRAY_SIZE(_crt_meta)) {
        out = &_crt_meta[_crt_numof++];
        *out = *crt;
        return out;
    }
    return NULL;
}

const gnrc_send_cp_t *gnrc_send_cp_add(const gnrc_send_cp_t *cp, const gnrc_send_crt_t *crt)
{
    assert(cp->ta);
    gnrc_send_cp_t *out;
    if (_crt_numof < ARRAY_SIZE(_crt_meta) && _cp_numof < ARRAY_SIZE(_cp_meta)) {
        _crt_meta[_crt_numof] = *crt;
        _cp_meta[_cp_numof] = *cp;
        out = &_cp_meta[_cp_numof++];
        out->crt = &_crt_meta[_crt_numof++];
        return out;
    }
    return NULL;
}
