/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gnrc_send
 * @{
 *
 * @file
 * @brief       Internal SEND API
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */
#ifndef GNRC_SEND_INTERNAL_H
#define GNRC_SEND_INTERNAL_H

#include <stdbool.h>

#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/sha256.h"
#include "random_mbedtls_riot.h"
#include "hashes/sha1.h"
#include "net/gnrc/netif.h"
#include "net/ipv6.h"
#include "net/send.h"
#include "net/gnrc/send.h"

#include "send_conf.h"
#include "x509_ip_extn.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    List of supported ECDSA curves
 * @{
 */
#if !defined(secp256r1) || defined(DOXYGEN)
/**
 * @brief   Translate the named elliptic curve secp256r1 to the mbedtls internal identifier
 */
#define secp256r1                   MBEDTLS_ECP_DP_SECP256R1
#endif
/** @} */

#if (GNRC_SEND_ECDSA_CURVE == secp256r1) || defined(DOXYGEN)
/**
 * @brief   if secp256r1 is used: the signature is 256 bit long
 */
#define GNRC_SEND_ECDSA_SIG_BITS    256
/**
 * @brief   if secp256r1 is used: the used hash algorithm is SHA256
 */
#define GNRC_SEND_ECDSA_MD          sha256
/**
 * @brief   if secp256r1 is used: the public key is 64 bytes long
 */
#define GNRC_SEND_ECDSA_PK_SIZE     64
/**
 * @brief   if secp256r1 is used: the estimated DER encoding length of the public key is 94 bytes
 */
#define GNRC_SEND_ECDSA_PK_DER_MAX (GNRC_SEND_ECDSA_PK_SIZE + 30)
#else
#error "gnrc_send: definition of CONFIG_GNRC_SEND_ECDSA_CURVE is not supported"
#endif

/**
 * @name    List of supported hash algorithms
 * @{
 */
#if !defined(sha256) || defined(DOXYGEN)
/**
 * @brief   Translate the hasch algorithm SHA256 to the mbedtls internal identifier
 */
#define sha256      MBEDTLS_MD_SHA256
#endif
/** @} */

#if (GNRC_SEND_ECDSA_MD == sha256) || defined(DOXYGEN)
/**
 * @brief   if sha256 is used: the hash digest is 32 bytes
 */
#define GNRC_SEND_ECDSA_MD_SIZE     32
/**
 * @brief   if sha256 is uses: this is the printf() format of the hash value
 */
#define GNRC_SEND_ECDSA_MD_FORMAT                                           \
    "%02x %02x %02x %02x %02x %02x %02x %02x\n"                             \
    "%02x %02x %02x %02x %02x %02x %02x %02x\n"                             \
    "%02x %02x %02x %02x %02x %02x %02x %02x\n"                             \
    "%02x %02x %02x %02x %02x %02x %02x %02x"
/**
 * @brief   if sha256 is uses: this is the printf() hash value
 */
#define GNRC_SEND_ECDSA_MD_VALUE(h)                                         \
    (h)[0], (h)[1], (h)[2], (h)[3], (h)[4], (h)[5], (h)[6], (h)[7],         \
    (h)[8], (h)[9], (h)[10], (h)[11], (h)[12], (h)[13], (h)[14], (h)[15],   \
    (h)[16], (h)[17], (h)[18], (h)[19], (h)[20], (h)[21], (h)[22], (h)[23], \
    (h)[24], (h)[25], (h)[26], (h)[27], (h)[28], (h)[29], (h)[30], (h)[31]
#else
#error "gnrc_send: definition of GNRC_SEND_ECDSA_MD is not supported"
#endif

/**
 * @brief   Number of nonces that can be stored
 *
 * If this is too small you will notice that nonce mismatches occur.
 */
#define GNRC_SEND_NONCE_CACHE_NUMOF     (1u << (GNRC_SEND_NONCE_CACHE_NUMOF_EXP))
/**
 * @brief   Number of CPS contexts that can be stored
 */
#define GNRC_SEND_CPS_CACHE_NUMOF       (1u << (GNRC_SEND_CPS_CACHE_NUMOF_EXP))
/**
 * @brief   Number of CPA contexts that can be stored
 */
#define GNRC_SEND_CPA_CACHE_NUMOF       (1u << (GNRC_SEND_CPA_CACHE_NUMOF_EXP))
/**
 * @brief   @ref GNRC_SEND_NAME_BUF_SIZE
 */
#define GNRC_SEND_NAME_DER_MAX          GNRC_SEND_NAME_BUF_SIZE
/**
 * @brief   Estimated length of a CBOR encoded name in a certificate
 */
#define GNRC_SEND_NAME_CBOR_MAX         (GNRC_SEND_NAME_DER_MAX / 2)

/**
 * @brief   Returned by @ref gnrc_send_new_cps_ctx()
 *          if the retrieval of a certificate is already in progress
 */
#define CPS_CTX_ALREADY     ((gnrc_send_cache_cps_t *)(0x08))

/**
 * @brief   Internal type of an identity of a node
 */
struct gnrc_send_ident {
    char hash[SHA1_DIGEST_LENGTH];      /**< identity is a SHA1 value */
};

/**
 * @brief   Internal type of an X509 certificate
 */
struct gnrc_send_x509_crt {
    mbedtls_x509_crt cert;              /**< mbedtls X509 certificate representation */
};

/**
 * @brief   Internal type of a parsed key
 */
struct gnrc_send_pk {
    mbedtls_pk_context pk;              /**< mbedtls public key representation */
};

/**
 * @brief   Internal type of an X509 IP block extension
 */
struct gnrc_send_x509_extn {
    x509_ip_address_block_t ip_block[GNRC_SEND_SEC_PFX_NUMOF];  /**< Array of IP restrictions */
    int ip_block_numof;                                         /**< Number of IP restrictions */
};

/**
 * @brief   Internal type of a public/private key pair
 */
struct gnrc_send_key {
    size_t key_size;                    /**< Prvate key size */
    const void *key;                    /**< Private key */
    size_t pubkey_size;                 /**< Public key size */
    const void *pubkey;                 /**< Public key */
    struct gnrc_send_pk pk;             /**< Parsed key */
};

/**
 * @brief   Iterator type for stored certificates
 */
typedef struct gnrc_send_crt_iter {
    const struct gnrc_send_crt *crt;    /**< Pointer to internal certificate path component */
} gnrc_send_crt_iter_t;

/**
 * @brief   Internal certificate path component
 */
struct gnrc_send_crt {
    struct gnrc_send_crt *parent;           /**< Issuer of this certificate path component */
    struct gnrc_send_ident subject_ident;   /**< Internal subject identifier */
    struct gnrc_send_ident issuer_ident;    /**< Internal issuer identifier */
    struct gnrc_send_ident pk_ident;        /**< Internal public key identifier */
    struct gnrc_send_x509_extn extn;        /**< Extensions of this certificate path component */
    union {
        char rel_path[VFS_NAME_MAX + 1];    /**< Relative path to the certificate in the VFS */
        struct {
            size_t zero;                        /**< Must be zero */
            size_t size;                        /**< Size of the certificate in memory */
            ndp_cert_type_t type;               /**< Certificate type */
            const void *buffer;                 /**< Pointer to where the certificate is stored */
        } crt_inmem;                        /**< Certificate is not stored in the VFS
                                                 but in some buffer */
    };
};

/**
 * @brief   Internal type of the trust anchor in a certificate path
 */
struct gnrc_send_ta {
    struct gnrc_send_crt *crt;                  /**< Pointer to certificate path component */
    size_t pk_size;                             /**< Public key size */
    size_t name_size;                           /**< Subject name size */
    uint8_t pk[GNRC_SEND_ECDSA_PK_DER_MAX];     /**< Buffer to store the public key */
    uint8_t name[GNRC_SEND_NAME_DER_MAX];       /**< Buffer to store the subject name */
};

/**
 * @brief   Iterator type for stored trust anchors
 */
typedef struct gnrc_send_ta_iter {
    const struct gnrc_send_ta *ta;              /**< Pointer to a trust anchor */
} gnrc_send_ta_iter_t;

/**
 * @brief   Internal type of a certificate path
 */
struct gnrc_send_cp {
    struct gnrc_send_crt *crt;                  /**< Router certificate path component */
    const struct gnrc_send_ta *ta;              /**< Trust anchor certificate path component */
    unsigned num_comp;                          /**< Number of components in the path */
};

/**
 * @brief   Iterator type for stored certificate paths
 */
typedef struct gnrc_send_cp_iter {
    const struct gnrc_send_cp *cp;              /**< Pointer to certificate path */
} gnrc_send_cp_iter_t;

/**
 * @brief   Internal type of a nonce
 */
typedef struct gnrc_send_cache_nonce {
    uint8_t nonce[GNRC_SEND_NONCE_SIZE];        /**< Nonce */
} gnrc_send_cache_nonce_t;

/**
 * @brief   Internal certificate path solicitation context
 */
struct gnrc_send_cache_cps {
    gnrc_netif_t *netif;                        /**< Network interface the CPS is sent over */
    const struct gnrc_send_ta *ta;              /**< Trust anchor, set from the CPA reply */
    gnrc_pktsnip_t *rtr_adv;                    /**< Saved RA that triggered the CPS */
    ipv6_hdr_t rtr_ipv6;                        /**< IPv6 header from the RA that triggered the CPS */
    uint16_t num_comp;                          /**< Number of components set from the CPA reply */
    uint16_t hi_comp;                           /**< Next expected component */
    uint16_t identifier;                        /**< Identifier to match a CPS with a CPA */
    uint16_t timeout_ms;                        /**< Next retransmission timeout */
    struct gnrc_send_ident rtr_ident;           /**< Node identity of the router whose certificate path is solicited */
    evtimer_msg_event_t ev_retransmission;      /**< CPS retransmission event */
};

/**
 * @brief   Internal certificate path advertisement context
 */
struct gnrc_send_cache_cpa {
    gnrc_netif_t *netif;                        /**< Network interface the CPA is sent over */
    const struct gnrc_send_cp *cp;              /**< Targeted certificate path */
    ipv6_addr_t sol_addr;                       /**< Address of the soliciting node */
    uint16_t hi_comp;                           /**< Next component to be sent */
    uint16_t identifier;                        /**< Identifier to match a CPS with a CPA */
    evtimer_msg_event_t ev_transmission;        /**< CPA transmission event */
};

/**
 * @brief   Internal function to get the raw DER encoding of an X509 certificate
 *
 * @param[in]       cert        Pointer to a parsed X509 certificate
 *
 * @return  Pointer to the DER encoding of the ceertificate
 */
static inline const void *gnrc_send_x509_get(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.raw.p;
}

/**
 * @brief   Internal function to get the size of the DER encoding of an X509 certificate
 *
 * @param[in]       cert        Pointer to a parsed X509 certificate
 *
 * @return  Size of the DER encoding ot the certificate
 */
static inline size_t gnrc_send_x509_get_size(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.raw.len;
}

/**
 * @brief   Internal function to get the raw DER encoding of the public key
 *          in an X509 certificate
 *
 * @param[in]       cert        Pointer to a parsed X509 certificate
 *
 * @return  Pointer to the DER encoding of the public key in the certificate
 */
static inline const void *gnrc_send_x509_get_pubkey(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.pk_raw.p;
}

/**
 * @brief   Internal function to get the size of the DER encoding of the public key
 *          in an X509 certificate
 *
 * @param[in]       cert        Pointer to a parsed X509 certificate
 *
 * @return  Size of the DER encoding of the public key in the certificate
 */
static inline size_t gnrc_send_x509_get_pubkey_size(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.pk_raw.len;
}

/**
 * @brief   Internal function to get the raw DER encoding of the subject name
 *          in an X509 certificate
 *
 * @param[in]       cert        Pointer to a parsed X509 certificate
 *
 * @return  Pointer to the DER encoding of the subject name in the certificate
 */
static inline const void *gnrc_send_x509_get_subject(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.subject_raw.p;
}

/**
 * @brief   Internal function to get the size of the DER encoding of the subject name
 *          in an X509 certificate
 *
 * @param[in]       cert        Pointer to a parsed X509 certificate
 *
 * @return  Size of the DER encoding of the subject name in the certificate
 */
static inline size_t gnrc_send_x509_get_subject_size(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.subject_raw.len;
}

/**
 * @brief   Internal function to get the raw DER encoding of the issuer name
 *          in an X509 certificate
 *
 * @param[in]       cert        Pointer to a parsed X509 certificate
 *
 * @return  Pointer to the DER encoding of the issuer name in the certificate
 */
static inline const void *gnrc_send_x509_get_issuer(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.issuer_raw.p;
}

/**
 * @brief   Internal function to get the size of the DER encoding of the issuer name
 *          in an X509 certificate
 *
 * @param[in]       cert        Pointer to a parsed X509 certificate
 *
 * @return  Size of the DER encoding of the issuer name in the certificate
 */
static inline size_t gnrc_send_x509_get_issuer_size(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.issuer_raw.len;
}

/**
 * @brief   Internal RNG function to be used within SEND
 *
 * @param[in]       p_rng       Any argumemt
 * @param[in]       output      Destination to write the random bytes to
 * @param[in]       output_len  How many random bytes to write to @p output
 *
 * @return  0
 */
static inline int gnrc_send_random_get(void *p_rng, unsigned char *output, size_t output_len)
{
    (void)p_rng;
    random_ctr_drbg_mbedtls_get(output, output_len);
    return 0; /* cannot fail */
}

/**
 * @brief   Check if a CPA can be sent to the Solicited-node multicast address
 *          instead of all-nodes
 *
 * @param[in]       send        SEND context
 *
 * @return  True if can be sent to the Solicited-node multicast address
 */
static inline bool gnrc_send_check_cpa_rate(const gnrc_send_ctx_t *send)
{
    return !cib_full(&send->last_cp_adv.cib) ||
           (ztimer_now(ZTIMER_MSEC) - send->last_cp_adv.ts[cib_peek(&send->last_cp_adv.cib)])
           >= MS_PER_SEC;
}

/**
 * @brief   Initialize a SEND node
 */
void gnrc_send_init_node(void);

/**
 * @brief   Check if an interface is able to run SEND
 *
 * @param[in]       netif       IPv6 Network interface to be checked
 *
 * @return  Negative number if not supported or @ref GNRC_SEND_STATUS_OK
 */
int gnrc_send_check_iface(const struct gnrc_netif_ipv6 *netif);

/**
 * @brief   Enable SEND on an interface
 *
 * @param[in]       netif       IPv6 Network interface on which to enable SEND
 *
 * @return  Negative number if not successful or @ref GNRC_SEND_STATUS_OK
 */
int gnrc_send_enable_iface(struct gnrc_netif_ipv6 *netif);

/**
 * @brief   Disable SEND on an interface
 *
 * @param[in]       netif       Network interface on which to disable SEND
 */
void gnrc_send_disable_iface(struct gnrc_netif_ipv6 *netif);

/**
 * @brief   Get the DER encoding of the public key of a network interface
 *
 * @param[in]       netif       Network interface from which to get the public key from
 * @param[out]      pk_size     Size of the public key
 *
 * @return  Pointer to the DER encoding of the public key
 */
const void *gnrc_send_get_iface_pubkey_der(struct gnrc_netif_ipv6 *netif, size_t *pk_size);

/**
 * @brief   Initialize the provisioned trust anchors of a node
 *
 * @param[out]      crt         Destination to store the certificate path components of the trust anchors
 * @param[in]       crt_max     Maximum number of supported trust anchors
 * @param[out]      ta          Destination to store the trust anchors
 * @param[in]       ta_max      Maximum number of supported trust anchors
 *
 * @return  Number of initialized trust anchors or negative number on error
 */
int gnrc_send_load_ta(struct gnrc_send_crt *crt, unsigned crt_max,
                      struct gnrc_send_ta *ta, unsigned ta_max);

/**
 * @brief   Initialize the provisioned intermediate certificates of a node
 *
 * @param[out]      crt         Destination to store the certificate path components
 * @param[in]       crt_max     Maximum number of supported certificates
 *
 * @return  Number of initialized certificates or negative number on error
 */
int gnrc_send_load_crt(struct gnrc_send_crt *crt, unsigned crt_max);

/**
 * @brief   Delete all router certificate paths from the VFS
 */
void gnrc_send_clear_cp_vfs(void);

/**
 * @brief   Initialize the provisioned certificate paths of a node
 *
 * @param[out]      crt         Destination to store the certificate path components of the certificate path
 * @param[in]       crt_max     Maximum number of supported certificate path certificates
 * @param[out]      cp          Destination to store the router certificate path component
 * @param[in]       cp_max      Maximum number of supported certificate paths
 *
 * @return  Number of supported certificate paths or negative number on error
 */
int gnrc_send_load_cp(struct gnrc_send_crt *crt, unsigned crt_max,
                      struct gnrc_send_cp *cp, unsigned cp_max);

/**
 * @brief   Initialize the provisioned keypair of a node
 *
 * @param[in]       key         Destination where to store the keypair
 * @param[in]       key_buf     Buffer to store the raw key data
 * @param[in]       buf_size    Buffer size for the raw key data
 *
 * @return  Negative number on error or 1 on success
 */
int gnrc_send_load_keys(struct gnrc_send_key *key, void *key_buf, size_t buf_size);

/**
 * @brief   Load an X509 certificate from the VFS and parse its content
 *
 * @p path can be NULL and in that case it is assumed that the certificate is
 * already present in @p buf.
 *
 * @param[in]       path        Absolute path in the VFS to the certificate
 * @param[in]       buf         Buffer where to load the raw certificate to
 * @param[in]       size        Buffer size
 * @param[out]      cert        Parsed certificate
 * @param[out]      extn_ctx    Parsed certificate extensions needed for SEND
 *
 * @return  Negative number on error or 0 on success
 */
int gnrc_send_load_x509(const char *path, void *buf, size_t size,
                        struct gnrc_send_x509_crt *cert, struct gnrc_send_x509_extn *extn_ctx);

/**
 * @brief   Parses an ECPrivateKey DER structure
 *
 * @param[in]       buf         Buffer where the DER private key is stored
 * @param[in]       size        Buffer size
 * @param[out]      pk          Parsed key structure
 *
 * @return  Negative number on error or 0 on success
 */
int gnrc_send_parse_keys(const void *buf, size_t size, struct gnrc_send_pk *pk);

/**
 * @brief   Initialize a @ref gnrc_send_crt_t structure from an X509 certificate
 *
 * @param[out]      crt         Certificate path component
 * @param[in]       x509        Parsed X509 certificae
 */
void gnrc_send_crt_init(struct gnrc_send_crt *crt, struct gnrc_send_x509_crt *x509);

/**
 * @brief   Compute a public key identifier
 *
 * @param[out]      ident       Computed public key identifier
 * @param[in]       pk          DER encoded public key
 * @param[in]       pk_size     Public key size
 */
void gnrc_send_pk_identifier(struct gnrc_send_ident *ident, const void *pk, size_t pk_size);

/**
 * @brief   Compute a name identifier
 *
 * @param[in]       ident       Computed name identifier
 * @param[in]       name        DER name
 * @param[in]       name_size   Name size
 */
void gnrc_send_name_identifier(struct gnrc_send_ident *ident, const void *name, size_t name_size);

/**
 * @brief   Get the own public key identifier of the own certificate
 *
 * @return  Self PK identity
 */
const struct gnrc_send_ident *gnrc_send_get_self_ident(void);

/**
 * @brief   Allocate a CPS context to be used in @ref gnrc_send_cp_sol_send()
 *
 * @param[in]       netif       Network interface the CPS is sent over
 * @param[in]       rtr_hdr     IPv6 header of the RA that triggered the CPS
 * @param[in]       rtr_ident   Computed identity of the router that sent the RA
 * @param[in]       rtr_adv     Router Advertisement message to be saved
 * @param[in]       rtr_adv_len Size of the ICMPv6 RA message
 *
 * @retval  CPS_CTX_ALREADY if ADD for this router is already in progress
 * @retval  NULL on error or an allocated CPS context
 */
struct gnrc_send_cache_cps *gnrc_send_new_cps_ctx(gnrc_netif_t *netif,
                                                  const ipv6_hdr_t *rtr_hdr,
                                                  const struct gnrc_send_ident *rtr_ident,
                                                  const ndp_rtr_adv_t *rtr_adv,
                                                  size_t rtr_adv_len);

/**
 * @brief   Free an allocated CPS context
 *
 * @param[in]       cps_ctx     An allocated CPS context
 */
void gnrc_send_free_cps_ctx(struct gnrc_send_cache_cps *cps_ctx);

/**
 * @brief   Acquire unique access to a CPS context
 *
 * @param[in]       cps_ctx     CPS context
 */
void gnrc_send_acquire_cps_ctx(struct gnrc_send_cache_cps *cps_ctx);

/**
 * @brief   Release a previously acquired CPS context
 *
 * @param[in]       cps_ctx     CPS context
 */
void gnrc_send_release_cps_ctx(struct gnrc_send_cache_cps *cps_ctx);

/**
 * @brief   Set the timeout for the next CPS retransmission
 *
 * @param[in, out]  cps_ctx     CPS context
 * @param[in]       offset      Time offset in milliseconds
 */
void gnrc_send_set_cps_timeout(struct gnrc_send_cache_cps *cps_ctx, uint32_t offset);

/**
 * @brief   Allocate a CPA context to be used in @ref gnrc_send_cp_adv_send()
 *
 * @param[in]       netif       Network interface the CPA is sent over
 * @param[in]       sol_addr    Address of the node who triggered the CPA transmisison
 * @param[in]       ident       Identity of the CPS which triggered the CPA
 * @param[in]       cp          Targeted certificate path
 *
 * @return  NULL on error or an allocated CPA context
 */
struct gnrc_send_cache_cpa *gnrc_send_new_cpa_ctx(gnrc_netif_t *netif, const ipv6_addr_t *sol_addr,
                                                  uint16_t ident, const struct gnrc_send_cp *cp);

/**
 * @brief   Free a previously allocated CPA context
 *
 * @param[in]       cpa_ctx     CPA context
 */
void gnrc_send_free_cpa_ctx(struct gnrc_send_cache_cpa *cpa_ctx);

/**
 * @brief   Acquire unieque access to a CPA context
 *
 * @param[in]       cpa_ctx     CPA context
 */
void gnrc_send_acquire_cpa_ctx(struct gnrc_send_cache_cpa *cpa_ctx);

/**
 * @brief   Release a previously acquired CPA context
 *
 * @param[in]       cpa_ctx
 */
void gnrc_send_release_cpa_ctx(struct gnrc_send_cache_cpa *cpa_ctx);

/**
 * @brief   Set the timeout for the next CPA transmission
 *
 * @param[in]       cpa_ctx     CPA context
 * @param[in]       offset      Time offset in milliseconds
 */
void gnrc_send_set_cpa_timeout(struct gnrc_send_cache_cpa *cpa_ctx, uint32_t offset);

/**
 * @brief   Get the own DER encoded public key
 *
 * @return  Public key in DER
 */
const void *gnrc_send_get_pubkey_der(void);

/**
 * @brief   Get the size of the own DER encoded public key
 *
 * @return  Size of the own public key
 */
size_t gnrc_send_get_pubkey_size_der(void);

/**
 * @brief   Get the own encoded private key
 *
 * @return  Private key in DER
 */
const void *gnrc_send_get_privkey_der(void);

/**
 * @brief   Get the size of the own DER encoded private key
 *
 * @return  Size of the own private key
 */
size_t gnrc_send_get_privkey_size_der(void);

/**
 * @brief   Get the own parsed keypair
 *
 * @return  Own parsed keypair
 */
const struct gnrc_send_pk *gnrc_send_get_pk(void);

/**
 * @brief   Find a trust anchor by name
 *
 * @param[in]       name        DER encoded name
 * @param[in]       nsize       Name size
 *
 * @return  NULL or the found trust anchor
 */
const struct gnrc_send_ta *gnrc_send_get_ta_by_name(const void *name, size_t nsize);

/**
 * @brief   Find a trust anchor by FQDN name
 *
 * @param[in]       fqdn        FQDN string
 * @param[in]       len         strlen(fqdn)
 *
 * @return  NULL or found trust anchor
 */
const struct gnrc_send_ta *gnrc_send_get_ta_by_fqdn(const char *fqdn, size_t len);

/**
 * @brief   Find a trust anchor by a subject name identity
 *
 * @param[in]       ident       Name identifier
 *
 * @return  NULL or found trust anchor
 */
const struct gnrc_send_ta *gnrc_send_get_ta_by_subject_ident(const gnrc_send_ident_t *ident);

/**
 * @brief   Find a certificate path component by a subject name identity
 *
 * @param[in]       ident       Name identifier
 *
 * @return  NULL or found certificate path component
 */
struct gnrc_send_crt *gnrc_send_get_crt_by_subject_ident(const gnrc_send_ident_t *ident);

/**
 * @brief   Find a certificate path component by a public key identity
 *
 * @param[in]       ident       PK identity
 *
 * @return  NULL or found certificate path component
 */
struct gnrc_send_crt *gnrc_send_get_crt_by_pk_ident(const gnrc_send_ident_t *ident);

/**
 * @brief   Find a certificate path by a public key identity and trust anchor
 *
 * @param[in]       ident       PK identity
 * @param[in]       ta          Trust anchor
 *
 * @return  NULL or found certificate path
 */
const struct gnrc_send_cp *gnrc_send_get_cp_by_pk_ident(const struct gnrc_send_ident *ident, const struct gnrc_send_ta *ta);

/**
 * @brief   Acquire unieque access to the internal X509 certificate buffers
 *          to parse and verify certificates
 *
 * @param[out]      size        Certificate buffer size
 * @param[out]      crt_buf     Certificate buffer
 *
 * @return  Array of certificate structures that can be used for parsing
 */
struct gnrc_send_x509_crt *gnrc_send_x509_acquire(size_t *size, void **crt_buf);

/**
 * @brief   Verify a certificate @p crt with the issuer certificate @p trust
 *
 * A certificate path is transmitted one after the other in a CPA so verification
 * also happens one by one.
 *
 * @param[in]       crt         Certificate to be verified
 * @param[in]       trust       Issuer certificate of @p crt
 *
 * @return  Negative number on error or 0 on success
 */
int gnrc_send_x509_verify(struct gnrc_send_x509_crt *crt,
                          struct gnrc_send_x509_crt *trust);

/**
 * @brief   Release the previously acquired certificate buffers
 */
void gnrc_send_x509_release(void);

/**
 * @brief   Acquire unieque access to the provisioned trust anchors
 *
 * @return  Trust anchor iterator
 */
gnrc_send_ta_iter_t gnrc_send_ta_acquire(void);

/**
 * @brief   Iterate through the provisioned trust anchors
 *
 * @param[in, out]      iter        Trust anchor iterator
 *
 * @return  Current trust anchor
 */
const struct gnrc_send_ta *gnrc_send_ta_iterator(gnrc_send_ta_iter_t *iter);

/**
 * @brief   Release previously acquired trust anchors
 */
void gnrc_send_ta_release(void);

/**
 * @brief   Acquire unieque access to provisioned certificate path components
 *
 * @return  certificate path component iterator
 */
gnrc_send_crt_iter_t gnrc_send_crt_acquire(void);

/**
 * @brief   Iterate through the provisioned certificate path components
 *
 * @param[in, out]  iter        certificate path iterator
 *
 * @return  Current certificate path component
 */
const struct gnrc_send_crt *gnrc_send_crt_iterator(gnrc_send_crt_iter_t *iter);

/**
 * @brief   Release previously acquired certificate path components
 */
void gnrc_send_crt_release(void);

/**
 * @brief   Acquire unieque access to the provisioned certificate paths
 *
 * @return  certificate path iterator
 */
gnrc_send_cp_iter_t gnrc_send_cp_acquire(void);

/**
 * @brief   Iterate through provisioned certificate paths
 *
 * @param[in, out]  iter        certificate path iterator
 *
 * @return  Current certificate path
 */
const struct gnrc_send_cp *gnrc_send_cp_iterator(gnrc_send_cp_iter_t *iter);

/**
 * @brief   Release previously acquired certificate paths
 */
void gnrc_send_cp_release(void);

/**
 * @brief   Add a new certificate path component (intermediate certificate) to the node
 *
 * @pre     The certificate must have been saved to the VFS and @p crt must be
 *          correctly initialized.
 *
 * @param[in]       crt         Populated certificate path component
 *
 * @return  Pointer to the new certificate path component
 */
const struct  gnrc_send_crt *gnrc_send_crt_add(const gnrc_send_crt_t *crt);

/**
 * @brief   Add a new certificate path to the node
 *
 * @pre     The certificate must have been saved to the VFS and @p crt and
 *          @p cp must be correctly initialized.
 *
 * @param[in]       cp          Populated certificate path
 * @param[in]       crt         Populated router certificate path component
 *
 * @return  Pointer to the new certificate path
 */
const struct gnrc_send_cp *gnrc_send_cp_add(const gnrc_send_cp_t *cp, const gnrc_send_crt_t *crt);

#ifdef __cplusplus
}
#endif
#endif /* GNRC_SEND_INTERNAL_H */
/** @} */
