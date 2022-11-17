/*
 * Copyright (C) 2022 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application to measure address validation delay
 *
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

#include <stdio.h>

#include "kernel_defines.h"
#include "shell.h"
#include "msg.h"
#include "thread.h"
#include "msg_bus.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/conf.h"
#include "ztimer.h"

static char str_ipv6_addr[IPV6_ADDR_MAX_STR_LEN];

#define MAIN_QUEUE_SIZE         (8u)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

#define MBUS_ADDR_VALID_SIZE    (8u)
#define ADDR_VALID_QUEUE_SIZE   MBUS_ADDR_VALID_SIZE
static msg_t _addr_valid_msg_queue[ADDR_VALID_QUEUE_SIZE];

static char _addr_valid_stack[THREAD_STACKSIZE_MEDIUM];

static void _mbus_sub_addr_valid(gnrc_netif_t *netif, msg_bus_entry_t *mbe)
{
    msg_bus_attach(gnrc_netif_get_bus(netif, GNRC_NETIF_BUS_IPV6), mbe);
    msg_bus_subscribe(mbe, GNRC_IPV6_EVENT_ADDR_VALID);
    msg_bus_subscribe(mbe, GNRC_IPV6_EVENT_DAD);
}

static void _mbus_unsub_addr_valid(gnrc_netif_t *netif, msg_bus_entry_t *mbe)
{
    msg_bus_unsubscribe(mbe, GNRC_IPV6_EVENT_DAD);
    msg_bus_unsubscribe(mbe, GNRC_IPV6_EVENT_ADDR_VALID);
    msg_bus_detach(gnrc_netif_get_bus(netif, GNRC_NETIF_BUS_IPV6), mbe);
}

static void *_thread_ipv6_event(void *arg)
{
    (void)arg;
    msg_init_queue(_addr_valid_msg_queue, ARRAY_SIZE(_addr_valid_msg_queue));
    static msg_bus_entry_t mbus_entry[MBUS_ADDR_VALID_SIZE];
    static gnrc_netif_t *mbus_netifs[MBUS_ADDR_VALID_SIZE];
    ztimer_now_t ipv6_event_start = ztimer_now(ZTIMER_MSEC);
    gnrc_netif_t *netif = NULL;
    msg_t m;
    for (unsigned i = 0; i < ARRAY_SIZE(mbus_entry); ) {
        if (!(netif = gnrc_netif_iter(netif))) {
            break;
        }
        _mbus_sub_addr_valid(netif, &mbus_entry[i]);
        mbus_netifs[i] = netif;
        i++;
    }
    while (1) {
        msg_receive(&m);
        netif = NULL;
        for (unsigned i = 0; i < ARRAY_SIZE(mbus_netifs); i++) {
            if (mbus_netifs[i]) {
                if (msg_is_from_bus(gnrc_netif_get_bus(mbus_netifs[i], GNRC_NETIF_BUS_IPV6), &m)) {
                    netif = mbus_netifs[i];
                    break;
                }
            }
        }
        if (!netif) {
            continue;
        }
        if (msg_bus_get_type(&m) == GNRC_IPV6_EVENT_DAD) {
            printf("%s on interface %"PRIkernel_pid
                   " is checked for being a duplicate %"PRIu32
                   " ms since main().\n",
                   ipv6_addr_to_str(str_ipv6_addr,
                                    m.content.ptr,
                                    sizeof(str_ipv6_addr)),
                   netif->pid,
                   msg_get_timestamp(&m) - ipv6_event_start);
        }
        else if (msg_bus_get_type(&m) == GNRC_IPV6_EVENT_ADDR_VALID) {
            printf("%s on interface %"PRIkernel_pid
                    " has become valid after %"PRIu32
                    " ms since main().\n",
                    ipv6_addr_to_str(str_ipv6_addr,
                                    m.content.ptr,
                                    sizeof(str_ipv6_addr)),
                    netif->pid,
                    msg_get_timestamp(&m) - ipv6_event_start);
        }
    }
    for (unsigned i = 0; i < ARRAY_SIZE(mbus_netifs); i++) {
        if (mbus_netifs[i]) {
            _mbus_unsub_addr_valid(mbus_netifs[i], &mbus_entry[i]);
        }
    }
    return NULL;
}

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, ARRAY_SIZE(_main_msg_queue));
    puts("GNRC networking benchmark example application");

    thread_create(_addr_valid_stack, sizeof(_addr_valid_stack),
                  THREAD_PRIORITY_MAIN, THREAD_CREATE_WOUT_YIELD,
                  _thread_ipv6_event, NULL,
                  "_thread_addr_valid");

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
