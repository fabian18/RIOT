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

#include "ptrtag.h"
#include "net/ipv6.h"
#include "net/icmpv6.h"
#include "net/send.h"
#include "net/gnrc/pkt.h"
#include "cib.h"
#include "ztimer.h"

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

/**
 * @brief   Secure Neghbor Discovery status codes
 */
enum {
    GNRC_SEND_STATUS_OK = 0x00,                 /**< all good */
    GNRC_SEND_STATUS_UNSECURED = 0x01u,         /**< ICMPv6 message is not SEND secured */
    GNRC_SEND_STATUS_CGA_FAIL,                  /**< CGA verification failed */
    GNRC_SEND_STATUS_SIGNATURE_FAIL,            /**< Signature verification failed */
    GNRC_SEND_STATUS_BAD_FORMAT,                /**< SEND related error in ICMPv6 message format */
    GNRC_SEND_STATUS_NO_KEY,                    /**< No key could be found to verify the signature */
    GNRC_SEND_STATUS_NONCE_MISMATCH,            /**< Nonce does not match */
    GNRC_SEND_STATUS_ADD_IN_PROGRESS,           /**< Certificate exchange has been started */
    GNRC_SEND_STATUS_RESOURCE_NOT_AVAILABLE,    /**< Some allocation of internal resources failed */
};

#if !defined(CONFIG_GNRC_SEND_DEF_MODE) || defined(DOXYGEN)
/**
 * @brief   Default SEND mode to use when SEND is enabled
 */
#define CONFIG_GNRC_SEND_DEF_MODE       GNRC_NIB_IPV6_SEND_MODE_COMPAT
#endif

/**
 * @brief   Size of the nonce in an ICMPv6 message
 */
#define GNRC_SEND_NONCE_SIZE            (6u)
/**
 * @brief   Maximum length of a path in the VFS for SEND related files
 */
#define GNRC_SEND_PATH_MAX              (64u)
/**
 * @brief   Number of reserved ip address blocks that can be processed
 *          in an X509 IP block extension
 */
#define GNRC_SEND_SEC_PFX_NUMOF         (2u)
/**
 * @brief   @ref CONFIG_GNRC_SEND_DEF_MODE
 */
#define GNRC_SEND_DEF_MODE              CONFIG_GNRC_SEND_DEF_MODE
/**
 * @brief   Extra IPv6 thread stack size reserved for SEND
 */
#define GNRC_SEND_EXTRA_STACKSIZE       (2048u)

/**
 * @brief   GNRC SEND identity type
 */
typedef struct gnrc_send_ident gnrc_send_ident_t;

/**
 * @brief   GNRC SEND X509 certificate type
 */
typedef struct gnrc_send_x509_crt gnrc_send_x509_crt_t;

/**
 * @brief   GNRC SEND X509 certificate extensions type
 */
typedef struct gnrc_send_x509_extn gnrc_send_x509_extn_t;

/**
 * @brief   GNRC SEND public key type
 */
typedef struct gnrc_send_pk gnrc_send_pk_t;

/**
 * @brief   GNRC SEND key type
 */
typedef struct gnrc_send_key gnrc_send_key_t;

/**
 * @brief   GNRC SEND certificate path component type
 */
typedef struct gnrc_send_crt gnrc_send_crt_t;

/**
 * @brief   GNRC SEND trust anchor type in a certificate path
 */
typedef struct gnrc_send_ta gnrc_send_ta_t;

/**
 * @brief   GNRC SEND certificate path type
 */
typedef struct gnrc_send_cp gnrc_send_cp_t;

/**
 * @brief   GNRC SEND Certificate Path Solicitation (CPS) type
 */
typedef struct gnrc_send_cache_cps gnrc_send_cache_cps_t;

/**
 * @brief   GNRC SEND Certificate Path Advertisement (CPA) type
 */
typedef struct gnrc_send_cache_cpa gnrc_send_cache_cpa_t;

/**
 * @brief   GNRC SEND context type
 */
typedef struct gnrc_send_ctx {
    struct {
        cib_t cib;                          /**< Circular buffer to limit CPA */
        ztimer_now_t ts[SEND_CPA_RATE_MAX]; /**< Timestamps of least recently sent CPAs */
    } last_cp_adv;                      /**< Context of least recently sent CPAs */
    int mode;                           /**< Current SEND mode */
} gnrc_send_ctx_t;

/**
 * @brief   Encode security status in a pointer
 *
 * @param[in]       ptr         Pointer to be tagged
 * @param[in]       stat        Status tag
 *
 * @return  Tagged pointer
 */
static inline void *gnrc_send_set_status(const void *ptr, uint8_t stat)
{
#if IS_USED(MODULE_GNRC_SEND)
    return ptrtag((void *)ptr, stat);
#endif
    (void)stat;
    return (void *)ptr;
}

/**
 * @brief   Retrieve the security status from a tagged pointer
 *
 * @param[in,out]   ptr         Pointer to be untagged
 *
 * @return  Security status tag
 */
static inline int gnrc_send_remove_status(const void **ptr)
{
#if IS_USED(MODULE_GNRC_SEND)
    int stat = ptrtag_tag((void *)*ptr);
    *ptr = ptrtag_ptr((void *)*ptr);
    return stat;
#endif
    (void)ptr;
    return 0;
}

/**
 * @brief   GNRC SEND auto-init function called once on system boot
 */
void gnrc_send_init(void);

/**
 * @brief   Build CGA parameters option of @p addr assigned to @p netif
 *
 * @param[in]       addr        IPv6 CGA assigned to @p netif
 * @param[in]       netif       Network interface with address @p addr
 * @param[in]       next        Next packet snip pointer
 *
 * @return  Packet snip containing the CGA option or NULL on error
 */
gnrc_pktsnip_t *gnrc_send_cga_params_build(const ipv6_addr_t *addr, struct gnrc_netif *netif,
                                           gnrc_pktsnip_t *next);

/**
 * @brief   Build GNRC SEND signature option
 *
 * @param[in]       icmpv6      ICMPv6 message packet snip with all added options
 * @param[in]       netif       Network interface @p icmpv6 is sent over
 * @param[in]       src         IPv6 source address
 * @param[in]       dst         IPv6 destination address
 *
 * @return  Packet snip containing the signature or NULL on error
 */
gnrc_pktsnip_t *gnrc_send_signature_build(const gnrc_pktsnip_t *icmpv6, struct gnrc_netif *netif,
                                          const ipv6_addr_t *src, const ipv6_addr_t *dst);

/**
 * @brief   Build GNRC SEND trust anchor option
 *
 * @param[in]       ta          Trust anchor
 * @param[in]       type        Encoding type
 * @param[in]       next        Next packet snip pointer
 *
 * @return  Packet snip containing the trust anchor or NULL on error
 */
gnrc_pktsnip_t *gnrc_send_trust_anchor_build(const gnrc_send_ta_t *ta, ndp_ta_name_type_t type, gnrc_pktsnip_t *next);

/**
 * @brief   Build GNRC SEND nonce option
 *
 * @param[in]       nonce       Nonce
 * @param[in]       size        Nonce size
 * @param[in]       next        Next packet snip pointer
 *
 * @return  Packt snip containing the nonce or NULL on error
 */
gnrc_pktsnip_t *gnrc_send_nonce_build(void *nonce, size_t size, gnrc_pktsnip_t *next);

/**
 * @brief   Check GNRC SEND signature
 *
 * @param[in]       icmpv6      ICMPv6 header
 * @param[in]       sig         signature option
 * @param[in]       pk          Public key
 * @param[in]       pk_size     Public key size
 * @param[in]       src         IPv6 source address
 * @param[in]       dst         IPv6 destination address
 *
 * @return  Negative GNRC SEND status code or @ref GNRC_SEND_STATUS_OK
 */
int gnrc_send_signature_check(const icmpv6_hdr_t *icmpv6, const ndp_opt_sig_t *sig,
                              const void *pk, size_t pk_size,
                              const ipv6_addr_t *src, const ipv6_addr_t *dst);

/**
 * @brief   Get next nonce
 *
 * @param[out]      nonce
 */
void gnrc_send_nonce_get(void *nonce);

/**
 * @brief   Send a Certificate Path Solicitation
 *
 * @param[in]       netif       Network interface the CPS is sent over
 * @param[in]       src         IPv6 source source address
 * @param[in]       dst         IPv6 destination address
 * @param[in]       comp        Certificate path component,
                                @ref SEND_CPS_ALL_COMP for all components
 * @param[in]       cps_ctx     Internal CPS context
 *
 * @return  Negative number or @ref GNRC_SEND_STATUS_OK
 */
int gnrc_send_cp_sol_send(struct gnrc_netif *netif,
                          const ipv6_addr_t *src, const ipv6_addr_t *dst,
                          uint16_t comp, gnrc_send_cache_cps_t *cps_ctx);

/**
 * @brief   Send a Certificate Path Advertisement
 *
 * @param[in]       netif       Network interface  the CPA is sent over
 * @param[in]       src         IPv6 source address
 * @param[in]       dst         IPv6 destination address
 * @param[in]       comp        Certificate path component,
                                @ref SEND_CPS_ALL_COMP for all components
 * @param[in]       ident       CP identity from solicitation
 * @param[in]       cp          Certificate path
 * @param[in]       cpa_ctx     Internal CPA context
 * @param[in]       ext_opts    Extra ICMPv6 options
 *
 * @return  Negative number or @ref GNRC_SEND_STATUS_OK
 */
int gnrc_send_cp_adv_send(struct gnrc_netif *netif,
                          const ipv6_addr_t *src, const ipv6_addr_t *dst,
                          uint16_t comp, uint16_t ident, const gnrc_send_cp_t *cp,
                          gnrc_send_cache_cpa_t *cpa_ctx, gnrc_pktsnip_t *ext_opts);

#ifdef __cplusplus
}
#endif

#endif /* NET_GNRC_SEND_H */
/** @} */
