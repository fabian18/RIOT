/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_send IPv6 SEcure Neighbor Discovery (SEND) RFC3971
 * @ingroup     net_ipv6
 * @brief       Provides IPv6 SEND data types and protocol constants
 * @{
 *
 * @file
 * @brief       IPv6 Secure Neighbor Discovery option types and constants
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */

#ifndef NET_SEND_H
#define NET_SEND_H

#include <stdint.h>

#include "time_units.h"
#include "net/ipv6/cga.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Configuration of SEND host constants
 * @{
 */
#if !defined(CONFIG_SEND_CPS_RETRY_MS) || defined(DOXYGEn)
/**
 * @brief   Initial certificate path solicitation retransmission delay
 *          (increases exponentially)
 *
 * The standard value of CPS_RETRY_MAX is 1s, but this is too impacient.
 */
#define CONFIG_SEND_CPS_RETRY_MS            (4u * MS_PER_SEC)
#endif
#if !defined(CONFIG_SEND_CPS_RETRY_FRAGMENTS_MS) || defined(DOXYGEN)
/**
 * @brief   After at least one certificate path component was received,
 *          delay after which missing components are requested with a new solicitation
 */
#define CONFIG_SEND_CPS_RETRY_FRAGMENTS_MS  (2u * MS_PER_SEC)
#endif
#if !defined(CONFIG_SEND_CPS_TRANSMIT_MAX) || defined(DOXYGEN)
/**
 * @brief How often a CP solicitation is transmitted before ADD fails
 */
#define CONFIG_SEND_CPS_TRANSMIT_MAX        (4u)
#endif
/** @} */

/**
 * @name    Configuration of SEND router constants
 * @{
 */
#if !defined(CONFIG_SEND_CPA_RATE_MAX) || defined(DOXYGEN)
/**
 * @brief   Maximum number of unicast certificate path advertisements per second
 *
 * The default value in RFC3971 is actually 10 but due to @ref cib_t
 * this must be a power of two.
 */
#define CONFIG_SEND_CPA_RATE_MAX            (8u) /* per sec */
#endif
/** @} */

#if !defined(CONFIG_SEND_NONCE_SIZE_MAX) || defined(DOXYGEN)
/**
 * @brief   Nonce size to be used for sent out nonces
 *
 * The nonce size must be 'x * 8 + 6 | x >= 0' to align with ICMPv6 options
 */
#define CONFIG_SEND_NONCE_SIZE_MAX          (6u) /* bytes */
#endif

/**
 * @name    SEND host constants
 * @{
 */
/**
 * @brief   @ref CONFIG_SEND_CPS_RETRY_MS
 */
#define SEND_CPS_RETRY_MS           CONFIG_SEND_CPS_RETRY_MS
/**
 * @brief @ref CONFIG_SEND_CPS_RETRY_FRAGMENTS_MS
 */
#define SEND_CPS_RETRY_FRAGMENTS_MS CONFIG_SEND_CPS_RETRY_FRAGMENTS_MS
/**
 * @brief   Maximum time in milliseconds it may take to retrieve a certificate
 *
 * For the standard value of 4 transmissions and CPS_RETRY_MS of 1s,
 * this results in the default CPS_RETRY_MAX of 15s. (1 + 2 + 4 + 8) = 15 = 2^4 - 1
 * The general formular is:
 * CPS_RETRY_MAX * 2^CONFIG_SEND_CPS_TRANSMIT_MAX - CPS_RETRY_MAX.
 */
#define SEND_CPS_RETRY_MAX_MS       ((CONFIG_SEND_CPS_RETRY_MS *               \
                                      (1u << CONFIG_SEND_CPS_TRANSMIT_MAX))    \
                                      - CONFIG_SEND_CPS_RETRY_MS)
/** @} */

/**
 * @name    SEND router constants
 * @{
 */
/**
 * @brief   @ref CONFIG_SEND_CPA_RATE_MAX
 */
#define SEND_CPA_RATE_MAX           CONFIG_SEND_CPA_RATE_MAX
/** @} */

/**
 * @brief   @ref CONFIG_SEND_NONCE_SIZE_MAX
 */
#define SEND_NONCE_SIZE_MAX         CONFIG_SEND_NONCE_SIZE_MAX

/**
 * @name    SEND ICMPv6 option types
 * @{
 */
/**
 * @brief   CGA parameters option
 */
#define NDP_OPT_CGA_PARAMETERS      (11)
/**
 * @brief   Signature option type
 */
#define NDP_OPT_SIGNATURE           (12)
/**
 * @brief   Timestamp option type
 */
#define NDP_OPT_TIMESTAMP           (13)
/**
 * @brief   Nonce option type
 */
#define NDP_OPT_NONCE               (14)
/**
 * @brief   Trust anchor option type
 */
#define NDP_OPT_TRUST_ANCHOR        (15)
/**
 * @brief   Certificate option type
 */
#define NDP_OPT_CERTIFICATE         (16)
/** @} */

/**
 * @name    SEND ICMPv6 message types
 * @{
 */
/**
 * @brief   Certificate path solicitation message type
 */
#define ICMPV6_CP_SOL               (148)
/**
 * @brief   Certificate path advertisement message type
 */
#define ICMPV6_CP_ADV               (149)
/** @} */

/**
 * @brief   CGA option format
 */
typedef struct __attribute__((packed)) {
    uint8_t type;                   /**< @ref NDP_OPT_CGA_PARAMETERS */
    uint8_t len;                    /**< Length in octets */
    uint8_t pad_len;                /**< Padding length in bytes */
    uint8_t resv;                   /**< Reserved */
    ipv6_cga_parameters_t cga_par;  /**< CGA parameters */
} ndp_opt_cga_params_t;

/**
 * @brief   Access the public key in a CGA option
 *
 * @param[in]       opt         CGA option
 */
#define gnrc_send_opt_cga_get_pk(opt)       ((void *)((opt) + 1))

/**
 * @brief   Get the size of the public key in a CGA option
 *
 * @param[in]       opt         CGA option
 *
 * @return  Size of public key
 */
static inline size_t gnrc_send_opt_cga_get_pk_size(const ndp_opt_cga_params_t *opt)
{
    return (8 * opt->len) - sizeof(*opt) - opt->pad_len;
}

/**
 * @brief   Signature option format
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< @ref NDP_OPT_SIGNATURE */
    uint8_t len;                /**< Length in octets */
    uint16_t resv;              /**< Reserved */
    uint8_t key_hash[16];       /**< SHA1 hash of the public key */
    /* variable length signature */
} ndp_opt_sig_t;

/**
 * @brief   Access the signature in a sgnature option
 *
 * @param[in]       opt         CGA option
 */
#define gnrc_send_opt_sig_get_sig(opt)      ((void *)((opt) + 1))

/**
 * @brief   Nonce option format
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< @ref NDP_OPT_NONCE */
    uint8_t len;                /**< Length in octets */
    /* variable length nonce */
} ndp_opt_nonce_t;

/**
 * @brief   Access the nonce in a nonce option
 *
 * @param[in]       opt         CGA option
 */
#define gnrc_send_opt_nonce_get_nonce(opt)  ((void *)((opt) + 1))

/**
 * @name Trust anchor formats
 * @{
 */
/**
 * @brief   Trust anchor is DER encoded
 */
#define SEND_OPT_TA_NAME_TYPE_DER       (1u)
/**
 * @brief   Trust anchor is an FQDN string
 */
#define SEND_OPT_TA_NAME_TYPE_FQDN      (2u)
/**
 * @brief   Trust anchor is CBOR encoded
 */
#define SEND_OPT_TA_NAME_TYPE_CBOR      (255u)
/** @} */

/**
 * @brief   Data type for trust anchor encodings
 */
typedef enum {
    NDP_TA_TYPE_DER = SEND_OPT_TA_NAME_TYPE_DER,    /**< @ref SEND_OPT_TA_NAME_TYPE_DER */
    NDP_TA_TYPE_FQDN = SEND_OPT_TA_NAME_TYPE_FQDN,  /**< @ref SEND_OPT_TA_NAME_TYPE_FQDN */
    NDP_TA_TYPE_CBOR = SEND_OPT_TA_NAME_TYPE_CBOR,  /**< @ref SEND_OPT_TA_NAME_TYPE_CBOR */
} ndp_ta_name_type_t;

/**
 * @brief   Trust anchor option format
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< @ref NDP_OPT_TRUST_ANCHOR */
    uint8_t len;                /**< Length in octets */
    uint8_t name_type;          /**< Trust anchor type */
    uint8_t pad_len;            /**< Padding length in bytes */
    /* variable length name */
} ndp_opt_ta_t;

/**
 * @brief   Access the trust anchor name in a trust anchor option
 *
 * @param[in]       opt         CGA option
 */
#define gnrc_send_opt_ta_get_name(opt)      ((void *)((opt) + 1))

/**
 * @brief   Get the size of the trust anchor name in a trust anchor option
 *
 * @param[in]       opt         Trust anchor option
 *
 * @return  Size of the trust anchor name
 */
static inline size_t gnrc_send_opt_ta_get_name_size(const ndp_opt_ta_t *opt)
{
    return (opt->len * 8) - sizeof(*opt) - opt->pad_len;
}

/**
 * @name Certificate formats
 * @{
 */
/**
 * @brief   Certificate is DER encoded
 */
#define SEND_OPT_CERT_CERT_TYPE_DER     (1u)
/**
 * @brief   Certificate is CBOR encoded
 */
#define SEND_OPT_CERT_CERT_TYPE_CBOR    (255u)
/** @} */

/**
 * @brief   Data type for certificate encodings
 */
typedef enum {
    NDP_CERT_TYPE_DER = SEND_OPT_CERT_CERT_TYPE_DER,    /**< @ref SEND_OPT_CERT_CERT_TYPE_DER */
    NDP_CERT_TYPE_CBOR = SEND_OPT_CERT_CERT_TYPE_CBOR,  /**< @ref SEND_OPT_CERT_CERT_TYPE_CBOR */
} ndp_cert_type_t;

/**
 * @brief   Certificate option format
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< @ref NDP_OPT_CERTIFICATE */
    uint8_t len;                /**< Length in octets */
    uint8_t cert_type;          /**< Certificate encoding */
    uint8_t resv;               /**< Reserved */
    /* variable length certificate */
} ndp_opt_cert_t;

/**
 * @brief   Access the certificate in a certificate option
 *
 * @param[in]       opt         CGA option
 */
#define gnrc_send_opt_cert_get_cert(opt)        ((void *)((opt) + 1))

/**
 * @brief   The component number to retrieve all certificates in a certificate path
 */
#define SEND_CPS_ALL_COMP   (65535u)

/**
 * @brief   Certificate path solicitation message format
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< @ref ICMPV6_CP_SOL */
    uint8_t code;               /**< 0 */
    network_uint16_t csum;      /**< Checksum */
    network_uint16_t ident;     /**< CPS identifier */
    network_uint16_t comp;      /**< Requested certificate component */
} ndp_cp_sol_t;

/**
 * @brief   Certificate path advertisement message format
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< @ref ICMPV6_CP_ADV */
    uint8_t code;               /**< 0 */
    network_uint16_t csum;      /**< Checksum */
    network_uint16_t ident;     /**< CPS identifier */
    network_uint16_t all_comp;  /**< Total number of certificates in the path */
    network_uint16_t comp;      /**< Certificate component */
    uint16_t resv;              /**< Reserved */
} ndp_cp_adv_t;

#ifdef __cplusplus
}
#endif

#endif /* NET_SEND_H */
/** @} */
