/*
 * Copyright (C) 2020 Otto-von-Gericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_ieee802154 IEEE802.15.4
 * @{
 *
 * @file
 * @brief       IEEE 802.15.4 security interface
 *
 * Specification: IEEE802154 - 2015
 * https://www.silabs.com/content/usergenerated/asi/cloud/attachments/siliconlabs/en/community/wireless/proprietary/forum/jcr:content/content/primary/qna/802_15_4_promiscuous-tbzR/hivukadin_vukadi-iTXQ/802.15.4-2015.pdf
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */

#ifndef IEEE802154_SECURITY
#define IEEE802154_SECURITY

#include <stdint.h>
#include "ieee802154.h"
#include "crypto/ciphers.h"

/**
 * @brief   AES key that is used in the test vectors from the specification
 */
#define IEEE802154_DEFAULT_KEY                  { 0xc0, 0xc1, 0xc2, 0xc3,   \
                                                  0xc4, 0xc5, 0xc6, 0xc7,   \
                                                  0xc8, 0xc9, 0xca, 0xcb,   \
                                                  0xcc, 0xcd, 0xce, 0xcf }
/**
 * @brief   Length of an AES key in bytes
 */
#define IEEE802154_SEC_KEY_LENGTH               (16U)

/**
 * @brief   Block size of an encryption block
 */
#define IEEE802154_SEC_BLOCK_SIZE               (16U)

/**
 * @brief   Maximum length of the security auxiliary header in bytes
 */
#define IEEE802154_MAX_AUX_HDR_LEN              (14U)

/**
 * @brief   Size of IEEE 802.15.4 MAC
 */
#define IEEE802154_MAC_SIZE                     (16U)

/**
 * @name Flag field of CCM input block
 *
 *   Bit 7    Bit6       Bit 5 - Bit 3            Bit2 - Bit 0
 * +--------+-------+-----------------------+-----------------------+
 * | 0 (r)  | Adata |          M            |          L            |
 * +--------+-------+-----------------------+-----------------------+
 *
 * r: value reserved
 * Adata: 0 if no MIC is present, 1 else
 * M: Number of octets in authentication field      (M-2)/2
 * L: Number of octets in length field              L-1
 *
 * L will actually always be 2 because the maximul message length is 127
 * which is expressed as two bytes.
 * Valid values for M are 0 (No MIC), 4 (32 bit MIC), 8 (64 bit MIC)
 * and 16 (128 bit MIC)
 */
#define IEEE802154_CCM_FLAG(M, L)                   \
    ((((M) >= 4) ? (1 << 6) : 0)                |   \
    (((M) >= 4) ? ((((M) - 2) / 2) << 3) : 0)   |   \
    ((L) - 1))
/** @} */

/**
 * @brief   Mask to get security level bits
 */
#define IEEE802154_SCF_SECLEVEL_MASK            (0x07)

/**
 * @brief   Number of shifts to set/get security level bits
 */
#define IEEE802154_SCF_SECLEVEL_SHIFT           (0)

/**
 * @brief   Mask to get key mode bits
 */
#define IEEE802154_SCF_KEYMODE_MASK             (0x18)

/**
 * @brief   Number of shifts to set/get key mode bits
 */
#define IEEE802154_SCF_KEYMODE_SHIFT            (3)

/**
 * @name    Security levels
 */
#define IEEE802154_SCF_SECLEVEL_NONE            (0x00)  /**< no security */
#define IEEE802154_SCF_SECLEVEL_MIC32           (0x01)  /**< 32 bit MIC */
#define IEEE802154_SCF_SECLEVEL_MIC64           (0x02)  /**< 64 bit MIC */
#define IEEE802154_SCF_SECLEVEL_MIC128          (0x03)  /**< 128 bit MIC */
#define IEEE802154_SCF_SECLEVEL_ENC             (0x04)  /**< encryption */
#define IEEE802154_SCF_SECLEVEL_ENC_MIC32       (0x05)  /**< enc. + 32 bit MIC */
#define IEEE802154_SCF_SECLEVEL_ENC_MIC64       (0x06)  /**< enc. + 64 bit MIC (mandatory) */
#define IEEE802154_SCF_SECLEVEL_ENC_MIC128      (0x07)  /**< enc. + 128 bit MIC */
/** @} */

/**
 * @name   Key identifier modes
 */
#define IEEE802154_SCF_KEYMODE_IMPLICIT         (0x00)  /**< Key is determined implicitly */
#define IEEE802154_SCF_KEYMODE_INDEX            (0x01)  /**< Key is determined from key index */
#define IEEE802154_SCF_KEYMODE_SHORT_INDEX      (0x02)  /**< Key is determined from 4 byte key source and key index */
#define IEEE802154_SCF_KEYMODE_HW_INDEX         (0x03)  /**< Key is determined from 8 byte key source and key index */
/** @} */

/**
 * @brief   IEEE 802.15.4 security error codes
 */
typedef enum {
    IEEE802154_SEC_OK,
    IEEE802154_SEC_FRAME_COUNTER_OVERFLOW,
    IEEE802154_SEC_NO_KEY,
    IEEE802154_SEC_MAC_CHECK_FAILURE,
    IEEE802154_SEC_UNSUPORTED,
    IEEE802154_SEC_FRAME_TOO_LARGE
} ieee802154_sec_error_t;

struct ieee802154_remote_sec;
struct ieee802154_sec_context;

/**
 * @brief   A block of 16 bytes
 *
 * AES works on blocks of 16 bytes
 */
typedef uint8_t block16_t[IEEE802154_SEC_BLOCK_SIZE];

/**
 * @brief   Struct to hold security information from connected
 *          IEEE 802.15.4 devices
 */
typedef struct ieee802154_remote_sec {
    /**
     * @brief   Next remote device
     */
    struct ieee802154_remote_sec *next;
    /**
     * @brief   Last frame counter received
     */
    uint32_t frame_counter;
    /**
     * @brief   PAN ID
     */
    uint16_t pan;
    /**
     * @brief   Short address
     */
    uint8_t short_addr[IEEE802154_SHORT_ADDRESS_LEN];
    /**
     * @brief   Long address
     */
    uint8_t long_addr[IEEE802154_LONG_ADDRESS_LEN];
    /**
     * @brief   Key to encrypt messages from that device
     */
    uint8_t key[IEEE802154_SEC_KEY_LENGTH];
} ieee802154_remote_sec_t;

typedef struct ieee802154_remote_sec ieee802154_remote_sec_t;

/**
 * @brief   Function to set the encryption/decryption key for the
 *          next cipher operation
 *
 * @param[in]       ctx         Pointer to IEEE 802.15.4 security context
 * @param[in]       key         Key to be used for the next cipher operation
 */
typedef void (*aes_set_key_fn)(struct ieee802154_sec_context *ctx,
                               const block16_t key);

/**
 * @brief   Function type to compute CBC-MAC
 *
 * @param[in]       ctx         Pointer to IEEE 802.15.4 security context
 * @param[in]       cipher      Output cipher blocks
 * @param[in, out]  iv          in: IV; out: computet MIC
 * @param[in]       plain       Input plain blocks
 * @param[in]       nblocks     Number of blocks
 */
typedef void (*aes_cbc_fn)(struct ieee802154_sec_context *ctx,
                           block16_t *cipher,
                           block16_t iv,
                           const block16_t *plain,
                           uint8_t nblocks);

/**
 * @brief   Function type to perform ECB encryption
 *
 * @param[in]       ctx         Pointer to IEEE 802.15.4 security context
 * @param[out]      cipher      Output cipher blocks
 * @param[in]       plain       Input plain blocks
 * @param[in]       nblocks     Number of blocks
 */
typedef void (*aes_ecb_fn)(struct ieee802154_sec_context *ctx,
                           block16_t *cipher,
                           const block16_t *plain,
                           uint8_t nblocks);

/**
 * @brief   Struct of security operations
 */
typedef struct ieee802154_cipher_ops {
    /**
     * @brief   Function to set the cipher key
     */
    aes_set_key_fn set_key;
    /**
     * @brief   CBC block cipher function
     */
    aes_cbc_fn cbc;
    /**
     * @brief   ECB block cipher function
     */
    aes_ecb_fn ecb;
} ieee802154_cipher_ops_t;

/**
 * @brief   Struct to hold IEEE 802.15.4 security information
 */
typedef struct ieee802154_sec_context {
    /**
     * @brief Cipher context with AES128 interface and key storage
     */
    cipher_t cipher;
    /**
     * @brief   Security level IEEE802154_SCF_SECLEVEL_*
     */
    uint8_t security_level;
    /**
     * @brief   Key mode IEEE802154_SCF_KEYMODE_*
     */
    uint8_t key_id_mode;
    /**
     * @brief   Key index
     */
    uint8_t key_index;
    /**
     * @brief   Key source
     *
     * Content depends on key_id_mode
     */
    uint8_t key_source[IEEE802154_LONG_ADDRESS_LEN];
    /**
     * @brief   Own frame counter
     */
    uint32_t frame_counter;
    /**
     * @brief   List of remote security information
     *
     * @note    This member is currently unused because it is assumed that
     *          everybody has the same key
     */
    ieee802154_remote_sec_t *rem_devs;
    /**
     * @brief   Security operations
     */
    const ieee802154_cipher_ops_t *cipher_ops;
} ieee802154_sec_context_t;

/**
 * @brief   IEEE 802.15.4 auxiliary security header
 */
typedef struct __attribute__((packed)) {
    /**
     * @brief   Security Control field (SCF)
     *
     *    Bit 7    Bit 6    Bit 5    Bit 4    Bit 3    Bit 2    Bit 1    Bit 0
     *  +--------+--------+--------+--------+--------+--------+--------+--------+
     *  | security level  | key id. mode    | fc sup.| ASN    |   r    |   r    |
     *  +--------+--------+--------+--------+--------+--------+--------+--------+
     *
     * security level:
     * one of IEEE802154_SCF_SECLEVEL_*
     * key identifier mode:
     * one of IEEE802154_SCF_KEY_*
     * frame counter suppression:
     * basically always zero because we do not support TSCH right now
     * ASN:
     * basically always zero because we do not support TSCG right now
     */
    uint8_t scf;
    /**
     * @brief   frame counter
     */
    uint32_t fc;
    /**
     * @brief   key identifier (0 - 9 bytes) according to key id. mode
     */
    uint8_t key_id[];
} ieee802154_aux_sec_t;

/**
 * @brief   Contet of key_source if key mode is IEEE802154_SCF_KEYMODE_INDEX
 */
typedef struct __attribute__((packed)) {
    /**
     * @brief   Key index of key from originator, defined by key source
     */
    uint8_t key_index;
} ieee802154_aux_sec_key_identifier_1_t;

/**
 * @brief   Contet of key_source if key mode is IEEE802154_SCF_KEYMODE_SHORT_INDEX
 */
typedef struct __attribute__((packed)) {
    /**
     * @brief   macPANId concatinated with macShortAddress
     */
    uint8_t key_source[4];
    /**
     * @brief   Key index of key from originator, defined by key source
     */
    uint8_t key_index;
} ieee802154_aux_sec_key_identifier_5_t;

/**
 * @brief   Contet of key_source if key mode is IEEE802154_SCF_KEYMODE_HW_INDEX
 */
typedef struct __attribute__((packed)) {
    /**
     * @brief   macExtendedAddress
     */
    uint8_t key_source[IEEE802154_LONG_ADDRESS_LEN];
    /**
     * @brief   Key index of key from originator, defined by key source
     */
    uint8_t key_index;
} ieee802154_aux_sec_key_identifier_9_t;

/**
 * @brief   Format of 13 byte nonce
 */
typedef struct __attribute__((packed)) {
    /**
     * @brief   Source long address
     */
    uint8_t src_addr[IEEE802154_LONG_ADDRESS_LEN];
    /**
     * @brief   Frame counter
     */
    uint32_t frame_counter;
    /**
     * @brief   One of IEEE802154_SCF_SECLEVEL_*
     */
    uint8_t security_level;
} ieee802154_ccm_nonce_t;

/**
 * @brief   Format of 16 byte input block to perform encryption
 */
typedef struct __attribute__((packed)) {
    /**
     * @brief   Flags field, constrcted with IEEE802154_CCM_FLAG
     */
    uint8_t flags;
    /**
     * @brief   Nonce (Number that is only used once)
     */
    ieee802154_ccm_nonce_t nonce;
    /**
     * @brief   Either the length of the actual message (for CBC-MAC) or
     *          a block counter (for CTR)
     */
    uint16_t counter;
} ieee802154_ccm_block_t;

/**
 * @brief   Initialize IEEE 802.15.4 security context with default values
 *
 * @param[out]      ctx                     security context
 */
void ieee802154_sec_init(ieee802154_sec_context_t *ctx);

/**
 * @brief   Encrypt IEEE 802.15.4 frame according to @p ctx
 *
 * @param[in]       ctx                     IEEE 802.15.4 security context
 * @param[in]       header                  Pointer to frame header
 * @param[in]       header_size             Size of header
 * @param[out]      auxiliary_header        Pointer to buffer to store aux. header
 * @param[out]      auxiliary_header_size   Size of aux. header
 * @param[in,out]   payload                 in: plain payload; out: encrypted payload
 * @param[in]       payload_size            Size of payload
 * @param[out]      mic                     Buffer to store computed MIC
 * @param[out]      mic_size                Size of MIC
 * @param[in]       src_address             Source address
 *
 * @return          0 Success
 * @return          negative integer on error
 */
int ieee802154_sec_encrypt_frame(ieee802154_sec_context_t *ctx,
                                 const uint8_t *header,
                                 uint8_t header_size,
                                 uint8_t *auxiliary_header,
                                 uint8_t *auxiliary_header_size,
                                 uint8_t *payload,
                                 uint8_t payload_size,
                                 uint8_t *mic,
                                 uint8_t *mic_size,
                                 const uint8_t *src_address);

/**
 * @brief   Decrypt IEEE 802.15.4 frame according to @p ctx
 *
 * @param[in]       ctx                     IEEE 802.15.4 security context
 * @param[in]       frame_size              Size of received frame
 * @param[in]       header                  Poinzter to header, which is also the frame
 * @param[in]       header_size             Size of header
 * @param[out]      auxiliary_header        Will point to the beginning of the auxiliary header
 * @param[out]      auxiliary_header_size   Pointer to store the size of the axiliary header
 * @param[out]      payload                 Will point to the beginning of the payload
 * @param[out]      payload_size            Pointer to store the payload size
 * @param[out]      mic                     Will point to the beginning of the MIC
 * @param[out]      mic_size                Pointer to store the size of the MIC
 * @param[in]       src_address             Pointer to remote long source address
 *
 * @return          0 Success
 * @return          negative integer on error
 */
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
                                 const uint8_t *src_address);

#endif /* IEEE802154_SECURITY */
/** @} */
