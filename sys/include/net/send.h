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
 * @name    SEND host constants
 * @{
 */
#ifndef CONFIG_SEND_CPS_RETRY_MS
#define CONFIG_SEND_CPS_RETRY_MS            (1u * MS_PER_SEC)
#endif

#ifndef CONFIG_SEND_CPS_RETRY_FRAGMENTS_MS
#define CONFIG_SEND_CPS_RETRY_FRAGMENTS_MS  (2u * MS_PER_SEC)
#endif

#ifndef CONFIG_SEND_CPS_RETRY_MAX_MS
#define CONFIG_SEND_CPS_RETRY_MAX_MS        (15u * MS_PER_SEC)
#endif
/** @} */

/**
 * @name    SEND router constants
 * @{
 */
#ifndef CONFIG_SEND_CPA_RATE_MAX
#define CONFIG_SEND_CPA_RATE_MAX            (10u) /* per sec */
#endif
/** @} */

#ifndef CONFIG_SEND_NONCE_SIZE_MAX
#define CONFIG_SEND_NONCE_SIZE_MAX          (6u) /* bytes */
#endif

#define NDP_OPT_CGA_PARAMETERS      (11)
#define NDP_OPT_SIGNATURE           (12)
#define NDP_OPT_TIMESTAMP           (13)
#define NDP_OPT_NONCE               (14)
#define NDP_OPT_TRUST_ANCHOR        (15)
#define NDP_OPT_CERTIFICATE         (16)

#define ICMPV6_CP_SOL               (148)
#define ICMPV6_CP_ADV               (149)

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t len;
    uint8_t pad_len;
    uint8_t resv;
    ipv6_cga_parameters_t cga_par;
} ndp_opt_cga_params_t;

#define gnrc_send_opt_cga_get_pk(opt)       ((void *)((opt) + 1))

static inline size_t gnrc_send_opt_cga_get_pk_size(const ndp_opt_cga_params_t *opt)
{
    return (8 * opt->len) - sizeof(*opt) - opt->pad_len;
}

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t len;
    uint16_t resv;
    uint8_t key_hash[16];
    /* variable length signature and padding */
} ndp_opt_sig_t;

#define gnrc_send_opt_sig_get_sig(opt)      ((void *)((opt) + 1))

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t len;
    /* variable length nonce */
} ndp_opt_nonce_t;

#define gnrc_send_opt_nonce_get_nonce(opt)  ((void *)((opt) + 1))

#define SEND_OPT_TA_NAME_TYPE_DER       (1u)
#define SEND_OPT_TA_NAME_TYPE_FQDN      (2u)

typedef enum {
    NDP_TA_TYPE_DER = SEND_OPT_TA_NAME_TYPE_DER,
    NDP_TA_TYPE_FQDN = SEND_OPT_TA_NAME_TYPE_FQDN,
} ndp_ta_name_type_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t len;
    uint8_t name_type;
    uint8_t pad_len;
    /* variable length name */
} ndp_opt_ta_t;

#define gnrc_send_opt_ta_get_name(opt)      ((void *)((opt) + 1))

static inline size_t gnrc_send_opt_ta_get_name_size(const ndp_opt_ta_t *opt)
{
    return (opt->len * 8) - sizeof(*opt) - opt->pad_len;
}

#define SEND_OPT_CERT_CERT_TYPE_DER     (1u)
#define SEND_OPT_CERT_CERT_TYPE_CBOR    (255u)

typedef enum {
    NDP_CERT_TYPE_DER = SEND_OPT_CERT_CERT_TYPE_DER,
    NDP_CERT_TYPE_C509 = SEND_OPT_CERT_CERT_TYPE_CBOR,
} ndp_cert_type_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t len;
    uint8_t cert_type;
    uint8_t resv;
    /* variable length certificate */
} ndp_opt_cert_t;

#define gnrc_send_opt_cert_get_cert(opt)        ((void *)((opt) + 1))

#define SEND_CPS_ALL_COMP   (65535u)

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t code;
    network_uint16_t csum;
    network_uint16_t ident;
    network_uint16_t comp;
} ndp_cp_sol_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t code;
    network_uint16_t csum;
    network_uint16_t ident;
    network_uint16_t all_comp;
    network_uint16_t comp;
} ndp_cp_adv_t;

#ifdef __cplusplus
}
#endif

#endif /* NET_SEND_H */
/** @} */
