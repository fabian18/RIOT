/*
 * Copyright (C) 2022 Otto-von-Guericke Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Tests Secure NDP message handling of gnrc stack.
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include "kernel_defines.h"
#include "embUnit.h"
#include "embUnit/AssertImpl.h"
#include "embUnit/TestCaller.h"
#include "embUnit/embUnit.h"
#include "msg.h"
#include "msg_bus.h"
#include "net/gnrc/ipv6/hdr.h"
#include "net/gnrc/ipv6/nib/conf.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/internal.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/nettype.h"
#include "net/gnrc/pkt.h"
#include "net/gnrc/pktbuf.h"
#include "net/icmpv6.h"
#include "net/gnrc/ndp.h"
#include "net/gnrc/netif.h"
#include "net/ipv6/addr.h"
#include "net/ipv6/cga.h"
#include "net/ipv6/hdr.h"
#include "net/ndp.h"
#include "net/netdev.h"
#include "net/send.h"
#include "test_utils/expect.h"
#include "net/netdev_test.h"
#include "pktbuf_static.h"
#include "net/gnrc/send.h"
#include "time_units.h"
#include "ztimer.h"
#include "byteorder.h"

#include "send_internal.h"

/* TODO: move to common header */
#define FOREACH_OPT(ndp_pkt, opt, icmpv6_len) \
    for (opt = (ndp_opt_t *)(ndp_pkt + 1); \
         icmpv6_len > 0; \
         icmpv6_len -= (opt->len << 3), \
         opt = (ndp_opt_t *)(((uint8_t *)opt) + (opt->len << 3)))

#define TEST_ASSERT_PKT_ALLOCATION(pkt, s, t, n)                            \
    TEST_ASSERT_NOT_NULL(pkt);                                              \
    TEST_ASSERT_EQUAL_INT(((uintptr_t)((pkt)->next)), ((uintptr_t)(n)));    \
    TEST_ASSERT_NOT_NULL((pkt)->data);                                      \
    TEST_ASSERT_EQUAL_INT((s), (pkt)->size);                                \
    TEST_ASSERT_EQUAL_INT((t), (pkt)->type);

#define TEST_ASSERT_NDP_OPT(o, s, t)                                        \
    TEST_ASSERT_EQUAL_INT((o)->type, t);                                    \
    TEST_ASSERT(!((s) % 8));                                                \
    TEST_ASSERT_EQUAL_INT((o)->len, (s) / 8);

extern uint8_t _k_buf[GNRC_SEND_KEY_BUF_SIZE];
extern gnrc_send_key_t _key_meta[1];
extern gnrc_send_crt_t _crt_meta[GNRC_SEND_CRT_NUMOF];
extern gnrc_send_ta_t _ta_meta[GNRC_SEND_TA_NUMOF];
extern gnrc_send_cp_t _cp_meta[GNRC_SEND_CP_NUMOF];

static const ipv6_addr_t test_dst = { { 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x73, 0x25, 0x22, 0xc6, 0xdf, 0x05, 0xf2, 0x6b } };
static const ipv6_addr_t test_src = { { 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0xe5, 0x43, 0xb7, 0x74, 0xd7, 0xa9, 0x30, 0x74 } };
static const uint8_t test_src_l2[] = { 0xe7, 0x43, 0xb7, 0x74, 0xd7, 0xa9, 0x30, 0x74 };

static const uint8_t test_ec_ta_certificate[] = {
    0x30, 0x82, 0x01, 0xb1, 0x30, 0x82, 0x01, 0x58, 0xa0, 0x03, 0x02, 0x01,
    0x02, 0x02, 0x14, 0x5d, 0x82, 0x07, 0x79, 0x07, 0xf8, 0x70, 0x0d, 0x6d,
    0xbb, 0x82, 0x65, 0x48, 0x61, 0x7b, 0x12, 0x63, 0x21, 0xe6, 0x5d, 0x30,
    0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30,
    0x14, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x09,
    0x63, 0x61, 0x5f, 0x61, 0x6e, 0x63, 0x68, 0x6f, 0x72, 0x30, 0x1e, 0x17,
    0x0d, 0x32, 0x32, 0x30, 0x38, 0x31, 0x37, 0x30, 0x39, 0x33, 0x31, 0x30,
    0x35, 0x5a, 0x17, 0x0d, 0x33, 0x32, 0x30, 0x38, 0x31, 0x34, 0x30, 0x39,
    0x33, 0x31, 0x30, 0x35, 0x5a, 0x30, 0x14, 0x31, 0x12, 0x30, 0x10, 0x06,
    0x03, 0x55, 0x04, 0x03, 0x0c, 0x09, 0x63, 0x61, 0x5f, 0x61, 0x6e, 0x63,
    0x68, 0x6f, 0x72, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48,
    0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03,
    0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x97, 0x98, 0x44, 0x0e, 0x5d, 0xd5,
    0x87, 0x0c, 0xac, 0x36, 0x85, 0x5b, 0xfb, 0xfa, 0x18, 0x31, 0x5a, 0x88,
    0x34, 0x33, 0x21, 0x95, 0x1d, 0x41, 0xbf, 0xa9, 0xde, 0x45, 0x39, 0x90,
    0x5f, 0x9d, 0x98, 0x7f, 0xec, 0x70, 0xfb, 0x66, 0xe3, 0x96, 0x1f, 0x33,
    0x96, 0x07, 0xf6, 0x8a, 0xbe, 0x94, 0x8b, 0x8b, 0xca, 0x3d, 0xab, 0x53,
    0xd1, 0x97, 0x34, 0xc8, 0x31, 0x4c, 0x63, 0xe7, 0xcc, 0x64, 0xa3, 0x81,
    0x87, 0x30, 0x81, 0x84, 0x30, 0x31, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05,
    0x05, 0x07, 0x01, 0x07, 0x01, 0x01, 0xff, 0x04, 0x22, 0x30, 0x20, 0x30,
    0x1e, 0x04, 0x02, 0x00, 0x02, 0x30, 0x18, 0x30, 0x16, 0x03, 0x09, 0x00,
    0xfd, 0x00, 0x00, 0x0a, 0x00, 0x0b, 0x00, 0x01, 0x03, 0x09, 0x01, 0xfd,
    0x00, 0x00, 0x0a, 0x00, 0x0b, 0x00, 0x00, 0x30, 0x1d, 0x06, 0x03, 0x55,
    0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xb2, 0xe4, 0xe9, 0xfc, 0x4c, 0x10,
    0x2f, 0xc2, 0x47, 0x83, 0x08, 0x57, 0xd0, 0xf7, 0x75, 0x8d, 0x99, 0x4a,
    0x7c, 0x51, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30,
    0x16, 0x80, 0x14, 0xb2, 0xe4, 0xe9, 0xfc, 0x4c, 0x10, 0x2f, 0xc2, 0x47,
    0x83, 0x08, 0x57, 0xd0, 0xf7, 0x75, 0x8d, 0x99, 0x4a, 0x7c, 0x51, 0x30,
    0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30,
    0x03, 0x01, 0x01, 0xff, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce,
    0x3d, 0x04, 0x03, 0x02, 0x03, 0x47, 0x00, 0x30, 0x44, 0x02, 0x20, 0x26,
    0xa5, 0x6e, 0x67, 0x0e, 0x27, 0xf7, 0x20, 0x0a, 0xe9, 0x6e, 0x0d, 0x2b,
    0xf1, 0x2d, 0x67, 0x92, 0xe2, 0x0b, 0x7c, 0xb9, 0x19, 0x82, 0x94, 0xf5,
    0xb4, 0x12, 0x29, 0xbf, 0x8e, 0x2f, 0x43, 0x02, 0x20, 0x0b, 0xcb, 0x4d,
    0xcc, 0x95, 0xdf, 0xba, 0x2a, 0x04, 0x00, 0x0b, 0x08, 0x17, 0xa0, 0x17,
    0x91, 0xf5, 0xf6, 0x1b, 0xb4, 0xf8, 0x70, 0x50, 0xdf, 0x39, 0xd6, 0x90,
    0x69, 0xdf, 0x32, 0xd6, 0x9a,
};
#define X509_EC_CERT_SIZE_TA            sizeof(test_ec_ta_certificate)

#define X509_EC_PK_OFFSET_TA            123
#define X509_EC_PK_SIZE_TA              91
static const uint8_t *test_ec_pk_ta = &test_ec_ta_certificate[X509_EC_PK_OFFSET_TA];

#define X509_ISSUER_NAME_OFFSET_TA      47
#define X509_ISSUER_NAME_SIZE_TA        22
static const uint8_t *test_ec_issuer_name_ta = &test_ec_ta_certificate[X509_ISSUER_NAME_OFFSET_TA];

#define X509_SUBJECT_NAME_OFFSET_TA     101
#define X509_SUBJECT_NAME_SIZE_TA       22
static const uint8_t *test_ec_subject_name_ta = &test_ec_ta_certificate[X509_SUBJECT_NAME_OFFSET_TA];

static const uint8_t test_ec_rtr_certificate[] = {
    0x30, 0x82, 0x01, 0x94, 0x30, 0x82, 0x01, 0x3b, 0xa0, 0x03, 0x02, 0x01,
    0x02, 0x02, 0x02, 0x10, 0x00, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48,
    0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x14, 0x31, 0x12, 0x30, 0x10, 0x06,
    0x03, 0x55, 0x04, 0x03, 0x0c, 0x09, 0x63, 0x61, 0x5f, 0x61, 0x6e, 0x63,
    0x68, 0x6f, 0x72, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x32, 0x30, 0x38, 0x31,
    0x37, 0x30, 0x39, 0x33, 0x31, 0x34, 0x36, 0x5a, 0x17, 0x0d, 0x32, 0x33,
    0x30, 0x38, 0x31, 0x37, 0x30, 0x39, 0x33, 0x31, 0x34, 0x36, 0x5a, 0x30,
    0x10, 0x31, 0x0e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x05,
    0x72, 0x74, 0x72, 0x5f, 0x30, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce,
    0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x8e, 0xac, 0xff, 0xe0,
    0x45, 0xc7, 0x9c, 0x14, 0x7a, 0x84, 0x6f, 0x99, 0xa9, 0x27, 0xb9, 0x65,
    0xdf, 0x03, 0xc3, 0xdf, 0xe2, 0x85, 0x14, 0xdb, 0x02, 0xbe, 0xce, 0x30,
    0xd3, 0xd7, 0xc5, 0xba, 0x86, 0xd4, 0x70, 0xcb, 0x52, 0x8d, 0x4f, 0xf0,
    0xa0, 0xd1, 0x9b, 0x4f, 0xaa, 0x40, 0x32, 0x31, 0xba, 0xd3, 0x3d, 0x21,
    0xa4, 0x0b, 0xb9, 0xd7, 0x06, 0x81, 0xe0, 0x51, 0x17, 0xaa, 0x8f, 0x7c,
    0xa3, 0x81, 0x80, 0x30, 0x7e, 0x30, 0x09, 0x06, 0x03, 0x55, 0x1d, 0x13,
    0x04, 0x02, 0x30, 0x00, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04,
    0x16, 0x04, 0x14, 0xbb, 0x31, 0x38, 0x61, 0x25, 0xc8, 0xd0, 0x71, 0x7c,
    0x6f, 0xf2, 0x9e, 0x94, 0x98, 0x34, 0xb6, 0x40, 0x0c, 0x12, 0x2a, 0x30,
    0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14,
    0xb2, 0xe4, 0xe9, 0xfc, 0x4c, 0x10, 0x2f, 0xc2, 0x47, 0x83, 0x08, 0x57,
    0xd0, 0xf7, 0x75, 0x8d, 0x99, 0x4a, 0x7c, 0x51, 0x30, 0x0b, 0x06, 0x03,
    0x55, 0x1d, 0x0f, 0x04, 0x04, 0x03, 0x02, 0x05, 0xe0, 0x30, 0x24, 0x06,
    0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x07, 0x01, 0x01, 0xff,
    0x04, 0x15, 0x30, 0x13, 0x30, 0x11, 0x04, 0x02, 0x00, 0x02, 0x30, 0x0b,
    0x03, 0x09, 0x00, 0xfd, 0x00, 0x00, 0x0a, 0x00, 0x0b, 0x00, 0x01, 0x30,
    0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03,
    0x47, 0x00, 0x30, 0x44, 0x02, 0x20, 0x51, 0xe3, 0xde, 0xb8, 0x7c, 0x2d,
    0xc9, 0x98, 0xad, 0xf7, 0xf4, 0xff, 0x75, 0x86, 0xb1, 0x7b, 0xed, 0x82,
    0x4d, 0x7b, 0x4b, 0x0f, 0x43, 0x3e, 0x9c, 0x23, 0x0d, 0x52, 0x48, 0xdd,
    0x68, 0x84, 0x02, 0x20, 0x75, 0x85, 0x4f, 0x58, 0x35, 0x4e, 0x40, 0xa5,
    0xa0, 0x70, 0x49, 0xc2, 0x0b, 0xba, 0x01, 0x47, 0xe0, 0x0f, 0x55, 0xaa,
    0x93, 0x6b, 0xfc, 0x49, 0x62, 0x92, 0x6b, 0x22, 0x82, 0xdf, 0x9a, 0x7c,

};
#define X509_EC_CERT_SIZE_RTR           sizeof(test_ec_rtr_certificate)

#define X509_EC_PK_OFFSET_RTR           101
#define X509_EC_PK_SIZE_RTR             91
static const uint8_t *test_ec_pk_rtr = &test_ec_rtr_certificate[X509_EC_PK_OFFSET_RTR];

#define X509_ISSUER_NAME_OFFSET_RTR     29
#define X509_ISSUER_NAME_SIZE_RTR       22
static const uint8_t *test_ec_issuer_name_rtr = &test_ec_rtr_certificate[X509_ISSUER_NAME_OFFSET_RTR];

#define X509_SUBJECT_NAME_OFFSET_RTR    83
#define X509_SUBJECT_NAME_SIZE_RTR      18
static const uint8_t *test_ec_subject_name_rtr = &test_ec_rtr_certificate[X509_SUBJECT_NAME_OFFSET_RTR];

static const uint8_t test_ec_priv_rtr[] = {
    0x30, 0x77, 0x02, 0x01, 0x01, 0x04, 0x20, 0x9b, 0x7a, 0xf2, 0xff, 0xe4,
    0x62, 0x9c, 0x08, 0xf7, 0xe4, 0xd6, 0x80, 0x69, 0xad, 0x1a, 0x12, 0xd3,
    0xd6, 0xb6, 0xba, 0xf3, 0x24, 0x7b, 0xf2, 0xb7, 0xa0, 0xb6, 0x76, 0x8e,
    0x9a, 0xff, 0xa5, 0xa0, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d,
    0x03, 0x01, 0x07, 0xa1, 0x44, 0x03, 0x42, 0x00, 0x04, 0x8e, 0xac, 0xff,
    0xe0, 0x45, 0xc7, 0x9c, 0x14, 0x7a, 0x84, 0x6f, 0x99, 0xa9, 0x27, 0xb9,
    0x65, 0xdf, 0x03, 0xc3, 0xdf, 0xe2, 0x85, 0x14, 0xdb, 0x02, 0xbe, 0xce,
    0x30, 0xd3, 0xd7, 0xc5, 0xba, 0x86, 0xd4, 0x70, 0xcb, 0x52, 0x8d, 0x4f,
    0xf0, 0xa0, 0xd1, 0x9b, 0x4f, 0xaa, 0x40, 0x32, 0x31, 0xba, 0xd3, 0x3d,
    0x21, 0xa4, 0x0b, 0xb9, 0xd7, 0x06, 0x81, 0xe0, 0x51, 0x17, 0xaa, 0x8f,
    0x7c,
};

static ipv6_addr_t *test_cga_rtr;
static uint8_t test_nonce[GNRC_SEND_NONCE_SIZE] = { 0x11, 0x22, 0x33,
                                                    0x44, 0x55, 0x66 };
static gnrc_netif_t *test_netif = NULL;
static gnrc_netif_t _netif;

static void init_pkt_handler(void);
static void init_test_netdev_thread(void);

static inline size_t _ceil8(size_t length)
{
    assert(length <= SIZE_MAX - 7);
    /* NDP options use units of 8 byte for their length field, so round up */
    return (length + 7U) & ~((size_t)0x7);
}

/**
 * @name Oerloaded function definitions
 * @{
 */
int gnrc_send_load_ta(gnrc_send_crt_t *crt, unsigned crt_max,
                      gnrc_send_ta_t *ta, unsigned ta_max)
{
    (void)crt_max; (void)ta_max;
    crt->parent = NULL;
    crt->extn = (gnrc_send_x509_extn_t){ .ip_block_numof = 0 };
    crt->crt_inmem.zero = 0;
    crt->crt_inmem.size = X509_EC_CERT_SIZE_TA;
    crt->crt_inmem.type = NDP_CERT_TYPE_DER,
    crt->crt_inmem.buffer = test_ec_ta_certificate;
    gnrc_send_pk_identifier(&crt->pk_ident,
                            test_ec_pk_ta, X509_EC_PK_SIZE_TA);
    gnrc_send_name_identifier(&crt->issuer_ident,
                              test_ec_issuer_name_ta, X509_ISSUER_NAME_SIZE_TA);
    gnrc_send_name_identifier(&crt->subject_ident,
                              test_ec_subject_name_ta, X509_SUBJECT_NAME_SIZE_TA);
    ta->crt = crt;
    memcpy(ta->name, test_ec_subject_name_ta, ta->name_size = X509_SUBJECT_NAME_SIZE_TA);
    memcpy(ta->pk, test_ec_pk_ta, ta->pk_size = X509_EC_PK_SIZE_TA);
    return 1;
}

int gnrc_send_load_crt(gnrc_send_crt_t *crt, unsigned crt_max)
{
    (void)crt; (void)crt_max;
    return 0;
}

int gnrc_send_load_cp(gnrc_send_crt_t *crt, unsigned crt_max,
                      gnrc_send_cp_t *cp, unsigned cp_max)
{
    (void)crt_max; (void)cp_max;
    crt->parent = _ta_meta[0].crt;
    crt->extn = (gnrc_send_x509_extn_t){ .ip_block_numof = 0 };
    crt->crt_inmem.zero = 0;
    crt->crt_inmem.size = X509_EC_CERT_SIZE_RTR;
    crt->crt_inmem.type = NDP_CERT_TYPE_DER;
    crt->crt_inmem.buffer = test_ec_rtr_certificate;
    gnrc_send_pk_identifier(&crt->pk_ident,
                            test_ec_pk_rtr, X509_EC_PK_SIZE_RTR);
    gnrc_send_name_identifier(&crt->issuer_ident,
                              test_ec_issuer_name_rtr, X509_ISSUER_NAME_SIZE_RTR);
    gnrc_send_name_identifier(&crt->subject_ident,
                              test_ec_subject_name_rtr, X509_SUBJECT_NAME_SIZE_RTR);
    cp->crt = crt;
    cp->num_comp = 2; /* TA counts as component */
    cp->ta = &_ta_meta[0];
    return 1;
}

int gnrc_send_load_keys(gnrc_send_key_t *key, void *key_buf, size_t key_buf_size)
{
    assert(key_buf_size >= sizeof(test_ec_priv_rtr));
    memcpy(key_buf, test_ec_priv_rtr, sizeof(test_ec_priv_rtr));
    key->pubkey = test_ec_pk_rtr;
    key->pubkey_size = X509_EC_PK_SIZE_RTR;
    key->key = key_buf;
    key->key_size = ARRAY_SIZE(test_ec_priv_rtr);
    return gnrc_send_parse_keys(key->key, key->key_size, &key->pk) == 0 ? 1 : 0;
}
/** @} */

static void set_up(void)
{
    gnrc_pktbuf_init();
}

static void test_opt_cga_params_build_verify__success(void)
{
    gnrc_pktsnip_t *pkt;
    ndp_opt_cga_params_t *opt;
    size_t size = _ceil8(sizeof(*opt) + X509_EC_PK_SIZE_RTR);

    TEST_ASSERT(gnrc_pktbuf_is_empty());
    pkt = gnrc_send_cga_params_build(test_cga_rtr, test_netif, NULL);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT_PKT_ALLOCATION(pkt, size, NETDEV_TYPE_UNKNOWN, NULL);
    opt = (ndp_opt_cga_params_t *)pkt->data;
    TEST_ASSERT_NDP_OPT(opt, size, NDP_OPT_CGA_PARAMETERS);
    TEST_ASSERT(!memcmp(&opt[1], test_ec_pk_rtr, X509_EC_PK_SIZE_RTR));
    TEST_ASSERT_EQUAL_INT(0, ipv6_cga_verify(test_cga_rtr, &opt->cga_par, test_ec_pk_rtr, X509_EC_PK_SIZE_RTR));
    gnrc_pktbuf_release(pkt);
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_opt_cga_params_build_verify__failure(void)
{
    gnrc_pktsnip_t *pkt;
    ndp_opt_cga_params_t *opt;
    size_t size = _ceil8(sizeof(*opt) + X509_EC_PK_SIZE_RTR);

    TEST_ASSERT(gnrc_pktbuf_is_empty());
    pkt = gnrc_send_cga_params_build(test_cga_rtr, test_netif, NULL);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT_PKT_ALLOCATION(pkt, size, NETDEV_TYPE_UNKNOWN, NULL);
    opt = (ndp_opt_cga_params_t *)pkt->data;
    TEST_ASSERT_NDP_OPT(opt, size, NDP_OPT_CGA_PARAMETERS);
    TEST_ASSERT(!memcmp(&opt[1], test_ec_pk_rtr, X509_EC_PK_SIZE_RTR));
    opt->cga_par.collision_count = 7;
    TEST_ASSERT(0 > ipv6_cga_verify(test_cga_rtr, &opt->cga_par, test_ec_pk_rtr, X509_EC_PK_SIZE_RTR));
    gnrc_pktbuf_release(pkt);
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_opt_trust_anchor_build__success(void)
{
    gnrc_pktsnip_t *pkt;
    ndp_opt_ta_t *opt;
    size_t size = _ceil8(sizeof(*opt) + X509_SUBJECT_NAME_SIZE_TA);

    TEST_ASSERT(gnrc_pktbuf_is_empty());
    pkt = gnrc_send_trust_anchor_build(&_ta_meta[0], NDP_TA_TYPE_DER, NULL);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT_PKT_ALLOCATION(pkt, size, NETDEV_TYPE_UNKNOWN, NULL);
    opt = (ndp_opt_ta_t *)pkt->data;
    TEST_ASSERT_NDP_OPT(opt, size, NDP_OPT_TRUST_ANCHOR);
    gnrc_pktbuf_release(pkt);
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_opt_signature_build_verify__success(void)
{
    gnrc_pktsnip_t *pkt;
    ndp_opt_sig_t *opt;

    TEST_ASSERT(gnrc_pktbuf_is_empty());
    gnrc_pktsnip_t *sllao = gnrc_ndp_opt_sl2a_build(test_src_l2, sizeof(test_src_l2), NULL);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT_PKT_ALLOCATION(sllao, sllao->size , NETDEV_TYPE_UNKNOWN, NULL);

    gnrc_pktsnip_t *rtr_sol = gnrc_ndp_rtr_sol_build(sllao);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT_PKT_ALLOCATION(rtr_sol, rtr_sol->size, GNRC_NETTYPE_ICMPV6, sllao);

    pkt = gnrc_send_signature_build(rtr_sol, test_netif, test_cga_rtr, &test_dst);
    TEST_ASSERT_PKT_ALLOCATION(pkt, pkt->size, NETDEV_TYPE_UNKNOWN, NULL);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    opt = (ndp_opt_sig_t *)pkt->data;
    TEST_ASSERT_NDP_OPT(opt, pkt->size, NDP_OPT_SIGNATURE);
    TEST_ASSERT_NOT_NULL((sllao = gnrc_pkt_append(sllao, pkt)));

    rtr_sol = gnrc_pktbuf_start_write(rtr_sol);
    TEST_ASSERT_EQUAL_INT(0, gnrc_pktbuf_merge(rtr_sol));
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    {
        ndp_rtr_sol_t *rs = rtr_sol->data;
        ndp_opt_t *o;
        FOREACH_OPT(rs, o, rtr_sol->size) {
            if (o->type == NDP_OPT_SIGNATURE) {
                opt = (ndp_opt_sig_t *)o;
                break;
            }
        }

    }
    TEST_ASSERT_EQUAL_INT(0, gnrc_send_signature_check(rtr_sol->data, opt,
                                                       test_ec_pk_rtr, X509_EC_PK_SIZE_RTR,
                                                       test_cga_rtr, &test_dst));
    gnrc_pktbuf_release(rtr_sol);
    //TEST_ASSERT(gnrc_pktbuf_is_empty()); HELP!
}

static void test_opt_signature_build_verify__failure(void)
{
    gnrc_pktsnip_t *pkt;
    ndp_opt_sig_t *opt;

    TEST_ASSERT(gnrc_pktbuf_is_empty());
    gnrc_pktsnip_t *sllao = gnrc_ndp_opt_sl2a_build(test_src_l2, sizeof(test_src_l2), NULL);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT_PKT_ALLOCATION(sllao, sllao->size, NETDEV_TYPE_UNKNOWN, NULL);

    gnrc_pktsnip_t *rtr_sol = gnrc_ndp_rtr_sol_build(sllao);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT_PKT_ALLOCATION(rtr_sol, rtr_sol->size, GNRC_NETTYPE_ICMPV6, sllao);

    pkt = gnrc_send_signature_build(rtr_sol, test_netif, test_cga_rtr, &test_dst);
    TEST_ASSERT_PKT_ALLOCATION(pkt, pkt->size, NETDEV_TYPE_UNKNOWN, NULL);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    opt = (ndp_opt_sig_t *)pkt->data;
    TEST_ASSERT_NDP_OPT(opt, pkt->size, NDP_OPT_SIGNATURE);
    TEST_ASSERT_NOT_NULL((sllao = gnrc_pkt_append(sllao, pkt)));

    rtr_sol = gnrc_pktbuf_start_write(rtr_sol);
    TEST_ASSERT_EQUAL_INT(0, gnrc_pktbuf_merge(rtr_sol));
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    {
        ndp_rtr_sol_t *rs = rtr_sol->data;
        ndp_opt_t *o;
        FOREACH_OPT(rs, o, rtr_sol->size) {
            if (o->type == NDP_OPT_SIGNATURE) {
                opt = (ndp_opt_sig_t *)o;
                break;
            }
        }

    }
    ipv6_addr_t bad_src = test_src;
    TEST_ASSERT_EQUAL_INT(-GNRC_SEND_STATUS_SIGNATURE_FAIL,
                          gnrc_send_signature_check(rtr_sol->data, opt,
                                                    test_ec_pk_rtr, X509_EC_PK_SIZE_RTR,
                                                    &bad_src, &test_dst));
    gnrc_pktbuf_release(rtr_sol);
    //TEST_ASSERT(gnrc_pktbuf_is_empty()); HELP!
}

static void test_opt_nonce_build__success(void)
{
    gnrc_pktsnip_t *pkt;
    ndp_opt_nonce_t *opt;
    size_t size = _ceil8(sizeof(*opt) + GNRC_SEND_NONCE_SIZE);

    TEST_ASSERT(gnrc_pktbuf_is_empty());
    pkt = gnrc_send_nonce_build(test_nonce, sizeof(test_nonce), NULL);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT_PKT_ALLOCATION(pkt, size, NETDEV_TYPE_UNKNOWN, NULL);
    opt = (ndp_opt_nonce_t *)pkt->data;
    TEST_ASSERT_NDP_OPT(opt, size, NDP_OPT_NONCE);
    gnrc_pktbuf_release(pkt);
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static Test *tests_gnrc_send_build(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_opt_cga_params_build_verify__success),
        new_TestFixture(test_opt_cga_params_build_verify__failure),
        new_TestFixture(test_opt_trust_anchor_build__success),
        new_TestFixture(test_opt_signature_build_verify__success),
        new_TestFixture(test_opt_signature_build_verify__failure),
        new_TestFixture(test_opt_nonce_build__success),
    };
    EMB_UNIT_TESTCALLER(tests, set_up, NULL, fixtures);
    return (Test *)&tests;
}

static void test_x509_load__success(void)
{
    gnrc_send_x509_crt_t x509 = { 0 };
    gnrc_send_x509_extn_t extn_ctx = { 0 };
    TEST_ASSERT_EQUAL_INT(0, gnrc_send_load_x509(NULL, (void *)test_ec_rtr_certificate,
                                                 X509_EC_CERT_SIZE_RTR,
                                                 &x509, &extn_ctx));
}

static void test_x509_verify__success(void)
{
    gnrc_send_x509_extn_t extn_ctx = { 0 };
    size_t size;
    void *buf;
    gnrc_send_x509_crt_t *x509 = gnrc_send_x509_acquire(&size, &buf);
    TEST_ASSERT_NOT_NULL(x509);
    TEST_ASSERT_EQUAL_INT(0, gnrc_send_load_x509(NULL, (void *)test_ec_ta_certificate,
                                                 X509_EC_CERT_SIZE_TA,
                                                 &x509[0], &extn_ctx));
    TEST_ASSERT_EQUAL_INT(0, gnrc_send_load_x509(NULL, (void *)test_ec_rtr_certificate,
                                                 X509_EC_CERT_SIZE_RTR,
                                                 &x509[1], &extn_ctx));
    TEST_ASSERT_EQUAL_INT(0, gnrc_send_x509_verify(&x509[1], &x509[0]));
    gnrc_send_x509_release();
}

static Test *tests_gnrc_send_x509(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_x509_load__success),
        new_TestFixture(test_x509_verify__success),
    };
    EMB_UNIT_TESTCALLER(tests, set_up, NULL, fixtures);
    return (Test *)&tests;
}

static void test_gnrc_send_ta_iterator(void)
{
    gnrc_send_ta_iter_t iter = { .ta = NULL };
    TEST_ASSERT(&_ta_meta[0] == gnrc_send_ta_iterator(&iter));
    TEST_ASSERT(NULL == gnrc_send_ta_iterator(&iter));
}

static void test_gnrc_send_get_ta_by_name__success(void)
{
    TEST_ASSERT(&_ta_meta[0] ==
                gnrc_send_get_ta_by_name(test_ec_subject_name_ta,
                                         X509_SUBJECT_NAME_SIZE_TA));
}

static void test_gnrc_send_get_ta_by_name__failure(void)
{
    TEST_ASSERT_NULL(gnrc_send_get_ta_by_name(test_ec_subject_name_ta + 1,
                                              X509_SUBJECT_NAME_SIZE_TA - 1));
}

static void test_gnrc_send_get_ta_by_subject_ident__success(void)
{
    gnrc_send_ident_t ident;
    gnrc_send_name_identifier(&ident,
                              test_ec_subject_name_ta,
                              X509_SUBJECT_NAME_SIZE_TA);
    TEST_ASSERT(&_ta_meta[0] ==
                gnrc_send_get_ta_by_subject_ident(&ident));
}

static void test_gnrc_send_get_ta_by_subject_ident__failure(void)
{
    gnrc_send_ident_t ident;
    gnrc_send_name_identifier(&ident,
                              test_ec_subject_name_ta + 1,
                              X509_SUBJECT_NAME_SIZE_TA - 1);
    TEST_ASSERT_NULL(gnrc_send_get_ta_by_subject_ident(&ident));
}

static Test *test_gnrc_send_ta(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_gnrc_send_ta_iterator),
        new_TestFixture(test_gnrc_send_get_ta_by_name__success),
        new_TestFixture(test_gnrc_send_get_ta_by_name__failure),
        new_TestFixture(test_gnrc_send_get_ta_by_subject_ident__success),
        new_TestFixture(test_gnrc_send_get_ta_by_subject_ident__failure),
    };
    EMB_UNIT_TESTCALLER(tests, set_up, NULL, fixtures);
    return (Test *)&tests;
}

static void test_gnrc_send_crt_iterator(void)
{
    gnrc_send_crt_iter_t iter = { .crt = NULL };
    TEST_ASSERT_NOT_NULL(gnrc_send_crt_iterator(&iter));
    TEST_ASSERT_NOT_NULL(gnrc_send_crt_iterator(&iter));
    TEST_ASSERT_NULL(gnrc_send_crt_iterator(&iter));
}

static void test_gnrc_send_get_crt_by_subject_ident__success(void)
{
    gnrc_send_ident_t ident;
    gnrc_send_name_identifier(&ident,
                              test_ec_subject_name_ta,
                              X509_SUBJECT_NAME_SIZE_TA);
    TEST_ASSERT(_ta_meta[0].crt == gnrc_send_get_crt_by_subject_ident(&ident));
    gnrc_send_name_identifier(&ident,
                              test_ec_subject_name_rtr,
                              X509_SUBJECT_NAME_SIZE_RTR);
    TEST_ASSERT(_cp_meta[0].crt == gnrc_send_get_crt_by_subject_ident(&ident));
}

static void test_gnrc_send_get_crt_by_subject_ident__failure(void)
{
    gnrc_send_ident_t ident;
    gnrc_send_name_identifier(&ident,
                              test_ec_subject_name_ta + 1,
                              X509_SUBJECT_NAME_SIZE_TA - 1);
    TEST_ASSERT_NULL(gnrc_send_get_crt_by_subject_ident(&ident));
    gnrc_send_name_identifier(&ident,
                              test_ec_subject_name_rtr + 1,
                              X509_SUBJECT_NAME_SIZE_RTR - 1);
    TEST_ASSERT_NULL(gnrc_send_get_crt_by_subject_ident(&ident));
}

static void test_gnrc_send_get_crt_by_pk_ident__success(void)
{
    gnrc_send_ident_t ident;
    gnrc_send_pk_identifier(&ident,
                            test_ec_pk_ta,
                            X509_EC_PK_SIZE_TA);
    TEST_ASSERT(_ta_meta[0].crt == gnrc_send_get_crt_by_pk_ident(&ident));
    gnrc_send_pk_identifier(&ident,
                            test_ec_pk_rtr,
                            X509_EC_PK_SIZE_RTR);
    TEST_ASSERT(_cp_meta[0].crt == gnrc_send_get_crt_by_pk_ident(&ident));
}

static void test_gnrc_send_get_crt_by_pk_ident__failure(void)
{
    gnrc_send_ident_t ident;
    gnrc_send_pk_identifier(&ident,
                            test_ec_pk_ta + 1,
                            X509_EC_PK_SIZE_TA - 1);
    TEST_ASSERT_NULL(gnrc_send_get_crt_by_pk_ident(&ident));
    gnrc_send_pk_identifier(&ident,
                            test_ec_pk_rtr + 1,
                            X509_EC_PK_SIZE_RTR - 1);
    TEST_ASSERT_NULL(gnrc_send_get_crt_by_pk_ident(&ident));
}

static Test *test_gnrc_send_crt(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_gnrc_send_crt_iterator),
        new_TestFixture(test_gnrc_send_get_crt_by_subject_ident__success),
        new_TestFixture(test_gnrc_send_get_crt_by_subject_ident__failure),
        new_TestFixture(test_gnrc_send_get_crt_by_pk_ident__success),
        new_TestFixture(test_gnrc_send_get_crt_by_pk_ident__failure),
    };
    EMB_UNIT_TESTCALLER(tests, set_up, NULL, fixtures);
    return (Test *)&tests;
}

static void test_gnrc_send_cp_iterator(void)
{
    gnrc_send_cp_iter_t iter = { .cp = NULL };
    TEST_ASSERT(&_cp_meta[0] == gnrc_send_cp_iterator(&iter));
    TEST_ASSERT(NULL == gnrc_send_cp_iterator(&iter));
}

static void test_gnrc_send_get_cp_by_pk_ident__success(void)
{
    gnrc_send_ident_t ident;
    gnrc_send_pk_identifier(&ident,
                            test_ec_pk_rtr,
                            X509_EC_PK_SIZE_RTR);
    TEST_ASSERT(&_cp_meta[0] == gnrc_send_get_cp_by_pk_ident(&ident, _cp_meta[0].ta));
}

static void test_gnrc_send_get_cp_by_pk_ident__failure(void)
{
    gnrc_send_ident_t ident;
    gnrc_send_pk_identifier(&ident,
                            test_ec_pk_rtr + 1,
                            X509_EC_PK_SIZE_RTR - 1);
    TEST_ASSERT_NULL(gnrc_send_get_cp_by_pk_ident(&ident, _cp_meta[0].ta));
}

static Test *test_gnrc_send_cp(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_gnrc_send_cp_iterator),
        new_TestFixture(test_gnrc_send_get_cp_by_pk_ident__success),
        new_TestFixture(test_gnrc_send_get_cp_by_pk_ident__failure),
    };
    EMB_UNIT_TESTCALLER(tests, set_up, NULL, fixtures);
    return (Test *)&tests;
}

static void test_cp_sol_send__timeout(void)
{
    msg_t msg;
    while (msg_try_receive(&msg) == 1) {
        /* empty message queue */
    }
    TEST_ASSERT(gnrc_pktbuf_is_empty());
    gnrc_pktsnip_t *rtr_adv = gnrc_ndp_rtr_adv_build(255, 0,
                                                     32000, 100000,
                                                     4000, NULL);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT_PKT_ALLOCATION(rtr_adv, rtr_adv->size, GNRC_NETTYPE_ICMPV6, NULL);
    gnrc_pktsnip_t *rtr_hdr = gnrc_ipv6_hdr_build(rtr_adv, &test_dst, test_cga_rtr);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT_PKT_ALLOCATION(rtr_hdr, rtr_hdr->size, GNRC_NETTYPE_IPV6, rtr_adv);
    gnrc_send_ident_t rtr_ident = {{ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10,
                                     0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x20 }};
    gnrc_send_cache_cps_t * cps = gnrc_send_new_cps_ctx(test_netif, rtr_hdr->data, &rtr_ident,
                                                        rtr_adv->data, rtr_adv->size);
    TEST_ASSERT_NOT_NULL(cps);
    TEST_ASSERT_EQUAL_INT(0, gnrc_send_cp_sol_send(test_netif, test_cga_rtr, &test_dst, SEND_CPS_ALL_COMP, cps));
    gnrc_send_set_cps_timeout(cps, cps->timeout_ms);
    gnrc_send_release_cps_ctx(cps);
    uint32_t cps_timeout_ms = 0;
    uint32_t tolerance_ms = 100;
    while (cps_timeout_ms < SEND_CPS_RETRY_MAX_MS) {
        if (ztimer_msg_receive_timeout(ZTIMER_MSEC, &msg, cps_timeout_ms + tolerance_ms) < 0) {
            if (cps_timeout_ms * 2 >= SEND_CPS_RETRY_MAX_MS) {
                break;
            }
            TEST_ASSERT_MESSAGE(false, "CPS not received within timeout");
            continue;
        }
        if (msg.type == GNRC_NETAPI_MSG_TYPE_SND) {
            TEST_ASSERT_EQUAL_INT(GNRC_NETAPI_MSG_TYPE_SND, msg.type);
            gnrc_pktsnip_t *pkt = msg.content.ptr;
            TEST_ASSERT_PKT_ALLOCATION(pkt, pkt->size, GNRC_NETTYPE_NETIF,
                                       pkt->next);
            TEST_ASSERT_PKT_ALLOCATION(pkt->next, pkt->next->size, GNRC_NETTYPE_IPV6,
                                       pkt->next->next);
            TEST_ASSERT_PKT_ALLOCATION(pkt->next->next, pkt->next->next->size, GNRC_NETTYPE_ICMPV6,
                                       pkt->next->next->next);
            ndp_cp_sol_t *cp_sol = pkt->next->next->data;
            TEST_ASSERT_EQUAL_INT(0, cp_sol->code);
            TEST_ASSERT_EQUAL_INT(SEND_CPS_ALL_COMP, byteorder_ntohs(cp_sol->comp));
            TEST_ASSERT_EQUAL_INT(ICMPV6_CP_SOL, cp_sol->type);
            if (cps_timeout_ms == 0) {
                TEST_ASSERT_EQUAL_INT(SEND_CPS_RETRY_MS, cps->timeout_ms);
            }
            else {
                TEST_ASSERT_EQUAL_INT(cps_timeout_ms * 2, cps->timeout_ms);
            }
            cps_timeout_ms = cps->timeout_ms;
            gnrc_pktbuf_release(pkt);
        }
        else {
            TEST_ASSERT_MESSAGE(false, "Unexpected message type");
        }
    }
    TEST_ASSERT_EQUAL_INT(-1, msg_try_receive(&msg));
    TEST_ASSERT_NULL(cps->netif);
    gnrc_pktbuf_release(rtr_hdr);
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_cp_adv_send__all_nodes_mc(void)
{
    msg_t msg;
    while (msg_try_receive(&msg) == 1) {
        /* empty message queue */
    }
    TEST_ASSERT(gnrc_pktbuf_is_empty());
    gnrc_send_cache_cpa_t *cpa = gnrc_send_new_cpa_ctx(test_netif,
                                                       &ipv6_addr_all_nodes_link_local,
                                                       0, &_cp_meta[0]);
    TEST_ASSERT_NOT_NULL(cpa);
    TEST_ASSERT_EQUAL_INT(0, gnrc_send_cp_adv_send(test_netif, test_cga_rtr, &test_dst,
                                                   cpa->hi_comp, cpa->identifier, &_cp_meta[0],
                                                   cpa, NULL));
    cpa->hi_comp--;
    gnrc_send_set_cpa_timeout(cpa, MS_PER_SEC / SEND_CPA_RATE_MAX);
    gnrc_send_release_cpa_ctx(cpa);
    msg_receive(&msg);
    if (msg.type == GNRC_NETAPI_MSG_TYPE_SND) {
        TEST_ASSERT_EQUAL_INT(GNRC_NETAPI_MSG_TYPE_SND, msg.type);
        gnrc_pktsnip_t *pkt = msg.content.ptr;
        TEST_ASSERT_PKT_ALLOCATION(pkt, pkt->size, GNRC_NETTYPE_NETIF,
                                   pkt->next);
        TEST_ASSERT_PKT_ALLOCATION(pkt->next, pkt->next->size, GNRC_NETTYPE_IPV6,
                                   pkt->next->next);
        TEST_ASSERT_PKT_ALLOCATION(pkt->next->next, pkt->next->next->size, GNRC_NETTYPE_ICMPV6,
                                   pkt->next->next->next);
        ndp_cp_adv_t *cp_adv = pkt->next->next->data;
        TEST_ASSERT_EQUAL_INT(0, cp_adv->code);
        TEST_ASSERT_EQUAL_INT(0, byteorder_ntohs(cp_adv->ident));
        TEST_ASSERT_EQUAL_INT(2, byteorder_ntohs(cp_adv->all_comp));
        TEST_ASSERT_EQUAL_INT(ICMPV6_CP_ADV, cp_adv->type);
        TEST_ASSERT_PKT_ALLOCATION(pkt->next->next->next, pkt->next->next->next->size, GNRC_NETTYPE_UNDEF,
                                   pkt->next->next->next->next); /* Trust Anchor */
        TEST_ASSERT_PKT_ALLOCATION(pkt->next->next->next->next, pkt->next->next->next->next->size, GNRC_NETTYPE_UNDEF,
                                   pkt->next->next->next->next->next); /* Certificate */
        gnrc_pktbuf_release(pkt);
    }
    else {
        TEST_ASSERT_MESSAGE(false, "Unexpected message type");
    }
    TEST_ASSERT_EQUAL_INT(-1, msg_try_receive(&msg));
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static Test *tests_gnrc_send_send(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_cp_sol_send__timeout),
        new_TestFixture(test_cp_adv_send__all_nodes_mc),
    };
    EMB_UNIT_TESTCALLER(tests, set_up, NULL, fixtures);
    return (Test *)&tests;
}

int main(void)
{
    TESTS_START();
    init_pkt_handler();
    init_test_netdev_thread();
    TESTS_RUN(tests_gnrc_send_build());
    TESTS_RUN(test_gnrc_send_ta());
    TESTS_RUN(test_gnrc_send_crt());
    TESTS_RUN(test_gnrc_send_cp());
    TESTS_RUN(tests_gnrc_send_x509());
    TESTS_RUN(tests_gnrc_send_send());
    TESTS_END();

    return 0;
}

#define MSG_QUEUE_SIZE          (2)

static char test_netif_stack[THREAD_STACKSIZE_DEFAULT];
static msg_t msg_queue_main[MSG_QUEUE_SIZE];
static gnrc_netreg_entry_t netreg_entry;
static netdev_test_t dev;
static msg_bus_entry_t mbus_entry;

static int _test_netif_send(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    (void)netif;
    gnrc_pktbuf_release(pkt);
    return 0;
}

static gnrc_pktsnip_t *_test_netif_recv(gnrc_netif_t *netif)
{
    (void)netif;
    return NULL;
}

static int _test_netif_set(gnrc_netif_t *netif, const gnrc_netapi_opt_t *opt)
{
    (void)netif;
    (void)opt;
    return -ENOTSUP;
}

static const gnrc_netif_ops_t _test_netif_ops = {
    .init = gnrc_netif_default_init,
    .send = _test_netif_send,
    .recv = _test_netif_recv,
    .get = gnrc_netif_get_from_netdev,
    .set = _test_netif_set,
};

static int _netdev_test_address_long(netdev_t *dev, void *value, size_t max_len)
{
    (void)dev;
    expect(max_len >= sizeof(test_src_l2));
    memcpy(value, test_src_l2, sizeof(test_src_l2));
    return sizeof(test_src_l2);
}

static int _netdev_test_proto(netdev_t *dev, void *value, size_t max_len)
{
    (void)dev;
     expect(max_len == sizeof(gnrc_nettype_t));
     *((gnrc_nettype_t *)value) = GNRC_NETTYPE_UNDEF;
     return sizeof(gnrc_nettype_t);
}

static int _netdev_test_src_len(netdev_t *dev, void *value, size_t max_len)
{
    (void)dev;
     expect(max_len == sizeof(uint16_t));
     *((uint16_t *)value) = sizeof(test_src_l2);
     return sizeof(uint16_t);
}

static int _netdev_test_max_pdu_size(netdev_t *dev, void *value, size_t max_len)
{
    (void)dev;
     expect(max_len == sizeof(uint16_t));
     *((uint16_t *)value) = 100U;
     return sizeof(uint16_t);
}

static int _netdev_test_device_type(netdev_t *dev, void *value, size_t max_len)
{
    (void)dev;
     expect(max_len == sizeof(uint16_t));
     *((uint16_t *)value) = NETDEV_TYPE_IEEE802154;
     return sizeof(uint16_t);
}

static void init_pkt_handler(void)
{
    msg_init_queue(msg_queue_main, MSG_QUEUE_SIZE);
    gnrc_netreg_entry_init_pid(&netreg_entry, GNRC_NETREG_DEMUX_CTX_ALL,
                               thread_getpid());
    gnrc_netreg_register(GNRC_NETTYPE_NDP, &netreg_entry);
    netdev_test_setup(&dev, NULL);
    netdev_test_set_get_cb(&dev, NETOPT_ADDRESS_LONG,
                           _netdev_test_address_long);
    netdev_test_set_get_cb(&dev, NETOPT_PROTO, _netdev_test_proto);
    netdev_test_set_get_cb(&dev, NETOPT_SRC_LEN, _netdev_test_src_len);
    netdev_test_set_get_cb(&dev, NETOPT_MAX_PDU_SIZE,
                           _netdev_test_max_pdu_size);
    netdev_test_set_get_cb(&dev, NETOPT_DEVICE_TYPE, _netdev_test_device_type);

}

static void init_test_netdev_thread(void)
{
    int res = gnrc_netif_create(&_netif, test_netif_stack, sizeof(test_netif_stack),
                                GNRC_NETIF_PRIO, "test-netif",
                                &dev.netdev.netdev, &_test_netif_ops);
    test_netif = &_netif;
    TEST_ASSERT_MESSAGE(res == 0, "Unable to start test interface");
    msg_t mvalid;
    msg_bus_attach(gnrc_netif_get_bus(&_netif, GNRC_NETIF_BUS_IPV6), &mbus_entry);
    msg_bus_subscribe(&mbus_entry, GNRC_IPV6_EVENT_ADDR_VALID);
    while (1) {
        uint32_t timeout = test_netif->ipv6.retrans_time * 2 * MS_PER_SEC;
        if ((res = ztimer_msg_receive_timeout(ZTIMER_USEC, &mvalid, timeout)) == -ETIME) {
            TEST_ASSERT_MESSAGE(1, "Valid address timeout");
            exit(1);
        }
        if (!gnrc_netif_ipv6_addr_is_cga(test_netif, (const ipv6_addr_t *)mvalid.content.ptr)) {
            continue;
        }
        test_cga_rtr = mvalid.content.ptr;
        break;
    }
    msg_bus_unsubscribe(&mbus_entry, GNRC_IPV6_EVENT_ADDR_VALID);
    msg_bus_detach(gnrc_netif_get_bus(&_netif, GNRC_NETIF_BUS_IPV6), &mbus_entry);
}
