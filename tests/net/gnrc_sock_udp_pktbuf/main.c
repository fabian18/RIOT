/*
 * Copyright (C) 2024 ML!PA Consulting GmbH
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
 *
 * @author      Fabian Hüßler <fabian.huessler@ml-pa.com>
 *
 * @}
 */

#include <stdint.h>

#include "container.h"
#include "fmt.h"
#include "msg.h"
#include "net/gnrc/netif.h"
#include "net/ipv6/addr.h"
#include "net/sock/udp.h"
#include "shell.h"
#include "thread.h"
#include "ztimer.h"

#define ENABLE_DEBUG                    1
#include "debug.h"

#ifndef TEST_SENDER_ONLY
#define TEST_SENDER_ONLY                0
#endif

#ifndef TEST_RECEIVER_ONLY
#define TEST_RECEIVER_ONLY              0
#endif

#ifndef MAIN_QUEUE_SIZE
#define MAIN_QUEUE_SIZE                 8
#endif

#ifndef TEST_RECEIVER_BUF_SIZE
#define TEST_RECEIVER_BUF_SIZE          512
#endif

#ifndef TEST_RECEIVER_ADDRESS
#define TEST_RECEIVER_ADDRESS           "::1"
#endif

#define TEST_PORT_UDP                   12345
#define TEST_RECEIVER_PRIORITY          (THREAD_PRIORITY_MAIN - 2)
#define TEST_SENDER_PRIORITY            (THREAD_PRIORITY_MAIN - 1)

/**
 * @brief   Datatype for data received and data sent statistics
 */
typedef struct {
    ztimer_now_t last_updated;      /**< Timestamp of last statistic update */
    uint32_t pkt_cnt;               /**< Number of packets sent/received */
    size_t data_cnt_new;            /**< Amount of data sent/received in bytes */
    size_t data_cnt_old;            /**< Previous number of sent/received data since last update */
    unsigned speed;                 /**< Calculated speed on ingress or egress data */
} _stats_t;

static _stats_t _stats_rx;

static int _stats_update(_stats_t *stats, unsigned delta)
{
    int ret = 0; /* not upated */
    stats->pkt_cnt++;
    stats->data_cnt_new += delta;
    ztimer_now_t now = ztimer_now(ZTIMER_MSEC);
    if (stats->pkt_cnt == 1) {
        stats->last_updated = now;
        stats->data_cnt_old = stats->data_cnt_new;
    }
    else if ((now - stats->last_updated) > 1 * MS_PER_SEC) {
        stats->speed = (1000 * (stats->data_cnt_new - stats->data_cnt_old)) /
                       (now - stats->last_updated);
        stats->last_updated = now;
        stats->data_cnt_old = stats->data_cnt_new;
        ret = 1; /* updated */
    }
    return ret;
}

static void _state_print(const _stats_t *stats)
{
    printf("Packets: %"PRIu32", Data: %lu Bytes, Speed: %u Bytes/s\n",
           stats->pkt_cnt, (unsigned long)stats->data_cnt_new, stats->speed);
}

static int _socket_receiver(sock_udp_t *sock,
                            const gnrc_netif_t *netif)
{
    const sock_udp_ep_t loc = {
        .family = AF_INET6,
        .netif = netif ? netif->pid : SOCK_ADDR_ANY_NETIF,
        .port = TEST_PORT_UDP,
    };
    return sock_udp_create(sock, &loc, NULL, 0);
}

MAYBE_UNUSED
static int _socket_sender(sock_udp_t *sock,
                          const gnrc_netif_t *netif)
{
    const sock_udp_ep_t loc = {
        .family = AF_INET6,
        .netif = netif ? netif->pid : SOCK_ADDR_ANY_NETIF,
        .port = TEST_PORT_UDP,
    };
    return sock_udp_create(sock, &loc, NULL, 0);
}

MAYBE_UNUSED
static void *_receiver(void *arg)
{
    static uint8_t buf[TEST_RECEIVER_BUF_SIZE];
    static sock_udp_t udp_sock;
    const gnrc_netif_t *netif = arg;

    int res;
    if ((res = _socket_receiver(&udp_sock, netif)) < 0) {
        printf("Receiver socket could not be created\n");
        return NULL;
    }

    while (1) {
        sock_udp_ep_t rem;
        if ((res = sock_udp_recv(&udp_sock, buf, sizeof(buf), SOCK_NO_TIMEOUT, &rem)) <= 0) {
            DEBUG("Error receiving message %d\n", res);
        }
        printf("Received message %"PRIu32"\n", scn_u32_dec((char *)buf, res));
        if (_stats_update(&_stats_rx, res)) {
            _state_print(&_stats_rx);
        }
    }
}

MAYBE_UNUSED
static void *_sender(void *arg)
{
    static uint8_t buf[TEST_RECEIVER_BUF_SIZE];
    static uint32_t counter;

    const gnrc_netif_t *netif = arg;
    sock_udp_ep_t rem = {
        .family = AF_INET6,
        .netif = netif ? netif->pid : SOCK_ADDR_ANY_NETIF,
        .port = TEST_PORT_UDP,
    };
    ipv6_addr_t addr;
    if (!ipv6_addr_from_str(&addr, TEST_RECEIVER_ADDRESS)) {
        printf("Could not parse receiver address\n");
        return NULL;
    }
    memcpy(rem.addr.ipv6, &addr, sizeof(rem.addr.ipv6));

    while (1) {
        memset(buf, 0x23, sizeof(buf));
        fmt_u32_dec((char *)buf, counter++);
        int ret;
        if ((ret = sock_udp_send(NULL, buf, sizeof(buf), &rem)) <= 0) {
            DEBUG("Error sending message %d\n", ret);
        }
    }
    return NULL;
}

int main(void) {
    static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];
    msg_init_queue(_main_msg_queue, ARRAY_SIZE(_main_msg_queue));

#if !TEST_SENDER_ONLY
    static char _receiver_stack[THREAD_STACKSIZE_LARGE];
    thread_create(_receiver_stack, sizeof(_receiver_stack),
                  TEST_RECEIVER_PRIORITY, THREAD_CREATE_STACKTEST,
                  _receiver, NULL, "test_rx");
#endif
#if !TEST_RECEIVER_ONLY
    static char _sender_stack[THREAD_STACKSIZE_LARGE];
    thread_create(_sender_stack, sizeof(_sender_stack),
                  TEST_SENDER_PRIORITY, THREAD_CREATE_STACKTEST,
                  _sender, NULL, "test_tx");
#endif

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
