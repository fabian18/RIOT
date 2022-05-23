/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gnrc_send GNRC internal SEcure Neighbor Discovery (SEND) API
 * @ingroup     net_gnrc_ipv6
 * @brief       Provides internal SEND API and constants
 * @{
 *
 * @file
 * @brief       GNRC-specific Secure Neghbor Discovery API
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef NET_GNRC_SEND_H
#define NET_GNRC_SEND_H

#include "net/ipv6.h"
#include "net/icmpv6.h"
#include "net/ndp.h"
#include "net/send.h"
#include "net/gnrc/pkt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief @ref gnrc_netif_t forward declaration to avoid recursive include
 */
struct gnrc_netif;

/**
 * @brief @ref gnrc_netif_ipv6_t forward declaration to avoid recursive include
 */
struct gnrc_netif_ipv6;

#define GNRC_SEND_CP_RTR_CERT_NAME      "1.der"
#define GNRC_SEND_NONCE_SIZE            (6u)
#define GNRC_SEND_PATH_MAX              (64u)

/**
 * @brief
 */
typedef struct gnrc_send_ident gnrc_send_ident_t;

/**
 * @brief
 */
typedef struct gnrc_send_x509_crt gnrc_send_x509_crt_t;

/**
 * @brief
 */
typedef struct gnrc_send_pk gnrc_send_pk_t;

/**
 * @brief
 */
typedef struct gnrc_send_key gnrc_send_key_t;

/**
 * @brief
 */
typedef struct gnrc_send_ta gnrc_send_ta_t;

/**
 * @brief
 */
typedef struct gnrc_send_cp gnrc_send_cp_t;

typedef struct gnrc_send_ctx {
    void *cpa_ctx;
    uint8_t ra_nonce[CONFIG_SEND_NONCE_SIZE_MAX];
    uint8_t ra_nonce_size;
} gnrc_send_ctx_t;

void gnrc_send_init(void);

int gnrc_send_enable_iface(struct gnrc_netif_ipv6 *netif);

void gnrc_send_disable_iface(struct gnrc_netif_ipv6 *netif);

int gnrc_send_cga_generate(struct gnrc_netif_ipv6 *netif, ipv6_addr_t *dst,
                           ipv6_cga_parameters_t *params);

const void *gnrc_send_get_iface_pubkey_der(struct gnrc_netif_ipv6 *netif, size_t *pk_size);

gnrc_pktsnip_t *gnrc_send_cga_params_build(const ipv6_addr_t *addr, struct gnrc_netif *netif,
                                           gnrc_pktsnip_t *next);

gnrc_pktsnip_t *gnrc_send_signature_build(const gnrc_pktsnip_t *icmpv6, struct gnrc_netif *netif,
                                          const ipv6_addr_t *src, const ipv6_addr_t *dst);

gnrc_pktsnip_t *gnrc_send_trust_anchor_build(const gnrc_send_ta_t *ta, ndp_ta_name_type_t type, gnrc_pktsnip_t *next);

gnrc_pktsnip_t *gnrc_send_nonce_build(void *nonce, size_t size, gnrc_pktsnip_t *next);

int gnrc_send_signature_check(const icmpv6_hdr_t *icmpv6, const ndp_opt_sig_t *sig,
                              const void *pk, size_t pk_size,
                              const ipv6_addr_t *src, const ipv6_addr_t *dst);

/**
 * @brief
 */
void gnrc_send_nonce_get(void *nonce);

/**
 * @brief
 */
int gnrc_send_cp_sol_send(struct gnrc_netif *netif,
                          const ipv6_addr_t *src, const ipv6_addr_t *dst,
                          uint16_t comp, const ipv6_hdr_t *rtr_ipv6,
                          const gnrc_send_ident_t *rtr_ident,
                          const ndp_rtr_adv_t *rtr_adv, size_t rtr_adv_len,
                          void **cps_ctx);

/**
 * @brief
 */
int gnrc_send_cp_adv_send(struct gnrc_netif *netif,
                          const ipv6_addr_t *src, const ipv6_addr_t *dst,
                          uint16_t comp, uint16_t ident, const gnrc_send_cp_t *cp,
                          void **cpa_ctx,
                          gnrc_pktsnip_t *ext_opts);

#ifdef __cplusplus
}
#endif

#endif /* NET_GNRC_SEND_H */
/** @} */
