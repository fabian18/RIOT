/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     unittests
 * @{
 *
 * @file
 * @brief       Unittests for Cryptographically Generated Addresses (CGA) RFC3972
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 * @}
 */

#include <errno.h>
#include <stdlib.h>

#include "embUnit.h"
#include "net/ipv6/cga.h"

#ifndef CONFIG_TEST_IPV6_CGA_SEC_MAX
#define CONFIG_TEST_IPV6_CGA_SEC_MAX IPV6_CGA_SEC_DEFAULT
#endif

#define IPV6_PREFIX {0x20, 0x01, 0x0d, 0xb8, 0x3c, 0x4d, 0x00, 0x15}

#define PUBKEYINFO_DER {0x30, 0x56,                                             \
                        0x30, 0x10,                                             \
                        0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,   \
                        0x06, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x0A,               \
                        0x03, 0x42, 0x00, 0x04, 0x48, 0xD1, 0x10, 0x48,         \
                        0x2A, 0x52, 0xB0, 0xBF, 0x13, 0x42, 0x20, 0x95,         \
                        0x2C, 0x92, 0x2D, 0x25, 0xBD, 0x8B, 0xA4, 0xF7,         \
                        0xA1, 0x95, 0x7F, 0x7E, 0xFF, 0x49, 0x4D, 0x14,         \
                        0x4B, 0x69, 0xB2, 0xCA, 0xE2, 0xF0, 0x77, 0x86,         \
                        0x6E, 0x9B, 0x27, 0xCD, 0x92, 0xE5, 0xCD, 0xBA,         \
                        0xA3, 0x5E, 0x18, 0x8F, 0x40, 0xA9, 0xF8, 0xB5,         \
                        0x7D, 0x8F, 0x10, 0x7D, 0x10, 0xBE, 0x6E, 0x58,         \
                        0x48, 0xE8, 0xF0, 0xE1}

typedef struct {
    ipv6_cga_parameters_t parameters;
    uint8_t pubkeyinfo[sizeof((uint8_t[])PUBKEYINFO_DER)];
} test_ipv6_cga_parameters_t;

static ipv6_addr_t _cga;
static test_ipv6_cga_parameters_t _cga_parameters = { .pubkeyinfo = PUBKEYINFO_DER };

static void _cga_init(void)
{
    static const uint8_t _prefix[] = IPV6_PREFIX;
    memset(_cga.u8, 0, sizeof(_cga.u8));
    memcpy(_cga.u8, _prefix, sizeof(_prefix));
    memset(&_cga_parameters.parameters, 0, sizeof(_cga_parameters.parameters));
}

static void test_ipv6_cga_generate_sec0(void)
{
    _cga_init();
    ipv6_cga_generate(&_cga, &_cga_parameters.parameters,
                      _cga_parameters.pubkeyinfo, sizeof(_cga_parameters.pubkeyinfo),
                      0, 0);
}

static void test_ipv6_cga_verify(void)
{
    int verify = ipv6_cga_verify(&_cga, &_cga_parameters.parameters,
                                 _cga_parameters.pubkeyinfo, sizeof(_cga_parameters.pubkeyinfo));
    TEST_ASSERT_EQUAL_INT(verify, 0);
}

#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 1
static void test_ipv6_cga_generate_sec1(void)
{
    _cga_init();
    ipv6_cga_generate(&_cga, &_cga_parameters.parameters,
                      _cga_parameters.pubkeyinfo, sizeof(_cga_parameters.pubkeyinfo),
                      1, 0);
}
#endif
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 2
static void test_ipv6_cga_generate_sec2(void)
{
    _cga_init();
    ipv6_cga_generate(&_cga, &_cga_parameters.parameters,
                      _cga_parameters.pubkeyinfo, sizeof(_cga_parameters.pubkeyinfo),
                      2, 0);
}
#endif
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 3
static void test_ipv6_cga_generate_sec3(void)
{
    _cga_init();
    ipv6_cga_generate(&_cga, &_cga_parameters.parameters,
                      _cga_parameters.pubkeyinfo, sizeof(_cga_parameters.pubkeyinfo),
                      3, 0);
}
#endif
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 4
static void test_ipv6_cga_generate_sec4(void)
{
    _cga_init();
    ipv6_cga_generate(&_cga, &_cga_parameters.parameters,
                      _cga_parameters.pubkeyinfo, sizeof(_cga_parameters.pubkeyinfo),
                      4, 0);
}
#endif
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 5
static void test_ipv6_cga_generate_sec5(void)
{
    _cga_init();
    ipv6_cga_generate(&_cga, &_cga_parameters.parameters,
                      _cga_parameters.pubkeyinfo, sizeof(_cga_parameters.pubkeyinfo),
                      5, 0);
}
#endif
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 6
static void test_ipv6_cga_generate_sec6(void)
{
    _cga_init();
    ipv6_cga_generate(&_cga, &_cga_parameters.parameters,
                      _cga_parameters.pubkeyinfo, sizeof(_cga_parameters.pubkeyinfo),
                      6, 0);
}
#endif
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 7
static void test_ipv6_cga_generate_sec7(void)
{
    _cga_init();
    ipv6_cga_generate(&_cga, &_cga_parameters.parameters,
                      _cga_parameters.pubkeyinfo, sizeof(_cga_parameters.pubkeyinfo),
                      7, 0);
}
#endif

Test *tests_ipv6_cga_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_ipv6_cga_generate_sec0),
        new_TestFixture(test_ipv6_cga_verify),
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 1
        new_TestFixture(test_ipv6_cga_generate_sec1),
        new_TestFixture(test_ipv6_cga_verify),
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 2
        new_TestFixture(test_ipv6_cga_generate_sec2),
        new_TestFixture(test_ipv6_cga_verify),
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 3
        new_TestFixture(test_ipv6_cga_generate_sec3),
        new_TestFixture(test_ipv6_cga_verify),
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 4
        new_TestFixture(test_ipv6_cga_generate_sec4),
        new_TestFixture(test_ipv6_cga_verify),
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 5
        new_TestFixture(test_ipv6_cga_generate_sec5),
        new_TestFixture(test_ipv6_cga_verify),
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 6
        new_TestFixture(test_ipv6_cga_generate_sec6),
        new_TestFixture(test_ipv6_cga_verify),
#if CONFIG_TEST_IPV6_CGA_SEC_MAX >= 7
        new_TestFixture(test_ipv6_cga_generate_sec7),
        new_TestFixture(test_ipv6_cga_verify),
#endif
#endif
#endif
#endif
#endif
#endif
#endif
    };

    EMB_UNIT_TESTCALLER(ipv6_cga_tests, NULL, NULL, fixtures);

    return (Test *)&ipv6_cga_tests;
}

void tests_ipv6_cga(void)
{
    TESTS_RUN(tests_ipv6_cga_tests());
}
/** @} */
