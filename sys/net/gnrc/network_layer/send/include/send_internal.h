#ifndef GNRC_SEND_INTERNAL_H
#define GNRC_SEND_INTERNAL_H

#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/sha256.h"
#include "random_mbedtls_riot.h"
#include "hashes/sha1.h"
#include "net/ipv6.h"
#include "send_conf.h"
#include "net/send.h"
#include "net/gnrc/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    List of supported ECDSA curves
 * @{
 */
#ifndef secp256r1
#define secp256r1                   MBEDTLS_ECP_DP_SECP256R1
#endif
/** @} */

#if CONFIG_GNRC_SEND_ECDSA_CURVE == MBEDTLS_ECP_DP_SECP256R1
#define GNRC_SEND_ECDSA_SIG_BITS    256
#define GNRC_SEND_ECDSA_MD          sha256
#else
#error "gnrc_send: definition of CONFIG_GNRC_SEND_ECDSA_CURVE is not supported"
#endif

/**
 * @name    List of supported hash algorithms
 */
#ifndef sha256
#define sha256      MBEDTLS_MD_SHA256
#endif
/** @} */

#if GNRC_SEND_ECDSA_MD == MBEDTLS_MD_SHA256
#define GNRC_SEND_ECDSA_MD_SIZE     32
#define GNRC_SEND_ECDSA_MD_FORMAT   \
"%02x %02x %02x %02x %02x %02x %02x %02x\n\
%02x %02x %02x %02x %02x %02x %02x %02x\n\
%02x %02x %02x %02x %02x %02x %02x %02x\n\
%02x %02x %02x %02x %02x %02x %02x %02x"
#define GNRC_SEND_ECDSA_MD_VALUE(h) \
(h)[0], (h)[1], (h)[2], (h)[3], (h)[4], (h)[5], (h)[6], (h)[7],         \
(h)[8], (h)[9], (h)[10], (h)[11], (h)[12], (h)[13], (h)[14], (h)[15],   \
(h)[16], (h)[17], (h)[18], (h)[19], (h)[20], (h)[21], (h)[22], (h)[23], \
(h)[24], (h)[25], (h)[26], (h)[27], (h)[28], (h)[29], (h)[30], (h)[31]
#else
#error "gnrc_send: definition of GNRC_SEND_ECDSA_MD is not supported"
#endif

#define GNRC_SEND_NONCE_CACHE_NUMOF     (1u << (CONFIG_GNRC_SEND_NONCE_CACHE_NUMOF_EXP))
#define GNRC_SEND_CPS_CACHE_NUMOF       (1u << (CONFIG_GNRC_SEND_CPS_CACHE_NUMOF_EXP))
#define GNRC_SEND_CPA_CACHE_NUMOF       (1u << (CONFIG_GNRC_SEND_CPA_CACHE_NUMOF_EXP))
#define GNRC_SEND_TAG_UNSECURED         GNRC_SEND_STATUS_UNSECURED

struct gnrc_send_ident {
    char hash[SHA1_DIGEST_LENGTH];
};

struct gnrc_send_x509_crt {
    mbedtls_x509_crt cert;
};

struct gnrc_send_pk {
    mbedtls_pk_context pk;
};

struct gnrc_send_key {
    size_t key_size;
    void *key;
    size_t pubkey_size;
    void *pubkey;
    struct gnrc_send_pk pk;
};

struct gnrc_send_ta {
    size_t ta_size;
    void *pk;
    size_t pk_size;
    void *name;
    size_t name_size;
    struct gnrc_send_x509_crt cert;
    struct gnrc_send_ident ta_ident;
    union {
        char fqdn[VFS_NAME_MAX + 1];
        char rel_path[VFS_NAME_MAX + 1];
    };
};

typedef struct gnrc_send_ta_iter {
    const struct gnrc_send_ta *ta;
} gnrc_send_ta_iter_t;

struct gnrc_send_cp {
    unsigned num_comp;
    const struct gnrc_send_ta *ta;
    struct gnrc_send_ident cp_ident;
    char rel_path[VFS_NAME_MAX + 1];
};

typedef struct gnrc_send_cp_iter {
    const struct gnrc_send_cp *cp;
} gnrc_send_cp_iter_t;

struct gnrc_send_cache_nonce {
    uint8_t nonce[GNRC_SEND_NONCE_SIZE];
};

struct gnrc_send_cache_cps {
    gnrc_netif_t *netif;
    const struct gnrc_send_ta *ta;
    gnrc_pktsnip_t *rtr_adv;
    ipv6_hdr_t rtr_ipv6;
    uint16_t num_comp;
    uint16_t hi_comp;
    uint16_t identifier;
    uint16_t timeout_ms;
    struct gnrc_send_ident rtr_ident;
    evtimer_msg_event_t ev_retransmission;
};

struct gnrc_send_cache_cpa {
    gnrc_netif_t *netif;
    const struct gnrc_send_cp *cp;
    ipv6_addr_t sol_addr;
    uint16_t hi_comp;
    uint16_t identifier;
    evtimer_msg_event_t ev_transmission;
};

static inline void *gnrc_send_x509_get(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.raw.p;
}

static inline size_t gnrc_send_x509_get_size(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.raw.len;
}

static inline void *gnrc_send_x509_get_pubkey(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.pk_raw.p;
}

static inline size_t gnrc_send_x509_get_pubkey_size(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.pk_raw.len;
}

static inline void *gnrc_send_x509_get_subject(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.subject_raw.p;
}

static inline size_t gnrc_send_x509_get_subject_size(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.subject_raw.len;
}

static inline void *gnrc_send_x509_get_issuer(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.issuer_raw.p;
}

static inline size_t gnrc_send_x509_get_issuer_size(const struct gnrc_send_x509_crt *cert)
{
    return cert->cert.issuer_raw.len;
}

static inline int gnrc_send_random_get(void *p_rng, unsigned char *output, size_t output_len)
{
    (void)p_rng;
    random_ctr_drbg_mbedtls_get(output, output_len);
    return 0; /* cannot fail */
}

void gnrc_send_cp_identifier(struct gnrc_send_ident *ident, const void *pk, size_t pk_size);

void gnrc_send_ta_identifier(struct gnrc_send_ident *ident, const void *name, size_t name_size);

const struct gnrc_send_ident *gnrc_send_get_self_ident(void);

struct gnrc_send_cache_cps *gnrc_send_get_cps_ctx(gnrc_netif_t *netif,
                                                  const ipv6_hdr_t *rtr_hdr,
                                                  const struct gnrc_send_ident *rtr_ident,
                                                  const ndp_rtr_adv_t *rtr_adv,
                                                  size_t rtr_adv_len);

void gnrc_send_free_cps_ctx(struct gnrc_send_cache_cps **cpa_ctx);

void gnrc_send_set_cps_timeout(struct gnrc_send_cache_cps *cps_ctx, uint32_t offset);

struct gnrc_send_cache_cpa *gnrc_send_get_cpa_ctx(gnrc_netif_t *netif, const ipv6_addr_t *sol_addr,
                                                  uint16_t ident, const struct gnrc_send_cp *cp);

void gnrc_send_free_cpa_ctx(struct gnrc_send_cache_cpa **cpa_ctx);

void gnrc_send_set_cpa_timeout(struct gnrc_send_cache_cpa *cpa_ctx, uint32_t offset);

void gnrc_send_init_node(void);

int gnrc_send_load_crt(const char *path, void *buf, size_t size, struct gnrc_send_x509_crt *cert);

int gnrc_send_load_pubkey(const char *path, void *buf, size_t size);

int gnrc_send_load_key(const char *path, void *buf, size_t size);

int gnrc_send_parse_keys(const void *buf, size_t size, struct gnrc_send_pk *pk);

int gnrc_send_load_ta(const char *path, struct gnrc_send_ta *ta, void *ta_buf, size_t ta_buf_size, unsigned max);

int gnrc_send_load_cp(const char *path, struct gnrc_send_cp *cp, unsigned max,
                      struct gnrc_send_key *key, void *key_buf, size_t key_size, void *pk_buf, size_t pk_size);

void *gnrc_send_get_pubkey_der(void);

size_t gnrc_send_get_pubkey_size_der(void);

void *gnrc_send_get_privkey_der(void);

size_t gnrc_send_get_privkey_size_der(void);

const struct gnrc_send_pk *gnrc_send_get_pk(void);

const struct gnrc_send_ta *gnrc_send_get_ta_by_name(const void *name, size_t nsize);

const struct gnrc_send_ta *gnrc_send_get_ta_by_fqdn(const char *fqdn, size_t len);

const struct gnrc_send_ta *gnrc_send_get_ta_by_ident(const struct gnrc_send_ident *ident);

const struct gnrc_send_cp *gnrc_send_get_cp_by_ident(const struct gnrc_send_ident *ident, const struct gnrc_send_ta *ta);

struct gnrc_send_x509_crt *gnrc_send_cert_acquire(size_t *size, void **crt_buf);

int gnrc_send_cert_verify(struct gnrc_send_x509_crt *crt,
                          struct gnrc_send_x509_crt *trust);

void gnrc_send_cert_release(void);

gnrc_send_ta_iter_t gnrc_send_ta_acquire(void);

const struct gnrc_send_ta *gnrc_send_ta_iterator(gnrc_send_ta_iter_t *iter);

void gnrc_send_ta_release(void);

gnrc_send_cp_iter_t gnrc_send_cp_acquire(void);

const struct gnrc_send_cp *gnrc_send_cp_iterator(gnrc_send_cp_iter_t *iter);

int gnrc_send_cp_add(const struct gnrc_send_cp *cp);

void gnrc_send_cp_release(void);

#ifdef __cplusplus
}
#endif
#endif
