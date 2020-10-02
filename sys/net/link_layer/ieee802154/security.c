
/*
 * Copyright (C) 2020 Otto-von-Guericke-Universität Magdeburg
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

#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "net/ieee802154_security.h"

static inline uint8_t _min(uint8_t a, uint8_t b)
{
    return a < b ? a : b;
}

static inline uint8_t _get_sec_level(uint8_t scf)
{
    return (scf & IEEE802154_SCF_SECLEVEL_MASK)
           >> IEEE802154_SCF_SECLEVEL_SHIFT;
}

static inline uint8_t _get_key_id_mode(uint8_t scf)
{
    return (scf & IEEE802154_SCF_KEYMODE_MASK)
           >> IEEE802154_SCF_KEYMODE_SHIFT;
}

static inline uint8_t _mac_size(uint8_t sec_level)
{
    switch (sec_level) {
        case IEEE802154_SCF_SECLEVEL_MIC32:
        case IEEE802154_SCF_SECLEVEL_ENC_MIC32:
            return 4;
        case IEEE802154_SCF_SECLEVEL_MIC64:
        case IEEE802154_SCF_SECLEVEL_ENC_MIC64:
            return 8;
        case IEEE802154_SCF_SECLEVEL_MIC128:
        case IEEE802154_SCF_SECLEVEL_ENC_MIC128:
            return 16;
        default:
            return 0;
    }
}

static inline bool _req_mac(uint8_t sec_level)
{
    switch (sec_level) {
        case IEEE802154_SCF_SECLEVEL_MIC32:
        case IEEE802154_SCF_SECLEVEL_MIC64:
        case IEEE802154_SCF_SECLEVEL_MIC128:
        case IEEE802154_SCF_SECLEVEL_ENC_MIC32:
        case IEEE802154_SCF_SECLEVEL_ENC_MIC64:
        case IEEE802154_SCF_SECLEVEL_ENC_MIC128:
            return true;
        default:
            return false;
    }
}

static inline void _memxor(void *dst, const void* src, size_t size)
{
    while (size--) {
        ((uint8_t *)dst)[size] ^= ((uint8_t *)src)[size];
    }
}

static inline uint8_t _scf(uint8_t sec_level, uint8_t key_mode)
{
    return (sec_level << IEEE802154_SCF_SECLEVEL_SHIFT) |
           (key_mode << IEEE802154_SCF_KEYMODE_SHIFT);
}

static inline uint8_t _get_aux_hdr_size(uint8_t security_level,
                                        uint8_t key_mode)
{
    if (security_level == IEEE802154_SCF_SECLEVEL_NONE) {
            return 0;
    }
    switch (key_mode) {
        case IEEE802154_SCF_KEYMODE_IMPLICIT:
            return 5;
        case IEEE802154_SCF_KEYMODE_INDEX:
            return 6;
        case IEEE802154_SCF_KEYMODE_SHORT_INDEX:
            return 10;
        case IEEE802154_SCF_KEYMODE_HW_INDEX:
            return 14;
        default:
            return 0;
    }
}

static uint8_t _set_aux_hdr(const ieee802154_sec_context_t *ctx,
                            ieee802154_aux_sec_t *ahr)
{
    ahr->scf = _scf(ctx->security_level, ctx->key_id_mode);
    /* If you look in the specification: Annex C,
       integers values are in little endian ?! */
    ahr->fc = byteorder_btoll(byteorder_htonl(ctx->frame_counter)).u32;
    size_t len = 5;
    switch (ctx->key_id_mode) {
        case IEEE802154_SCF_KEYMODE_IMPLICIT:
            break;
        case IEEE802154_SCF_KEYMODE_INDEX:
            memcpy(ahr->key_id, &ctx->key_index, 1);
            len++;
            break;
        case IEEE802154_SCF_KEYMODE_SHORT_INDEX:
            memcpy(ahr->key_id, ctx->key_source, 4);
            memcpy(ahr->key_id + 4, &ctx->key_index, 1);
            len += 5;
            break;
        case IEEE802154_SCF_KEYMODE_HW_INDEX:
            memcpy(ahr->key_id, ctx->key_source, 8);
            memcpy(ahr->key_id + 4, &ctx->key_index, 1);
            len += 9;
            break;
        default:
            break;
    }
    return len;
}

static const uint8_t *_get_encryption_key(const ieee802154_sec_context_t *ctx,
                                          const uint8_t *mhr,
                                          uint8_t mhr_len,
                                          const ieee802154_aux_sec_t *ahr)
{
    (void)mhr;
    (void)mhr_len;
    (void)ahr;
    /* For simplicity, assume that everyone has the same key */
    /* Else you´d have to look up the key based on the destination address */
    return ctx->cipher.context.context;
}

static const uint8_t *_get_decryption_key(const ieee802154_sec_context_t *ctx,
                                          const uint8_t *mhr,
                                          uint8_t mhr_len,
                                          const ieee802154_aux_sec_t *ahr)
{
    (void)mhr;
    (void)mhr_len;
    (void)ahr;
    /* For simplicity, assume that everyone has the same key */
    /* Else you´d have to look up the key based on the source address */
    return ctx->cipher.context.context;
}

static uint8_t _ecb(ieee802154_sec_context_t *ctx,
                    uint8_t *tmp1, uint8_t *tmp2, uint8_t *data,
                    const uint8_t *Ai, uint8_t size)
{
    uint8_t s = _min(sizeof(block16_t), size);
    ctx->cipher_ops->ecb(ctx, (block16_t *)tmp2, (block16_t *)Ai, 1);
    memcpy(tmp1, data, s);
    memset(tmp1 + s, 0, sizeof(block16_t) - s);
    _memxor(tmp1, tmp2, sizeof(block16_t));
    memcpy(data, tmp1, s);
    return s;
}

static uint8_t _cbc_next(ieee802154_sec_context_t *ctx,
                         uint8_t *last, uint8_t *tmp,
                         const uint8_t *next, uint8_t size)
{
    uint8_t s = _min(sizeof(block16_t), size);
    memcpy(tmp, next, s);
    memset(tmp + s, 0, sizeof(block16_t) - s);
    ctx->cipher_ops->cbc(ctx, (block16_t *)last, last, (block16_t *)tmp, 1);
    return s;
}

static void _set_key(ieee802154_sec_context_t *ctx, const block16_t key) {
    ctx->cipher_ops->set_key(ctx, key);
    memcpy(ctx->cipher.context.context, key, IEEE802154_SEC_KEY_LENGTH);
}

void ieee802154_sec_init(ieee802154_sec_context_t *ctx)
{
    assert(ctx->cipher_ops);
    /* MIC64 is the only mandatory security mode */
    ctx->security_level = IEEE802154_SCF_SECLEVEL_ENC_MIC64;
    ctx->key_id_mode = IEEE802154_SCF_KEYMODE_IMPLICIT;
    memset(ctx->key_source, 0, sizeof(ctx->key_source));
    ctx->key_index = 0;
    ctx->frame_counter = 0;
    uint8_t key[] = IEEE802154_DEFAULT_KEY;
    ctx->rem_devs = NULL;

    assert(CIPHER_MAX_CONTEXT_SIZE >= IEEE802154_SEC_KEY_LENGTH);
    cipher_init(&ctx->cipher, CIPHER_AES_128, key, IEEE802154_SEC_KEY_LENGTH);
}

int ieee802154_sec_encrypt_frame(ieee802154_sec_context_t *ctx,
                                 const uint8_t *header,
                                 uint8_t header_size,
                                 uint8_t *auxiliary_header,
                                 uint8_t *auxiliary_header_size,
                                 uint8_t *payload,
                                 uint8_t payload_size,
                                 uint8_t *mic,
                                 uint8_t *mic_size,
                                 const uint8_t *src_address)
{
    /* For non data frames (MAC commands, beacons) a_len would be larger.
       ACKs are not encrypted. */
    assert((*((uint8_t *)header)) & IEEE802154_FCF_TYPE_DATA);

    if (ctx->security_level == IEEE802154_SCF_SECLEVEL_NONE) {
        *auxiliary_header_size = 0;
        *mic_size = 0;
        return 0;
    }
    if (ctx->frame_counter == 0xFFFFFFFF) {
        return -IEEE802154_SEC_FRAME_COUNTER_OVERFLOW;
    }
    if (!ctx->cipher_ops->ecb ||
        (_req_mac(ctx->security_level) && !(ctx->cipher_ops->cbc))) {
        return -IEEE802154_SEC_UNSUPORTED;
    }
    if (header_size + payload_size >
        (IEEE802154_FRAME_LEN_MAX -
        IEEE802154_MAX_AUX_HDR_LEN -
        IEEE802154_FCS_LEN)) {
        return -IEEE802154_SEC_FRAME_TOO_LARGE;
    }

    /* for reference, check the examples in the specification */
    /* In the linux kernel: net/mac802154/llsec.c may also be an interesting reference */
    ieee802154_aux_sec_t *aux = (ieee802154_aux_sec_t *)auxiliary_header;
    *auxiliary_header_size = _get_aux_hdr_size(ctx->security_level,
                                               ctx->key_id_mode);
    _set_aux_hdr(ctx, aux);
    const uint8_t *key;
    if (!(key = _get_encryption_key(ctx, header, header_size, aux))) {
        return -IEEE802154_SEC_NO_KEY;
    }
    _set_key(ctx, key);

    *mic_size = _mac_size(ctx->security_level);
    uint16_t a_len = header_size + *auxiliary_header_size;
    uint16_t m_len = payload_size;
    uint8_t block_buffer1[IEEE802154_SEC_BLOCK_SIZE] = { 0 };
    uint8_t block_buffer2[IEEE802154_SEC_BLOCK_SIZE] = { 0 };
    uint8_t *m = payload;
    uint8_t size, offset, blk;
    ieee802154_ccm_block_t Ai;
    Ai.flags = IEEE802154_CCM_FLAG(0, 2);
    Ai.nonce.frame_counter = htonl(ctx->frame_counter);
    Ai.nonce.security_level = ctx->security_level;
    Ai.counter = 0;
    memcpy(Ai.nonce.src_addr, src_address, IEEE802154_LONG_ADDRESS_LEN);
    *((uint16_t *)block_buffer1) = htons(a_len);
    uint8_t hoff = _min(sizeof(block_buffer1) - sizeof(uint16_t), header_size);
    memcpy(block_buffer1 + sizeof(uint16_t), header, hoff);
    uint8_t ahoff = _min(sizeof(block_buffer1) - sizeof(uint16_t) - hoff,
                         *auxiliary_header_size);
    memcpy(block_buffer1 + sizeof(uint16_t) + hoff, auxiliary_header, ahoff);
    offset = hoff + ahoff;
    if (_req_mac(ctx->security_level)) {
        ieee802154_ccm_block_t *B0 = &Ai;
        B0->flags = IEEE802154_CCM_FLAG(*mic_size, 2);
        B0->counter = htons(m_len);
        ctx->cipher_ops->cbc(ctx, (block16_t *)block_buffer2, block_buffer2, (block16_t *)B0, 1);
        _cbc_next(ctx, block_buffer2, block_buffer1, block_buffer1,
                  sizeof(uint16_t) + offset);
        for (blk = 2; offset < a_len; blk++) {
            uint8_t hsize = _min(sizeof(block_buffer1), header_size - hoff);
            memcpy(block_buffer1, &header[hoff], hsize);
            hoff += hsize;
            uint8_t ahsize = _min(sizeof(block_buffer1) - hsize,
                                  *auxiliary_header_size - ahoff);
            memcpy(block_buffer1 + hsize, &auxiliary_header[ahoff], ahsize);
            ahoff += ahsize;
            offset = hoff + ahoff;
            size = hsize + ahsize;
            _cbc_next(ctx, block_buffer2, block_buffer1,
                      block_buffer1, size);
        }
        for(offset = 0; offset < m_len; blk++) {
            offset += _cbc_next(ctx, block_buffer2, block_buffer1,
                                &m[offset], m_len - offset);
        }
        memcpy(mic, block_buffer2, *mic_size);
        Ai.flags = IEEE802154_CCM_FLAG(0, 2);
        Ai.counter = 0;
        _ecb(ctx, block_buffer1, block_buffer2,
             mic, (uint8_t *)&Ai, *mic_size);
    }
    for (offset = 0, blk = 0; offset < m_len; blk++) {
        Ai.counter = htons(blk + 1);
        offset += _ecb(ctx, block_buffer1, block_buffer2,
                       &m[offset], (uint8_t *)&Ai, m_len - offset);
    }
    ctx->frame_counter++;
    return IEEE802154_SEC_OK;
}

int ieee802154_sec_decrypt_frame(ieee802154_sec_context_t *ctx,
                                 uint8_t frame_size,
                                 uint8_t *header,
                                 uint8_t header_size,
                                 uint8_t **auxiliary_header,
                                 uint8_t *auxiliary_header_size,
                                 uint8_t **payload,
                                 uint8_t *payload_size,
                                 uint8_t **mic,
                                 uint8_t *mic_size,
                                 const uint8_t *src_address)
{
    /* For non data frames (MAC commands, beacons) a_len would be larger.
       ACKs are not encrypted. */
    assert(*header & IEEE802154_FCF_TYPE_DATA);

    ieee802154_aux_sec_t *aux = (ieee802154_aux_sec_t *)(header + header_size);
    uint8_t security_level = _get_sec_level(aux->scf);
    uint8_t key_mode = _get_key_id_mode(aux->scf);
    uint8_t mac_size = _mac_size(security_level);

    if (security_level == IEEE802154_SCF_SECLEVEL_NONE) {
        *auxiliary_header = NULL;
        *auxiliary_header_size = 0;
        *payload = header + header_size;
        *payload_size = frame_size - header_size;
        *mic = NULL;
        *mic_size = 0;
        return IEEE802154_SEC_OK;
    }
    if (!ctx->cipher_ops->ecb ||
        (_req_mac(security_level) && !(ctx->cipher_ops->cbc))) {
        return -ENOTSUP;
    }
    *auxiliary_header_size = _get_aux_hdr_size(security_level, key_mode);
    *auxiliary_header = (uint8_t *)aux;
    *payload_size = frame_size - header_size -
                    *auxiliary_header_size - mac_size;
    *payload = header + header_size + *auxiliary_header_size;
    *mic_size = mac_size;
    *mic = header + frame_size - mac_size;

    uint8_t a_len = header_size + *auxiliary_header_size;
    uint8_t c_len = *payload_size;

    const uint8_t *key;
    if (!(key = _get_decryption_key(ctx, header, header_size, aux))) {
        return -IEEE802154_SEC_NO_KEY;
    }
    _set_key(ctx, key);

    /* for reference, see the examples in the specification */
    /* In the linux kernel: net/mac802154/llsec.c may also be an interesting reference */
    /* better decrypt block by block, than holding
       a large buffer because I had stack issues */
    uint8_t block_buffer1[IEEE802154_SEC_BLOCK_SIZE] = { 0 };
    uint8_t block_buffer2[IEEE802154_SEC_BLOCK_SIZE] = { 0 };
    uint8_t *c = *payload;
    uint8_t *mac = *mic;
    uint8_t size, offset, blk;
    ieee802154_ccm_block_t Ai;
    Ai.flags = IEEE802154_CCM_FLAG(0, 2);
    Ai.nonce.frame_counter = htonl(aux->fc);
    /* A better implementation would check if the received frame counter is
       greater then the frame counter that has previously been received from
       the other endpoint. This is done to protect against replay attacks.
       But we do not store this information because we also do not have
       a proper key store, to avoid complexity on embedded devices. */
    Ai.nonce.security_level = security_level;
    Ai.counter = 0;
    memcpy(Ai.nonce.src_addr, src_address, IEEE802154_LONG_ADDRESS_LEN);
    /* decrypt MIC */
    if (mac_size) {
        _ecb(ctx, block_buffer1, block_buffer2,
             mac, (uint8_t *)&Ai, mac_size);
    }
    /* decrypt cipher */
    for (offset = 0, blk = 0; offset < c_len; blk++) {
        Ai.counter = htons(blk + 1);
        offset += _ecb(ctx, block_buffer1, block_buffer2,
                       &c[offset], (uint8_t *)&Ai, c_len - offset);
    }
    /* check MIC */
    if (mac_size) {
        memset(block_buffer2, 0, sizeof(block_buffer2));
        Ai.flags = IEEE802154_CCM_FLAG(mac_size, 2);
        Ai.counter = htons(c_len);
        _cbc_next(ctx, block_buffer2, block_buffer1,
                  (uint8_t *)&Ai, sizeof(Ai));
        *((uint16_t *)block_buffer1) = htons(a_len);
        size = _min(sizeof(block_buffer1) - sizeof(uint16_t), a_len);
        memcpy(block_buffer1 + sizeof(uint16_t), header, size);
        memset(block_buffer1 + sizeof(uint16_t) + size, 0,
               sizeof(block_buffer1) - sizeof(uint16_t) - size);
        _memxor(block_buffer1, block_buffer2, sizeof(block_buffer1));
        memset(block_buffer2, 0, sizeof(block_buffer2));
        ctx->cipher_ops->cbc(ctx, (block16_t *)block_buffer2, block_buffer2, (block16_t *)block_buffer1, 1);
        for (offset = size; offset < a_len;) {
            offset += _cbc_next(ctx, block_buffer2,
                                block_buffer1, &header[offset],
                                a_len - offset);
        }
        for (offset = 0, blk = 0; offset < c_len; blk++) {
            offset += _cbc_next(ctx, block_buffer2,
                                block_buffer1, &c[offset],
                                c_len - offset);
        }
        if (memcmp(block_buffer2, *mic, mac_size)) {
            return -IEEE802154_SEC_MAC_CHECK_FAILURE;
        }
    }
    return IEEE802154_SEC_OK;
}
