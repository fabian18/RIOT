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
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/errno.h>

#include "fmt.h"
#include "hashes/sha1.h"
#include "base64.h"
#include "net/gnrc/ipv6/nib/send.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/netif/internal.h"
#include "net/gnrc/icmpv6.h"
#include "net/gnrc/ndp.h"
#include "net/gnrc/send.h"
#include "net/ipv6/cga.h"
#include "net/ipv6/hdr.h"
#include "net/ndp.h"
#include "net/send.h"
#include "vfs_default.h"
#include "vfs_util.h"
#include "send_internal.h"

#define ENABLE_DEBUG            0
#define ENABLE_DEBUG_CRYPTO     0
#include "debug.h"

#define DEBUG_GNRC_SEND(s, ...)  DEBUG("[SEND GNRC] function %s: " s, __func__, ## __VA_ARGS__)

#define DEBUG_GNRC_SEND_CRYPTO(s, ...)  if (ENABLE_DEBUG_CRYPTO) { \
    DEBUG("[SEND GNRC CRYPTO] function %s: " s, __func__, ## __VA_ARGS__); }

#define GNRC_SEND_CGA_MSG_TYPE      { 0x08, 0x6F, 0xCA, 0x5E, 0x10, 0xB2, 0x00, 0xC9,   \
                                      0x9C, 0x8C, 0xE0, 0x01, 0x64, 0x27, 0x7C, 0x08 }

static int _compute_keyhash(void *hash, const void *pk, size_t pk_size);

static int _x509_crt_ext_cb(void *p_ctx,
                            mbedtls_x509_crt const *crt,
                            mbedtls_x509_buf const *oid,
                            int critical,
                            const unsigned char *p,
                            const unsigned char *end)
{
    (void)p_ctx; (void)crt; (void)oid; (void)critical; (void)p; (void)end;
    return 0;
}

int gnrc_send_load_crt(const char *path, void *buf, size_t size, gnrc_send_x509_crt_t *cert)
{
    int ret = 0;
    if (path && (ret = vfs_file_to_buffer(path, buf, size)) < 0) {
        return ret;
    }
    if (mbedtls_x509_crt_parse_der_with_ext_cb(&cert->cert, buf, size,
                                               0, _x509_crt_ext_cb, NULL) < 0) {
        return -EBADMSG;
    }
    return ret;
}

int gnrc_send_load_pubkey(const char *path, void *buf, size_t size)
{
    return vfs_file_to_buffer(path, buf, size);
}

int gnrc_send_load_key(const char *path, void *buf, size_t size)
{
    return vfs_file_to_buffer(path, buf, size);
}

int gnrc_send_parse_keys(const void *buf, size_t size, gnrc_send_pk_t *pk)
{
    int ret;
    mbedtls_pk_init(&pk->pk);
    if ((ret = mbedtls_pk_parse_key(&pk->pk, buf, size, NULL, 0))) {
        return ret;
    }
    return GNRC_SEND_STATUS_OK;
}

int gnrc_send_load_ta(const char *path, gnrc_send_ta_t *ta, void *ta_buf, size_t ta_buf_size, unsigned max)
{
    int ret, plen;
    vfs_DIR dir;
    vfs_dirent_t entry;
    char path_buf[GNRC_SEND_PATH_MAX];
    unsigned m = max;
    if ((plen = strlen(path)) >= (int)sizeof(path_buf)) {
        return -ENOBUFS;
    }
    strcpy(path_buf, path);
    if ((ret = vfs_opendir(&dir, path_buf)) < 0) {
        return ret;
    }
    path_buf[plen++] = '/';
    while (m) {
        path_buf[plen] = '\0';
        if (vfs_readdir(&dir, &entry) <= 0) {
            break;
        }
        if (entry.d_name[0] == '.') {
            continue;
        }
        if ((sizeof(path_buf) - plen) <= strlen(entry.d_name)) {
            continue;
        }
        strcat(path_buf, entry.d_name);
        if ((ret = vfs_file_to_buffer(path_buf, ta_buf, ta_buf_size)) < 0) {
            continue;
        }
        mbedtls_x509_crt_init(&ta->cert.cert);
        if (mbedtls_x509_crt_parse_der_with_ext_cb(&ta->cert.cert, ta_buf, ret,
                                                   0, _x509_crt_ext_cb, NULL)) {
            mbedtls_x509_crt_free(&ta->cert.cert);
            continue;
        }
        ta->ta_size = ret;
        ta->pk = ta->cert.cert.pk_raw.p;
        ta->pk_size = ta->cert.cert.pk_raw.len;
        ta->name = ta->cert.cert.subject_raw.p;
        ta->name_size = ta->cert.cert.subject_raw.len;
        gnrc_send_ta_identifier(&ta->ta_ident, ta->name, ta->name_size);
        strcpy(ta->rel_path, path_buf + strlen(path) + 1);
        ta_buf = ((uint8_t *)ta_buf) + ret;
        ta_buf_size -= ret;
        ta++;
        m--;
    }
    ret = max - m;
    vfs_closedir(&dir);
    return ret;
}

int gnrc_send_load_cp(const char *path, gnrc_send_cp_t *cp, unsigned max,
                      gnrc_send_key_t *key, void *key_buf, size_t key_size, void *pk_buf, size_t pk_size)
{
    int ret, cp_len, rtr_len, ta_len;
    vfs_DIR cp_dir, rtr_dir, ta_dir;
    vfs_dirent_t cp_entry, rtr_entry, ta_entry;
    char path_buf[GNRC_SEND_PATH_MAX];
    unsigned m = max;
    memset(key, 0, sizeof(*key));
    if (((cp_len = strlen(path)) + 1) >= (int)sizeof(path_buf)) {
        return -ENOBUFS;
    }
    strcpy(path_buf, path);
    if ((ret = vfs_opendir(&cp_dir, path_buf)) < 0) {
        return ret;
    }
    path_buf[cp_len++] = '/';
    path_buf[cp_len] = '\0';
    while (m) {
        if (vfs_readdir(&cp_dir, &cp_entry) <= 0) {
            break;
        }
        if (cp_entry.d_name[0] == '.') {
            continue;
        }
        if ((sizeof(path_buf) - cp_len) <= strlen(cp_entry.d_name) + 1) {
            continue;
        }
        strcpy(&path_buf[cp_len], cp_entry.d_name);
        rtr_len = strlen(path_buf);
        path_buf[rtr_len++] = '/';
        path_buf[rtr_len] = '\0';
        if (vfs_opendir(&rtr_dir, path_buf) < 0) {
            continue;
        }
        while (m) {
            if (vfs_readdir(&rtr_dir, &rtr_entry) <= 0) {
                break;
            }
            if (rtr_entry.d_name[0] == '.') {
                continue;
            }
            if ((sizeof(path_buf) - rtr_len) <= strlen(rtr_entry.d_name) + 1 + strlen(GNRC_SEND_CP_RTR_CERT_NAME)) {
                continue;
            }
            strcpy(&path_buf[rtr_len], rtr_entry.d_name);
            ta_len = strlen(path_buf);
            path_buf[ta_len++] = '/';
            path_buf[ta_len] = '\0';
            if (vfs_opendir(&ta_dir, path_buf) < 0) {
                continue;
            }
            memset(cp, 0, sizeof(*cp));
            strcpy(cp->rel_path, path_buf + strlen(path) + 1);
            if ((cp->ta = gnrc_send_get_ta_by_fqdn(rtr_entry.d_name, strlen(rtr_entry.d_name)))) {
                cp->num_comp = 1; /* trust anchor counts as component */
                for (uint16_t i = 1; vfs_readdir(&ta_dir, &ta_entry) > 0; ) {
                    if (ta_entry.d_name[0] == '.') {
                        continue;
                    }
                    /* The n-component CP must be certificate files named 0.der to <n - 1>.der,
                       where 0.der is the router certificate and all others are intermediate certificates */
                    char comp_name[] = "65535.der";
                    size_t fmt = fmt_u16_dec(comp_name, i);
                    strcpy(&comp_name[fmt], ".der");
                    if (!strcmp(ta_entry.d_name, comp_name)) {
                        if (i == 1) {
                            size_t crt_size;
                            void *crt;
                            gnrc_send_x509_crt_t *x509 = gnrc_send_cert_acquire(&crt_size, &crt);
                            mbedtls_x509_crt_init(&x509->cert);
                            strcpy(&path_buf[ta_len], GNRC_SEND_CP_RTR_CERT_NAME);
                            if ((ret = vfs_file_to_buffer(path_buf, crt, crt_size)) > 0) {
                                if (!(ret = mbedtls_x509_crt_parse_der_with_ext_cb(&x509->cert, crt, ret,
                                                                                   0, _x509_crt_ext_cb, NULL))) {
                                    gnrc_send_cp_identifier(&cp->cp_ident, x509->cert.pk_raw.p, x509->cert.pk_raw.len);
                                }
                            }
                            mbedtls_x509_crt_free(&x509->cert);
                            gnrc_send_cert_release();
                            if (ret != 0) {
                                goto skip;
                            }
                        }
                        cp->num_comp++;
                        i++;
                    }
                    else if (!(strcmp(cp_entry.d_name, CONFIG_GNRC_SEND_CP_SELF_VFS_NAME))) {
                        if (!strcmp(ta_entry.d_name, CONFIG_GNRC_SEND_PK_VFS_NAME)) {
                            strcpy(&path_buf[ta_len], CONFIG_GNRC_SEND_PK_VFS_NAME);
                            key->pubkey = pk_buf;
                            key->pubkey_size = gnrc_send_load_pubkey(path_buf, pk_buf, pk_size);
                        }
                        else if (!strcmp(ta_entry.d_name, CONFIG_GNRC_SEND_KEY_VFS_NAME)) {
                            strcpy(&path_buf[ta_len], CONFIG_GNRC_SEND_KEY_VFS_NAME);
                            key->key = key_buf;
                            key->key_size = gnrc_send_load_key(path_buf, key_buf, key_size);
                            gnrc_send_parse_keys(key_buf, key->key_size, &key->pk);
                        }
                    }
                }
            }
            cp++;
            m--;
skip:
            vfs_closedir(&ta_dir);
        }
        vfs_closedir(&rtr_dir);
    }
    vfs_closedir(&cp_dir);
    return max - m;
}

void gnrc_send_init(void)
{
    gnrc_send_init_node();
    /* cast away const once as an exception here */
    extern gnrc_send_ident_t gnrc_send_self_ident;
    _compute_keyhash((void *)gnrc_send_self_ident.hash,
                     gnrc_send_get_pubkey_der(),
                     gnrc_send_get_pubkey_size_der());
}

int gnrc_send_cga_generate(gnrc_netif_ipv6_t *netif, ipv6_addr_t *dst, ipv6_cga_parameters_t *params)
{
    (void)netif;
    return ipv6_cga_generate(dst, params,
                             gnrc_send_get_pubkey_der(),
                             gnrc_send_get_pubkey_size_der(),
                             CONFIG_GNRC_SEND_CGA_SEC,
                             params->collision_count);
}

int gnrc_send_enable_iface(gnrc_netif_ipv6_t *netif)
{
    (void)netif;
    if (!gnrc_send_get_pubkey_size_der()) {
        return -ENOTSUP;
    }
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

void gnrc_send_cp_identifier(gnrc_send_ident_t *ident, const void *pk, size_t pk_size)
{
    _compute_keyhash(ident, pk, pk_size);
}

void gnrc_send_ta_identifier(gnrc_send_ident_t *ident, const void *name, size_t name_size)
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
gnrc_pktsnip_t *gnrc_ndp_opt_certificate_build(void *cert, size_t cert_size,
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
    cs = inet_csum(cs, (const uint8_t *)&u_hdr.hdr.icmp_hdr, sizeof(u_hdr.hdr.icmp_hdr));
    assert(icmpv6->size >= sizeof(icmpv6_hdr_t));
    cs = inet_csum(cs, (const uint8_t *)(((icmpv6_hdr_t *)icmpv6->data) + 1),
                   icmpv6->size - sizeof(icmpv6_hdr_t));
    while (icmpv6->next) {
        icmpv6 = icmpv6->next;
        cs = inet_csum(cs, icmpv6->data, icmpv6->size);
    }
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

    if ((ret = _compute_signature(sig, &sig_size, src, dst, icmpv6,
                                  _icmp_checksum(icmpv6, gnrc_pkt_len(icmpv6), src, dst)))) {
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
            return ret;
        }
        if ((ret = mbedtls_ecdsa_from_keypair(&ctx_verify, mbedtls_pk_ec(pk_ctx)))) {
            mbedtls_pk_free(&pk_ctx);
            mbedtls_ecdsa_free(&ctx_verify);
            return ret;
        }
        mbedtls_pk_free(&pk_ctx);
    }
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
    uint8_t keyhash[SHA1_DIGEST_LENGTH];
    if (!pk ||
        _compute_keyhash(keyhash, pk, pk_size) ||
        memcmp(sig->key_hash, keyhash, sizeof(sig->key_hash))) {
        return -GNRC_SEND_STATUS_NO_KEY;
    }
    if ((uintptr_t)sig <= (uintptr_t)icmpv6 ||
        (uintptr_t)sig - (uintptr_t)icmpv6 < sizeof(icmpv6_hdr_t)) {
        return -GNRC_SEND_STATUS_BAD_FORMAT;
    }
    gnrc_pktsnip_t icmpv6_snip = {
        .data = (void *)icmpv6,
        .size = (uintptr_t)sig - (uintptr_t)icmpv6,
    };
    if ((ret = _verify_signature(icmpv6, sig, sig_max_size, pk, pk_size, src, dst,
                                 _icmp_checksum(&icmpv6_snip, icmpv6_snip.size, src, dst)))) {
        return ret;
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
    char buf[1024];
    (void)data; (void)depth;
    mbedtls_x509_crt_info(buf, sizeof(buf) - 1, "", crt);
    DEBUG_GNRC_SEND_CRYPTO("%s",buf);
    if (*flags == 0) {
        DEBUG_GNRC_SEND_CRYPTO("This certificate has no flags\n");
    }
    else {
        mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", *flags);
        DEBUG_GNRC_SEND_CRYPTO("%s\n",buf);
    }
    return 0;
}

int gnrc_send_cert_verify(gnrc_send_x509_crt_t *crt,
                          gnrc_send_x509_crt_t *trust)
{
    mbedtls_x509_crl cacrl = { 0 };
    uint32_t flags;
    return mbedtls_x509_crt_verify(&crt->cert, &trust->cert, &cacrl,
                                   NULL, &flags, _x509_verify_cb, NULL);
}

int gnrc_send_cp_sol_send(gnrc_netif_t *netif,
                          const ipv6_addr_t *src, const ipv6_addr_t *dst,
                          uint16_t comp, const ipv6_hdr_t *rtr_ipv6,
                          const gnrc_send_ident_t *rtr_ident,
                          const ndp_rtr_adv_t *rtr_adv, size_t rtr_adv_len,
                          void **cps_ctx)
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
    gnrc_pktsnip_t *l2hdr = gnrc_netif_hdr_build(NULL, 0, NULL, 0);
    if (!l2hdr) {
        goto release;
    }
    pkt = l2hdr = gnrc_pkt_prepend(pkt, l2hdr);
    if (!*cps_ctx) {
        if (!(*cps_ctx = gnrc_send_get_cps_ctx(netif, rtr_ipv6, rtr_ident, rtr_adv, rtr_adv_len))) {
            goto release;
        }
    }
    ((ndp_cp_sol_t *)cp_sol->data)->ident = byteorder_htons((*(gnrc_send_cache_cps_t **)cps_ctx)->identifier);
    gnrc_netif_hdr_set_netif(l2hdr->data, netif);
    if (gnrc_netapi_dispatch_send(GNRC_NETTYPE_NDP, GNRC_NETREG_DEMUX_CTX_ALL, l2hdr) == 0) {
        DEBUG_GNRC_SEND("unable to dispatch Certificate Path Solicitation\n");
        gnrc_pktbuf_release(l2hdr);
        return -ECANCELED;
    }
    DEBUG_GNRC_SEND("sent CPS ctx=%p\n", *cps_ctx);
    return GNRC_SEND_STATUS_OK;
release:
    gnrc_pktbuf_release(pkt);
    return -ENOMEM;
}

int gnrc_send_cp_adv_send(gnrc_netif_t *netif,
                          const ipv6_addr_t *src, const ipv6_addr_t *dst,
                          uint16_t comp, uint16_t ident, const gnrc_send_cp_t *cp,
                          void **cpa_ctx,
                          gnrc_pktsnip_t *ext_opts)
{
    assert(cpa_ctx);
    gnrc_pktsnip_t *hdr, *pkt = ext_opts;
    if (!src && !(src = gnrc_netif_ipv6_addr_best_src(netif, dst, true))) {
        return -EADDRNOTAVAIL;
    }
    if (cp) {
        if (comp == SEND_CPS_ALL_COMP) {
            comp = cp->num_comp - 1;
        }
        if (comp >= cp->num_comp) {
            return -EBADMSG;
        }
        /* add certificate option */
        char path[GNRC_SEND_PATH_MAX] = { 0 };
        if (sizeof(path) <= strlen(CONFIG_GNRC_SEND_CP_ROOT"/")+
                            strlen(cp->rel_path) +
                            strlen("65535.der")) {
            return -ENOBUFS;

        }
        strcat(path, CONFIG_GNRC_SEND_CP_ROOT"/");
        strcat(path, cp->rel_path);
        fmt_u16_dec(path + strlen(path), comp);
        strcat(path, ".der");
        struct stat st;
        if (vfs_stat(path, &st) < 0) {
            return -ENOENT;
        }
        /* add TA option if the first component was requested */
        if (comp == cp->num_comp - 1) {
            if (!(hdr = gnrc_ndp_opt_trust_anchor_build(cp->ta->name, cp->ta->name_size, NDP_TA_TYPE_DER, pkt))) {
                goto release;
            }
            pkt = hdr;
        }
        if (!(hdr = gnrc_ndp_opt_certificate_build(NULL, st.st_size, NDP_CERT_TYPE_DER, pkt))) {
            goto release;
        }
        pkt = hdr;
        if (vfs_file_to_buffer(path, ((ndp_opt_cert_t *)pkt->data) + 1, pkt->size - sizeof(ndp_opt_cert_t)) < 0) {
            goto release;
        }
    }
    if (!(hdr = gnrc_ndp_cp_adv_build(ident, comp, cp->num_comp, pkt))) {
        goto release;
    }
    pkt = hdr;
    if (!(hdr = gnrc_ipv6_hdr_build(pkt, src, dst))) {
        goto release;
    }
    ((ipv6_hdr_t *)hdr->data)->hl = NDP_HOP_LIMIT;
    pkt = hdr;
    gnrc_pktsnip_t *l2hdr = gnrc_netif_hdr_build(NULL, 0, NULL, 0);
    if (!l2hdr) {
        goto release;
    }
    pkt = l2hdr = gnrc_pkt_prepend(pkt, l2hdr);
    gnrc_netif_acquire(netif);
    if (!*cpa_ctx) {
        if (netif->ipv6.send_ctx.cpa_ctx) {
            gnrc_netif_release(netif);
            goto release;
        }
        if (!(*cpa_ctx = gnrc_send_get_cpa_ctx(netif, dst, ident, cp))) {
            gnrc_netif_release(netif);
            goto release;
        }
        netif->ipv6.send_ctx.cpa_ctx = *cpa_ctx;
    }
    gnrc_netif_release(netif);
    gnrc_netif_hdr_set_netif(l2hdr->data, netif);
    if (gnrc_netapi_dispatch_send(GNRC_NETTYPE_NDP, GNRC_NETREG_DEMUX_CTX_ALL, l2hdr) == 0) {
        DEBUG_GNRC_SEND("unable to dispatch Certificate Path Advertisement\n");
        gnrc_pktbuf_release(l2hdr);
        return -ECANCELED;
    }
    DEBUG_GNRC_SEND("sent CPA ctx=%p\n", netif->ipv6.send_ctx.cpa_ctx);
    return GNRC_SEND_STATUS_OK;
release:
    gnrc_pktbuf_release(pkt);
    return -ENOMEM;
}
/** @} */
