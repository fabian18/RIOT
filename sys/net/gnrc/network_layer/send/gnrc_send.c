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
 * @brief   GNRC SEND impmementation
 *
 * @author  Fabian Hüßler <fabian.huessler@ovgu.de>
 * @}
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "kernel_defines.h"
#include "fmt.h"
#include "hashes/sha1.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/netif/internal.h"
#include "net/gnrc/icmpv6.h"
#include "net/gnrc/ndp.h"
#include "net/gnrc/send.h"
#include "net/inet_csum.h"
#include "net/ipv6/addr.h"
#include "net/ipv6/cga.h"
#include "net/ipv6/hdr.h"
#include "net/ndp.h"
#include "net/send.h"
#include "vfs.h"
#include "vfs_default.h"
#include "vfs_util.h"

#include "send_conf.h"
#include "send_internal.h"
#include "x509_ip_extn.h"

#define ENABLE_DEBUG            0
#define ENABLE_DEBUG_CRYPTO     0
#include "debug.h"

#define DEBUG_GNRC_SEND(s, ...)  DEBUG("[SEND GNRC] function %s: " s, __func__, ## __VA_ARGS__)

#define DEBUG_GNRC_SEND_CRYPTO(s, ...)  if (ENABLE_DEBUG_CRYPTO) { \
    DEBUG("[SEND GNRC CRYPTO] function %s: " s, __func__, ## __VA_ARGS__); }

#define GNRC_SEND_CGA_MSG_TYPE      { 0x08, 0x6F, 0xCA, 0x5E, 0x10, 0xB2, 0x00, 0xC9,   \
                                      0x9C, 0x8C, 0xE0, 0x01, 0x64, 0x27, 0x7C, 0x08 }

static int _compute_keyhash(void *hash, const void *pk, size_t pk_size);

static const uint8_t _oid_ip_resource[] = X509_EXTN_IP_RESOURCE_OID;

static inline size_t _min(size_t a, size_t b)
{
    return a <= b ? a : b;
}

static inline size_t _max(size_t a, size_t b)
{
    return a >= b ? a : b;
}

static int _load_pubkey(const char *path, void *buf, size_t size)
{
    return vfs_file_to_buffer(path, buf, size);
}

static int _load_key(const char *path, void *buf, size_t size)
{
    return vfs_file_to_buffer(path, buf, size);
}

static int _x509_crt_ext_cb(void *p_ctx,
                            mbedtls_x509_crt const *crt,
                            mbedtls_x509_buf const *oid,
                            int critical,
                            const unsigned char *p,
                            const unsigned char *end)
{
    (void)crt;
    /* assume certificate buffer mutex is locked */
    struct gnrc_send_x509_extn *extn = p_ctx;
    if (!extn) {
        return 0;
    }
    int ret;
    if (oid->len == ARRAY_SIZE(_oid_ip_resource) && !memcmp(oid->p, _oid_ip_resource, oid->len)) {
        if (!critical) {
            return -EBADMSG;
        }
        x509_ip_address_block_t ip[GNRC_SEND_SEC_PFX_NUMOF];
        if ((ret = x509_parse_ip_address_block(ip, ARRAY_SIZE(ip), p, end)) < 0) {
            DEBUG_GNRC_SEND("X509 IP extension contains errors\n");
            return ret;
        }
        for (int i = 0; i < ret; i++) {
            x509_ip_prefix_to_range(&ip[i]);
            int any = -1;
            for (int j = 0; j < extn->ip_block_numof; j++) {
                if (x509_ip_is_subrange(&ip[i], &extn->ip_block[j], &ip[i]) >= 0) {
                    any = i;
                }
            }
            if (extn->ip_block_numof > 0 && any == -1) {
                DEBUG_GNRC_SEND("X509 IP extension intersection is empty\n");
                return -EPERM;
            }
        }
        if (ret > 0) {
            memcpy(extn->ip_block, ip, sizeof(ip));
            extn->ip_block_numof = ret;
        }
    }
    return 0;
}

int gnrc_send_load_x509(const char *path, void *buf, size_t size,
                        gnrc_send_x509_crt_t *cert, gnrc_send_x509_extn_t *extn_ctx)
{
    int ret = 0;
    if (path && (ret = vfs_file_to_buffer(path, buf, size)) < 0) {
        return ret;
    }
    if (mbedtls_x509_crt_parse_der_with_ext_cb(&cert->cert, buf, size,
                                               0, _x509_crt_ext_cb, extn_ctx) < 0) {
        return -EBADMSG;
    }
    return ret;
}

int gnrc_send_parse_keys(const void *buf, size_t size, gnrc_send_pk_t *pk)
{
    int ret;
    mbedtls_pk_init(&pk->pk);
    if ((ret = mbedtls_pk_parse_key(&pk->pk, buf, size, NULL, 0))) {
        return ret;
    }
    return 0;
}

void gnrc_send_crt_init(gnrc_send_crt_t *crt, gnrc_send_x509_crt_t *x509)
{
    gnrc_send_name_identifier(&crt->subject_ident,
                              gnrc_send_x509_get_subject(x509),
                              gnrc_send_x509_get_subject_size(x509));
    gnrc_send_name_identifier(&crt->issuer_ident,
                              gnrc_send_x509_get_issuer(x509),
                              gnrc_send_x509_get_issuer_size(x509));
    gnrc_send_pk_identifier(&crt->pk_ident,
                            gnrc_send_x509_get_pubkey(x509),
                            gnrc_send_x509_get_pubkey_size(x509));
    crt->extn = (gnrc_send_x509_extn_t){ .ip_block_numof = 0 };
    crt->parent = NULL;
    crt->rel_path[0] = '\0';
}

void gnrc_send_clear_cp_vfs(void)
{
    int ret, read, plen;
    vfs_DIR dir;
    vfs_dirent_t entry;
    char path_buf[GNRC_SEND_PATH_MAX] = GNRC_SEND_CP_ROOT;
    plen = strlen(path_buf);
    path_buf[plen++] = '/';
    do {
        path_buf[plen] = '\0';
        if ((ret = vfs_opendir(&dir, path_buf)) < 0) {
            DEBUG_GNRC_SEND("Error deleting certificate paths\n");
            return; /* error */
        }
        while ((read = vfs_readdir(&dir, &entry)) > 0) {
            if (entry.d_name[0] == '.') {
                continue;
            }
            if ((sizeof(path_buf) - plen) <= strlen(entry.d_name)) {
                continue;
            }
            if (entry.d_name[0] == '_') {
                /* underscore shall denote own certificate, which should not be deleted */
                continue;
            }
            strcpy(&path_buf[plen], entry.d_name);
            break;
        }
        vfs_closedir(&dir);
        if (read > 0) {
            vfs_unlink(path_buf);
        }
    } while (read > 0);
}

int gnrc_send_load_ta(gnrc_send_crt_t *crt, unsigned crt_max,
                      gnrc_send_ta_t *ta, unsigned ta_max)
{
    int ret, plen;
    vfs_DIR dir;
    vfs_dirent_t entry;
    char path_buf[GNRC_SEND_PATH_MAX] = GNRC_SEND_TA_ROOT;
    unsigned m = _min(crt_max, ta_max);
    if ((ret = vfs_opendir(&dir, path_buf)) < 0) {
        return ret;
    }
    plen = strlen(path_buf);
    path_buf[plen++] = '/';
    path_buf[plen] = '\0';
    while (m) {
        if (vfs_readdir(&dir, &entry) <= 0) {
            break;
        }
        if (entry.d_name[0] == '.') {
            continue;
        }
        if ((sizeof(path_buf) - plen) <= strlen(entry.d_name)) {
            continue;
        }
        strcpy(&path_buf[plen], entry.d_name);
        gnrc_send_x509_extn_t extn = (gnrc_send_x509_extn_t){ 0 };
        size_t size;
        void *buf;
        gnrc_send_x509_crt_t *x509 = gnrc_send_x509_acquire(&size, &buf);
        ret = gnrc_send_load_x509(path_buf, buf, size, x509, &extn);
        if (ret < 0 ||
            gnrc_send_x509_get_pubkey_size(x509) > ARRAY_SIZE(ta->pk) ||
            gnrc_send_x509_get_subject_size(x509) > ARRAY_SIZE(ta->name)) {
            gnrc_send_x509_release();
            continue;
        }
        memcpy(ta->pk,
               gnrc_send_x509_get_pubkey(x509),
               (ta->pk_size = gnrc_send_x509_get_pubkey_size(x509)));
        memcpy(ta->name,
               gnrc_send_x509_get_subject(x509),
               (ta->name_size = gnrc_send_x509_get_subject_size(x509)));
        ta->crt = crt;
        gnrc_send_crt_init(ta->crt, x509);
        ta->crt->extn = extn;
        gnrc_send_x509_release();
        strcpy(ta->crt->rel_path, path_buf + plen);
        crt++;
        ta++;
        m--;
    }
    ret = _min(crt_max, ta_max) - m;
    vfs_closedir(&dir);
    return ret;
}

int gnrc_send_load_crt(gnrc_send_crt_t *crt, unsigned crt_max)
{
    int ret, plen;
    vfs_DIR dir;
    vfs_dirent_t entry;
    char path_buf[GNRC_SEND_PATH_MAX] = GNRC_SEND_CRT_ROOT;
    unsigned m = crt_max;
    if ((ret = vfs_opendir(&dir, path_buf)) < 0) {
        return ret;
    }
    plen = strlen(path_buf);
    path_buf[plen++] = '/';
    path_buf[plen] = '\0';
    while (m) {
        if (vfs_readdir(&dir, &entry) <= 0) {
            break;
        }
        if (entry.d_name[0] == '.') {
            continue;
        }
        if ((sizeof(path_buf) - plen) <= strlen(entry.d_name)) {
            continue;
        }
        strcpy(&path_buf[plen], entry.d_name);
        gnrc_send_x509_extn_t extn = (gnrc_send_x509_extn_t){ 0 };
        size_t size;
        void *buf;
        gnrc_send_x509_crt_t *x509 = gnrc_send_x509_acquire(&size, &buf);
        ret = gnrc_send_load_x509(path_buf, buf, size, x509, &extn);
        if (ret < 0) {
            gnrc_send_x509_release();
            continue;
        }
        gnrc_send_crt_init(crt, x509);
        crt->extn = extn;
        gnrc_send_x509_release();
        strcpy(crt->rel_path, path_buf + plen);
        assert(!memcmp(crt->issuer_ident.hash, crt->subject_ident.hash, sizeof(crt->issuer_ident.hash)));
        crt++;
        m--;
    }
    ret = crt_max - m;
    vfs_closedir(&dir);
    return ret;
}

int gnrc_send_load_cp(gnrc_send_crt_t *crt, unsigned crt_max,
                      gnrc_send_cp_t *cp, unsigned cp_max)
{
    int ret, plen;
    vfs_DIR dir;
    vfs_dirent_t entry;
    char path_buf[GNRC_SEND_PATH_MAX] = GNRC_SEND_CP_ROOT;
    unsigned m = _min(crt_max, cp_max);
    if ((ret = vfs_opendir(&dir, path_buf)) < 0) {
        return ret;
    }
    plen = strlen(path_buf);
    path_buf[plen++] = '/';
    path_buf[plen] = '\0';
    while (m) {
        if (vfs_readdir(&dir, &entry) <= 0) {
            break;
        }
        if (entry.d_name[0] == '.') {
            continue;
        }
        if ((sizeof(path_buf) - plen) <= strlen(entry.d_name)) {
            continue;
        }
        strcpy(&path_buf[plen], entry.d_name);
        size_t size;
        void *buf;
        gnrc_send_x509_crt_t *x509 = gnrc_send_x509_acquire(&size, &buf);
        ret = gnrc_send_load_x509(path_buf, buf, size, x509, NULL);
        if (ret < 0) {
            gnrc_send_x509_release();
            continue;
        }
        *cp = (gnrc_send_cp_t){
            .crt = crt,
            .num_comp = 1,
        };
        gnrc_send_crt_init(cp->crt, x509);
        gnrc_send_x509_release();
        strcpy(crt->rel_path, path_buf + plen);
        assert(memcmp(cp->crt->issuer_ident.hash,
                      cp->crt->subject_ident.hash,
                      sizeof(cp->crt->issuer_ident.hash)));
        /* build path */
        gnrc_send_crt_t *tmp = cp->crt;
        gnrc_send_ident_t issuer = cp->crt->issuer_ident;
        while (memcmp(tmp->issuer_ident.hash, tmp->subject_ident.hash, sizeof(tmp->issuer_ident.hash))) {
            gnrc_send_crt_t *parent = gnrc_send_get_crt_by_subject_ident(&issuer);
            if (!parent) {
                goto skip;
            }
            issuer = parent->issuer_ident;
            tmp->parent = parent;
            tmp = parent;
            cp->num_comp++;
        };
        cp->ta = gnrc_send_get_ta_by_subject_ident(&issuer);
        assert(cp->ta);
        /* verify all paths beginning from the Trust Anchor */
        gnrc_send_x509_extn_t extn = (gnrc_send_x509_extn_t){ 0 };
        gnrc_send_crt_t *parent = cp->ta->crt;
        while (parent != cp->crt) {
            tmp = cp->crt;
            while (tmp->parent != parent) {
                tmp = tmp->parent;
            }
            x509 = gnrc_send_x509_acquire(&size, &buf);
            if (parent == cp->ta->crt) {
                strcpy(path_buf, GNRC_SEND_TA_ROOT"/");
            }
            else {
                strcpy(path_buf, GNRC_SEND_CRT_ROOT"/");
            }
            strcat(path_buf, parent->rel_path);
            if ((ret = gnrc_send_load_x509(path_buf, buf, size, x509, &extn)) < 0) {
                gnrc_send_x509_release();
                goto skip;
            }
            buf = ((uint8_t *)buf) + ret;
            size -= ret;
            if (tmp == cp->crt) {
                strcpy(path_buf, GNRC_SEND_CP_ROOT"/");
            }
            else {
                strcpy(path_buf, GNRC_SEND_CRT_ROOT"/");
            }
            strcat(path_buf, tmp->rel_path);
            if ((ret = gnrc_send_load_x509(path_buf, buf, size, x509 + 1, &extn)) < 0) {
                gnrc_send_x509_release();
                goto skip;
            }
            tmp->extn = extn;
            if ((ret = gnrc_send_x509_verify(x509 + 1, x509))) {
                gnrc_send_x509_release();
                goto skip;
            }
            gnrc_send_x509_release();
            parent = tmp;
        }
        cp++;
        crt++;
        m--;
skip: ;
    }
    ret = _min(crt_max, cp_max) - m;
    vfs_closedir(&dir);
    return ret;
}

int gnrc_send_load_keys(gnrc_send_key_t *key, void *key_buf, size_t key_buf_size)
{
    int ret, key_len;
    char path_buf[GNRC_SEND_PATH_MAX] = GNRC_SEND_KEY_ROOT;
    key_len = strlen(path_buf);
    if (sizeof(path_buf) - key_len <= 1 +
                                      _max(strlen(GNRC_SEND_PK_VFS_NAME),
                                           strlen(GNRC_SEND_KEY_VFS_NAME))) {
        return -ENOBUFS;
    }
    path_buf[key_len++] = '/';
    path_buf[key_len] = '\0';
    memset(key, 0, sizeof(*key));
    strcpy(&path_buf[key_len], GNRC_SEND_PK_VFS_NAME);
    if ((ret = _load_pubkey(path_buf, key_buf, key_buf_size)) < 0) {
        return ret;
    }
    key->pubkey = key_buf;
    key->pubkey_size = ret;
    key_buf = ((uint8_t *)key_buf) + key->pubkey_size;
    key_buf_size -= key->pubkey_size;
    strcpy(&path_buf[key_len], GNRC_SEND_KEY_VFS_NAME);
    if ((ret = _load_key(path_buf, key_buf, key_buf_size)) < 0) {
        return ret;
    }
    key->key = key_buf;
    key->key_size = ret;
    key_buf = ((uint8_t *)key_buf) + key->key_size;
    key_buf_size -= key->key_size;
    if ((ret = gnrc_send_parse_keys(key->key, key->key_size, &key->pk)) < 0) {
        return ret;
    }
    return 1;
}

void gnrc_send_init(void)
{
    gnrc_send_init_node();
    /* cast away const once as an exception here */
    _compute_keyhash((void *)gnrc_send_get_self_ident()->hash,
                     gnrc_send_get_pubkey_der(),
                     gnrc_send_get_pubkey_size_der());
}

int gnrc_send_cga_generate(gnrc_netif_ipv6_t *netif, ipv6_addr_t *dst, ipv6_cga_parameters_t *params)
{
    (void)netif;
    return ipv6_cga_generate(dst, params,
                             gnrc_send_get_pubkey_der(),
                             gnrc_send_get_pubkey_size_der(),
                             GNRC_SEND_CGA_SEC,
                             params->collision_count);
}

int gnrc_send_check_iface(const gnrc_netif_ipv6_t *netif)
{
    (void)netif;
    if (!gnrc_send_get_pubkey_size_der()) {
        return -ENOTSUP;
    }
    if (!(netif->aac_mode & GNRC_NETIF_AAC_AUTO)) {
        return -ENOTSUP;
    }
    return GNRC_SEND_STATUS_OK;
}

int gnrc_send_enable_iface(gnrc_netif_ipv6_t *netif)
{
    (void)netif;
    return GNRC_SEND_STATUS_OK;
}

void gnrc_send_disable_iface(gnrc_netif_ipv6_t *netif)
{
    (void)netif;
}

const void *gnrc_send_get_iface_pubkey_der(gnrc_netif_ipv6_t *netif, size_t *pk_size)
{
    (void)netif;
    *pk_size = gnrc_send_get_pubkey_size_der();
    return gnrc_send_get_pubkey_der();
}

void gnrc_send_pk_identifier(gnrc_send_ident_t *ident, const void *pk, size_t pk_size)
{
    _compute_keyhash(ident, pk, pk_size);
}

void gnrc_send_name_identifier(gnrc_send_ident_t *ident, const void *name, size_t name_size)
{
    _compute_keyhash(ident, name, name_size);
}

static
gnrc_pktsnip_t *gnrc_ndp_opt_cga_params_build(const ipv6_cga_parameters_t *cga_params,
                                              const void *pk, size_t pk_size,
                                              void *extn, size_t extn_size,
                                              gnrc_pktsnip_t *next)
{
    size_t size = sizeof(ndp_opt_cga_params_t) + pk_size + extn_size;
    gnrc_pktsnip_t *pkt = gnrc_ndp_opt_build(NDP_OPT_CGA_PARAMETERS, size, next);
    if (!pkt) {
        return NULL;
    }
    ndp_opt_cga_params_t *opt = pkt->data;
    memset(((uint8_t *)opt) + sizeof(ndp_opt_t), 0, pkt->size - sizeof(ndp_opt_t));
    if (cga_params) {
        opt->cga_par = *cga_params;
    }
    opt->pad_len = pkt->size - size;
    if (pk) {
        memcpy(((uint8_t *)opt) + sizeof(*opt), pk, pk_size);
    }
    if (extn) {
        memcpy(((uint8_t *)opt) + sizeof(*opt) + pk_size, extn, extn_size);
    }
    return pkt;
}

static
gnrc_pktsnip_t *gnrc_ndp_opt_signature_build(const void *khash,
                                             const void *sig, size_t sig_size)
{
    size_t size = sizeof(ndp_opt_sig_t) + sig_size;
    gnrc_pktsnip_t *pkt = gnrc_ndp_opt_build(NDP_OPT_SIGNATURE, size, NULL);
    if (!pkt) {
        return NULL;
    }
    ndp_opt_sig_t *opt = pkt->data;
    memset(((uint8_t *)opt) + sizeof(ndp_opt_t), 0, pkt->size - sizeof(ndp_opt_t));
    if (khash) {
        memcpy(opt->key_hash, khash, sizeof(opt->key_hash));
    }
    if (sig) {
        memcpy(((uint8_t *)opt) + sizeof(*opt), sig, sig_size);
    }
    return pkt;
}

static
gnrc_pktsnip_t *gnrc_ndp_opt_nonce_build(void *nonce, size_t nonce_size,
                                         gnrc_pktsnip_t *next)
{
    size_t size = sizeof(ndp_opt_nonce_t) + nonce_size;
    if (size % 8) {
        return NULL;
    }
    gnrc_pktsnip_t *pkt = gnrc_ndp_opt_build(NDP_OPT_NONCE, size, next);
    if (!pkt) {
        return NULL;
    }
    ndp_opt_nonce_t *opt = pkt->data;
    memset(((uint8_t *)opt) + sizeof(ndp_opt_t), 0, pkt->size - sizeof(ndp_opt_t));
    if (nonce) {
        memcpy(((uint8_t *)opt) + sizeof(*opt), nonce, nonce_size);
    }
    return pkt;
}

static
gnrc_pktsnip_t *gnrc_ndp_opt_trust_anchor_build(const void *name, size_t name_size,
                                                ndp_ta_name_type_t name_type,
                                                gnrc_pktsnip_t *next)
{
    size_t size = sizeof(ndp_opt_ta_t) + name_size;
    gnrc_pktsnip_t *pkt = gnrc_ndp_opt_build(NDP_OPT_TRUST_ANCHOR, size, next);
    if (!pkt) {
        return NULL;
    }
    ndp_opt_ta_t *opt = pkt->data;
    memset(((uint8_t *)opt) + sizeof(ndp_opt_t), 0, pkt->size - sizeof(ndp_opt_t));
    if (name) {
        memcpy(((uint8_t *)opt) + sizeof(ndp_opt_ta_t), name, name_size);
    }
    opt->name_type = name_type;
    opt->pad_len = pkt->size - size;
    return pkt;
}

static
gnrc_pktsnip_t *gnrc_ndp_opt_certificate_build(const void *cert, size_t cert_size,
                                               ndp_cert_type_t cert_type,
                                               gnrc_pktsnip_t *next)
{
    size_t size = sizeof(ndp_opt_cert_t) + cert_size;
    gnrc_pktsnip_t *pkt = gnrc_ndp_opt_build(NDP_OPT_CERTIFICATE, size, next);
    if (!pkt) {
        return NULL;
    }
    ndp_opt_cert_t *opt = pkt->data;
    memset(((uint8_t *)opt) + sizeof(ndp_opt_t), 0, pkt->size - sizeof(ndp_opt_t));
    if (cert) {
        memcpy(((uint8_t *)opt) + sizeof(ndp_opt_cert_t), cert, cert_size);
    }
    opt->cert_type = cert_type;
    return pkt;

}

static
gnrc_pktsnip_t *gnrc_ndp_cp_sol_build(uint16_t ident, uint16_t comp,
                                      gnrc_pktsnip_t *options)
{
    gnrc_pktsnip_t *pkt = gnrc_icmpv6_build(options, ICMPV6_CP_SOL, 0, sizeof(ndp_cp_sol_t));
    if (!pkt) {
        return NULL;
    }
    ndp_cp_sol_t *msg = pkt->data;
    memset(((uint8_t *)msg) + sizeof(icmpv6_hdr_t), 0, pkt->size - sizeof(icmpv6_hdr_t));
    msg->ident = byteorder_htons(ident);
    msg->comp = byteorder_htons(comp);
    return pkt;
}

static
gnrc_pktsnip_t *gnrc_ndp_cp_adv_build(uint16_t ident, uint16_t comp,
                                      uint16_t all_comp,
                                      gnrc_pktsnip_t *options)
{
    gnrc_pktsnip_t *pkt = gnrc_icmpv6_build(options, ICMPV6_CP_ADV, 0, sizeof(ndp_cp_adv_t));
    if (!pkt) {
        return NULL;
    }
    ndp_cp_adv_t *msg = pkt->data;
    memset(((uint8_t *)msg) + sizeof(icmpv6_hdr_t), 0, pkt->size - sizeof(icmpv6_hdr_t));
    msg->ident = byteorder_htons(ident);
    msg->comp = byteorder_htons(comp);
    msg->all_comp = byteorder_htons(all_comp);
    return pkt;
}

gnrc_pktsnip_t *gnrc_send_cga_params_build(const ipv6_addr_t *addr, gnrc_netif_t *netif,
                                           gnrc_pktsnip_t *next)
{
    ipv6_cga_parameters_t *params = NULL;
    gnrc_ipv6_cga_ctx_t *cga_ctx = gnrc_netif_ipv6_get_cga_ctx(&netif->ipv6);
    int idx = gnrc_netif_ipv6_addr_idx(netif, addr);
    if (idx < 0) {
        return NULL;
    }
    if (netif->ipv6.addrs_priv[idx] & GNRC_NETIF_IPV6_ADDR_PRIV_CGA) {
        for (int p_idx = 0; p_idx < (int)ARRAY_SIZE(cga_ctx->addr_idx); p_idx++) {
            if (idx == cga_ctx->addr_idx[p_idx]) {
                params = &cga_ctx->params[p_idx];
                break;
            }
        }
        if (!params) {
            return NULL;
        }
        size_t pk_size;
        const void *pk = gnrc_send_get_iface_pubkey_der(&netif->ipv6, &pk_size);
        return gnrc_ndp_opt_cga_params_build(params,
                                             pk, pk_size,
                                             NULL, 0, next);
    }
    return NULL;
}

static uint16_t _icmp_checksum(const gnrc_pktsnip_t *icmpv6, uint16_t icmpv6_len,
                               const ipv6_addr_t *src, const ipv6_addr_t *dst)
{
    uint16_t cs;
    union {
        struct {
            ipv6_addr_t src;
            ipv6_addr_t dst;
            uint32_t len;
            uint16_t cs;
            uint16_t nh;
        } pseudo_hdr;
        struct {
            icmpv6_hdr_t icmp_hdr;
        } hdr;
    } u_hdr;
    u_hdr.pseudo_hdr.src = *src;
    u_hdr.pseudo_hdr.dst = *dst;
    u_hdr.pseudo_hdr.len = byteorder_htonl(icmpv6_len).u32;
    u_hdr.pseudo_hdr.cs = 0;
    u_hdr.pseudo_hdr.nh = byteorder_htons(PROTNUM_ICMPV6).u16;
    cs = ipv6_hdr_inet_csum(0, (ipv6_hdr_t *)&u_hdr.pseudo_hdr, PROTNUM_ICMPV6, icmpv6_len);

    u_hdr.hdr.icmp_hdr = *(icmpv6_hdr_t *)icmpv6->data;
    u_hdr.hdr.icmp_hdr.csum.u16 = 0;
    size_t len = 0;
    cs = inet_csum_slice(cs, (const uint8_t *)&u_hdr.hdr.icmp_hdr, sizeof(u_hdr.hdr.icmp_hdr), len);

    len += sizeof(icmpv6_hdr_t);
    assert(icmpv6->size >= sizeof(icmpv6_hdr_t));
    cs = inet_csum_slice(cs, (const uint8_t *)(((icmpv6_hdr_t *)icmpv6->data) + 1),
                         icmpv6->size - sizeof(icmpv6_hdr_t), len);

    len += icmpv6->size - sizeof(icmpv6_hdr_t);
    while (icmpv6->next) {
        icmpv6 = icmpv6->next;
        cs = inet_csum_slice(cs, icmpv6->data, icmpv6->size, len);
        len += icmpv6->size;
    }
    assert(len == icmpv6_len);
    return byteorder_htons(~cs).u16;
}

static int _compute_keyhash(void *hash, const void *pk, size_t pk_size)
{
    int ret;
    mbedtls_md_context_t md;
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    if (!md_info) {
        return -ENOTSUP;
    }
    mbedtls_md_init(&md);
    if ((ret = mbedtls_md_setup(&md, md_info, 0))) {
        mbedtls_md_free(&md);
        return ret;
    }
    if ((ret = mbedtls_md_update(&md, pk, pk_size))) {
        mbedtls_md_free(&md);
        return ret;
    }
    if ((ret = mbedtls_md_finish(&md, hash))) {
        mbedtls_md_free(&md);
        return ret;
    }
    mbedtls_md_free(&md);
    return 0;
}

static int _compute_signature(void *sig, size_t *sig_size,
                              const ipv6_addr_t *src, const ipv6_addr_t *dst,
                              const gnrc_pktsnip_t *icmpv6,
                              uint16_t icmp_cs)
{
    int ret;
    uint8_t hash[GNRC_SEND_ECDSA_MD_SIZE] = GNRC_SEND_CGA_MSG_TYPE;
    {
        mbedtls_md_context_t md;
        const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(GNRC_SEND_ECDSA_MD);
        mbedtls_md_init(&md);
        if ((ret = mbedtls_md_setup(&md, md_info, 0))) {
            mbedtls_md_free(&md);
            return ret;
        }
        if ((ret = mbedtls_md_starts(&md))) {
            mbedtls_md_free(&md);
            return ret;
        }
        if ((ret = mbedtls_md_update(&md, hash, sizeof((uint8_t[])GNRC_SEND_CGA_MSG_TYPE)))) {
            mbedtls_md_free(&md);
            return ret;
        }
        if ((ret = mbedtls_md_update(&md, (const uint8_t *)src, sizeof(*src)))) {
            mbedtls_md_free(&md);
            return ret;
        }
        if ((ret = mbedtls_md_update(&md, (const uint8_t *)dst, sizeof(*dst)))) {
            mbedtls_md_free(&md);
            return ret;
        }
        icmpv6_hdr_t icmpv6_hdr = {
            .type = ((icmpv6_hdr_t *)icmpv6->data)->type,
            .code = ((icmpv6_hdr_t *)icmpv6->data)->code,
            .csum = { .u16 = icmp_cs },
        };
        if ((ret = mbedtls_md_update(&md, (const uint8_t *)&icmpv6_hdr, sizeof(icmpv6_hdr)))) {
            mbedtls_md_free(&md);
            return ret;
        }
        if ((ret = mbedtls_md_update(&md, (const uint8_t *)&(((icmpv6_hdr_t *)icmpv6->data)[1]),
                                     icmpv6->size - sizeof(icmpv6_hdr_t)))) {
            mbedtls_md_free(&md);
            return ret;
        }
        while (icmpv6->next) {
            icmpv6 = icmpv6->next;
            if ((ret = mbedtls_md_update(&md, icmpv6->data, icmpv6->size))) {
                mbedtls_md_free(&md);
                return ret;
            }
        }
        if ((ret = mbedtls_md_finish(&md, hash))) {
            mbedtls_md_free(&md);
            return ret;
        }
        mbedtls_md_free(&md);
        DEBUG_GNRC_SEND_CRYPTO("_compute_signature(): hash is\n"GNRC_SEND_ECDSA_MD_FORMAT"\n",
                               GNRC_SEND_ECDSA_MD_VALUE(hash));
    }
    mbedtls_ecdsa_context ctx_sign;
    mbedtls_ecdsa_init(&ctx_sign);
    if ((ret = mbedtls_ecdsa_from_keypair(&ctx_sign, mbedtls_pk_ec(gnrc_send_get_pk()->pk)))) {
        mbedtls_ecdsa_free(&ctx_sign);
        return ret;
    }
    if ((ret = mbedtls_ecdsa_write_signature(&ctx_sign,
                                             GNRC_SEND_ECDSA_MD, hash, sizeof(hash),
                                             sig, sig_size,
                                             gnrc_send_random_get, NULL))) {
        mbedtls_ecdsa_free(&ctx_sign);
        return ret;
    }
    assert(MBEDTLS_ECDSA_MAX_SIG_LEN(GNRC_SEND_ECDSA_SIG_BITS) >= *sig_size);
    mbedtls_ecdsa_free(&ctx_sign);
    return 0;
}

gnrc_pktsnip_t *gnrc_send_signature_build(const gnrc_pktsnip_t *icmpv6, gnrc_netif_t *netif,
                                          const ipv6_addr_t *src, const ipv6_addr_t *dst)
{
    (void)netif;
    int ret;
    size_t sig_size;
    uint8_t sig[MBEDTLS_ECDSA_MAX_SIG_LEN(GNRC_SEND_ECDSA_SIG_BITS)];
    uint16_t cs = _icmp_checksum(icmpv6, gnrc_pkt_len(icmpv6), src, dst);
    if ((ret = _compute_signature(sig, &sig_size, src, dst, icmpv6, cs))) {
        return NULL;
    }
    return gnrc_ndp_opt_signature_build(gnrc_send_get_self_ident()->hash, sig, sig_size);
}

static int _verify_signature(const icmpv6_hdr_t *icmpv6,
                             const ndp_opt_sig_t *sig, size_t sig_size,
                             const void *pk, size_t pk_size,
                             const ipv6_addr_t *src, const ipv6_addr_t *dst,
                             uint16_t icmp_cs)
{
    int ret;
    uint8_t hash[GNRC_SEND_ECDSA_MD_SIZE] = GNRC_SEND_CGA_MSG_TYPE;
    {
        mbedtls_md_context_t md;
        const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(GNRC_SEND_ECDSA_MD);
        mbedtls_md_init(&md);
        if ((ret = mbedtls_md_setup(&md, md_info, 0))) {
            mbedtls_md_free(&md);
            return ret;
        }
        if ((ret = mbedtls_md_starts(&md))) {
            mbedtls_md_free(&md);
            return ret;
        }
        if ((ret = mbedtls_md_update(&md, hash, sizeof((uint8_t[])GNRC_SEND_CGA_MSG_TYPE)))) {
            mbedtls_md_free(&md);
            return ret;
        }
        if ((ret = mbedtls_md_update(&md, (const uint8_t *)src, sizeof(*src)))) {
            mbedtls_md_free(&md);
            return ret;
        }
        if ((ret = mbedtls_md_update(&md, (const uint8_t *)dst, sizeof(*dst)))) {
            mbedtls_md_free(&md);
            return ret;
        }
        icmpv6_hdr_t icmpv6_hdr = {
            .type = icmpv6->type,
            .code = icmpv6->code,
            .csum = { .u16 = icmp_cs },
        };
        if ((ret = mbedtls_md_update(&md, (const uint8_t *)&icmpv6_hdr, sizeof(icmpv6_hdr)))) {
            mbedtls_md_free(&md);
            return ret;
        }
        size_t icmpv6_size = (uintptr_t)sig - (uintptr_t)icmpv6;
        if (icmpv6_size < sizeof(icmpv6_hdr_t)) {
            mbedtls_md_free(&md);
            return -GNRC_SEND_STATUS_BAD_FORMAT;
        }
        if ((ret = mbedtls_md_update(&md, (const uint8_t *)(icmpv6 + 1),
                                     icmpv6_size - sizeof(*icmpv6)))) {
            mbedtls_md_free(&md);
            return ret;
        }
        if ((ret = mbedtls_md_finish(&md, hash))) {
            mbedtls_md_free(&md);
            return ret;
        }
        mbedtls_md_free(&md);
        DEBUG_GNRC_SEND_CRYPTO("_verify_signature(): hash is\n"GNRC_SEND_ECDSA_MD_FORMAT"\n",
                               GNRC_SEND_ECDSA_MD_VALUE(hash));
    }
    mbedtls_ecdsa_context ctx_verify;
    mbedtls_ecdsa_init(&ctx_verify);
    {
        mbedtls_pk_context pk_ctx;
        mbedtls_pk_init(&pk_ctx);
        if ((ret = mbedtls_pk_parse_public_key(&pk_ctx, pk, pk_size))) {
            mbedtls_pk_free(&pk_ctx);
            mbedtls_ecdsa_free(&ctx_verify);
            return ret;
        }
        if ((ret = mbedtls_ecdsa_from_keypair(&ctx_verify, mbedtls_pk_ec(pk_ctx)))) {
            mbedtls_pk_free(&pk_ctx);
            mbedtls_ecdsa_free(&ctx_verify);
            return ret;
        }
        mbedtls_pk_free(&pk_ctx);
    }
    /* Unfortunately the signature has no padding field.
       Hence the signatures length must be clear from its DER encoding.
       See errata https://www.rfc-editor.org/errata_search.php?rfc=3971.
       First figure out the length of the pure signature encoded in the DER tag and
       then add the length overhead of the DER tag. */
    uint8_t *sig_der = (uint8_t *)gnrc_send_opt_sig_get_sig(sig);
    if ((ret = mbedtls_asn1_get_tag(&sig_der, sig_der + sig_size, &sig_size,
                                    MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE)) != 0) {
        mbedtls_ecdsa_free(&ctx_verify);
        return -GNRC_SEND_STATUS_BAD_FORMAT;
    }
    if ((ret = mbedtls_ecdsa_read_signature(&ctx_verify,
                                            hash, sizeof(hash),
                                            (const uint8_t *)(sig + 1),
                                            sig_size + (sig_der - ((const uint8_t *)(sig + 1)))))) {
        mbedtls_ecdsa_free(&ctx_verify);
        return -GNRC_SEND_STATUS_SIGNATURE_FAIL;
    }
    mbedtls_ecdsa_free(&ctx_verify);
    return GNRC_SEND_STATUS_OK;
}

int gnrc_send_signature_check(const icmpv6_hdr_t *icmpv6, const ndp_opt_sig_t *sig,
                              const void *pk, size_t pk_size,
                              const ipv6_addr_t *src, const ipv6_addr_t *dst)
{
    if (sig->len * 8 < sizeof(*sig)) {
        return -GNRC_SEND_STATUS_BAD_FORMAT;
    }
    int ret;
    size_t sig_max_size = sig->len * 8 - sizeof(*sig);
    /*  Such a key
        can either be stored in the certificate cache of the receiver or
        be received in the CGA option in the same message.
        [RFC 3971](https://datatracker.ietf.org/doc/html/rfc3971#section-5.2) */
    gnrc_send_x509_crt_t *x509 = NULL;
    if ((uintptr_t)sig <= (uintptr_t)icmpv6 ||
        (uintptr_t)sig - (uintptr_t)icmpv6 < sizeof(icmpv6_hdr_t)) {
        return -GNRC_SEND_STATUS_BAD_FORMAT;
    }
    if (pk) {
        uint8_t keyhash[SHA1_DIGEST_LENGTH];
        if (_compute_keyhash(keyhash, pk, pk_size) ||
            memcmp(sig->key_hash, keyhash, sizeof(sig->key_hash))) {
            return -GNRC_SEND_STATUS_NO_KEY;
        }
    }
    else {
        gnrc_send_crt_iter_t iter = gnrc_send_crt_acquire();
        while (gnrc_send_crt_iterator(&iter)) {
            if (!memcmp(sig->key_hash, iter.crt->pk_ident.hash, sizeof(sig->key_hash))) {
                break;
            }
        }
        gnrc_send_crt_release();
        if (!iter.crt) {
            return -GNRC_SEND_STATUS_NO_KEY;
        }
        size_t size;
        void *buf;
        /* It might be more efficient to store the public key in crt datatype
           instead of loading the certificate here. */
        x509 = gnrc_send_x509_acquire(&size, &buf);
        gnrc_send_load_x509(iter.crt->rel_path, buf, size, x509, NULL);
        pk = gnrc_send_x509_get_pubkey(x509);
        pk_size = gnrc_send_x509_get_pubkey_size(x509);
    }
    gnrc_pktsnip_t icmpv6_snip = {
        .data = (void *)icmpv6,
        .size = (uintptr_t)sig - (uintptr_t)icmpv6,
    };
    uint16_t cs = _icmp_checksum(&icmpv6_snip, icmpv6_snip.size, src, dst);
    if ((ret = _verify_signature(icmpv6, sig, sig_max_size, pk, pk_size, src, dst, cs))) {
        if (x509) {
            gnrc_send_x509_release();
        }
        return ret;
    }
    if (x509) {
        gnrc_send_x509_release();
    }
    return GNRC_SEND_STATUS_OK;
}

gnrc_pktsnip_t *gnrc_send_nonce_build(void *nonce, size_t size, gnrc_pktsnip_t *next)
{
    return gnrc_ndp_opt_nonce_build(nonce, size, next);
}

gnrc_pktsnip_t *gnrc_send_trust_anchor_build(const gnrc_send_ta_t *ta, ndp_ta_name_type_t type, gnrc_pktsnip_t *next)
{
    return gnrc_ndp_opt_trust_anchor_build(ta->name, ta->name_size, type, next);
}

static int _x509_verify_cb(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
{
    (void)data; (void)crt; (void)depth; (void)flags;
    return 0;
}

int gnrc_send_x509_verify(gnrc_send_x509_crt_t *crt,
                          gnrc_send_x509_crt_t *trust)
{
    uint32_t flags;
    return mbedtls_x509_crt_verify(&crt->cert, &trust->cert, NULL,
                                   NULL, &flags, _x509_verify_cb, NULL);
}

int gnrc_send_cp_sol_send(gnrc_netif_t *netif,
                          const ipv6_addr_t *src, const ipv6_addr_t *dst,
                          uint16_t comp, gnrc_send_cache_cps_t *cps_ctx)
{
    assert(cps_ctx);
    gnrc_pktsnip_t *hdr, *pkt = NULL, *cp_sol;
    if (!src && !(src = gnrc_netif_ipv6_addr_best_src(netif, dst, true))) {
        src = &ipv6_addr_unspecified;
    }
    /* add TA options that the client is willing to accept certificates from */
    gnrc_send_ta_iter_t iter = gnrc_send_ta_acquire();
    while ((gnrc_send_ta_iterator(&iter))) {
        if ((hdr = gnrc_send_trust_anchor_build(iter.ta, NDP_TA_TYPE_DER, pkt))) {
            pkt = hdr;
        }
    }
    gnrc_send_ta_release();
    if (!(hdr = gnrc_ndp_cp_sol_build(0, comp, pkt))) {
        goto release;
    }
    cp_sol = pkt = hdr;
    if (!(hdr = gnrc_ipv6_hdr_build(pkt, src, dst))) {
        goto release;
    }
    ((ipv6_hdr_t *)hdr->data)->hl = NDP_HOP_LIMIT;
    pkt = hdr;
    if (!(hdr = gnrc_netif_hdr_build(NULL, 0, NULL, 0))) {
        goto release;
    }
    pkt = gnrc_pkt_prepend(pkt, hdr);
    ((ndp_cp_sol_t *)cp_sol->data)->ident = byteorder_htons(cps_ctx->identifier);
    gnrc_netif_hdr_set_netif(pkt->data, netif);
    if (gnrc_netapi_dispatch_send(GNRC_NETTYPE_NDP, GNRC_NETREG_DEMUX_CTX_ALL, pkt) == 0) {
        DEBUG_GNRC_SEND("unable to dispatch Certificate Path Solicitation\n");
        gnrc_pktbuf_release(pkt);
        return -ECANCELED;
    }
    DEBUG_GNRC_SEND("sent CPS ctx=%p\n", (void *)cps_ctx);
    return GNRC_SEND_STATUS_OK;
release:
    DEBUG_GNRC_SEND("No memory\n");
    gnrc_pktbuf_release(pkt);
    return -ENOMEM;
}

int gnrc_send_cp_adv_send(gnrc_netif_t *netif,
                          const ipv6_addr_t *src, const ipv6_addr_t *dst,
                          uint16_t comp, uint16_t ident, const gnrc_send_cp_t *cp,
                          gnrc_send_cache_cpa_t *cpa_ctx,
                          gnrc_pktsnip_t *ext_opts)
{
    gnrc_pktsnip_t *hdr, *pkt = ext_opts;
    if (!src && !(src = gnrc_netif_ipv6_addr_best_src(netif, dst, true))) {
        DEBUG_GNRC_SEND("No source address\n");
        gnrc_pktbuf_release(pkt);
        return -EADDRNOTAVAIL;
    }
    if (cp) {
        if (comp == SEND_CPS_ALL_COMP) {
            comp = cp->num_comp - 1;
        }
        if (comp >= cp->num_comp) {
            gnrc_pktbuf_release(pkt);
            DEBUG_GNRC_SEND("Component out of range\n");
            return -EINVAL;
        }
        gnrc_send_crt_t *crt = cp->crt;
        for (unsigned i = 1; i < comp; i++) {
            assert(crt->parent);
            crt = crt->parent;
        }
        /* add TA option if the first component was requested */
        if (comp == cp->num_comp - 1) {
            if (!(hdr = gnrc_ndp_opt_trust_anchor_build(cp->ta->name, cp->ta->name_size, NDP_TA_TYPE_DER, pkt))) {
                goto release;
            }
            pkt = hdr;
        }
        if (comp) {
            if (crt->rel_path[0]) {
                /* load certificate from VFS and add certificate option */
                char path[GNRC_SEND_PATH_MAX];
                if (comp == cp->num_comp - 1) {
                    strcpy(path, GNRC_SEND_CP_ROOT"/");
                }
                else {
                    strcpy(path, GNRC_SEND_CRT_ROOT"/");
                }
                strcat(path, crt->rel_path);
                struct stat st;
                if (vfs_stat(path, &st) < 0) {
                    DEBUG_GNRC_SEND("Certificate not found\n");
                    gnrc_pktbuf_release(pkt);
                    return -ENOENT;
                }
                if (!(hdr = gnrc_ndp_opt_certificate_build(NULL, st.st_size, NDP_CERT_TYPE_DER, pkt))) {
                    goto release;
                }
                pkt = hdr;
                if (vfs_file_to_buffer(path, ((ndp_opt_cert_t *)pkt->data) + 1, pkt->size - sizeof(ndp_opt_cert_t)) < 0) {
                    goto release;
                }
            }
            else {
                assert(crt->crt_inmem.zero == 0);
                assert(crt->crt_inmem.buffer);
                /* certificate is already in some buffer */
                if (!(hdr = gnrc_ndp_opt_certificate_build(crt->crt_inmem.buffer, crt->crt_inmem.size,
                                                           crt->crt_inmem.type, pkt))) {
                    goto release;
                }
                pkt = hdr;
            }
        }
    }
    if (!(hdr = gnrc_ndp_cp_adv_build(ident, comp, cp ? cp->num_comp : 0, pkt))) {
        goto release;
    }
    pkt = hdr;
    if (!(hdr = gnrc_ipv6_hdr_build(pkt, src, dst))) {
        goto release;
    }
    ((ipv6_hdr_t *)hdr->data)->hl = NDP_HOP_LIMIT;
    pkt = hdr;
    if (!(hdr = gnrc_netif_hdr_build(NULL, 0, NULL, 0))) {
        goto release;
    }
    pkt = gnrc_pkt_prepend(pkt, hdr);
    gnrc_netif_hdr_set_netif(pkt->data, netif);
    if (gnrc_netapi_dispatch_send(GNRC_NETTYPE_NDP, GNRC_NETREG_DEMUX_CTX_ALL, pkt) == 0) {
        DEBUG_GNRC_SEND("unable to dispatch Certificate Path Advertisement\n");
        gnrc_pktbuf_release(pkt);
        return -ECANCELED;
    }
    (void)cpa_ctx;
    DEBUG_GNRC_SEND("sent CPA ctx=%p\n", (void *)cpa_ctx);
    return GNRC_SEND_STATUS_OK;
release:
    DEBUG_GNRC_SEND("No memory\n");
    gnrc_pktbuf_release(pkt);
    return -ENOMEM;
}
