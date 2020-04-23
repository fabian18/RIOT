/*
 * Copyright (C) 2020 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup sys_auto_init_gnrc_netif
 * @{
 *
 * @file
 * @brief   Auto initialization for at86rf233 network interfaces
 *
 * @author  Fabian Hüßler <fabian.huessler@ovgu.com>
 */
#ifdef MODULE_AT86RF233

#include "log.h"
#include "board.h"
#include "net/gnrc/netif/ieee802154.h"
#ifdef MODULE_GNRC_LWMAC
#include "net/gnrc/lwmac/lwmac.h"
#endif
#ifdef MODULE_GNRC_GOMACH
#include "net/gnrc/gomach/gomach.h"
#endif
#include "net/gnrc.h"

#include "at86rf233.h"
#include "at86rf233_params.h"

/**
 * @brief   Define stack parameters for the MAC layer thread
 * @{
 */
#ifndef AT86RF233_MAC_STACKSIZE
#define AT86RF233_MAC_STACKSIZE     (THREAD_STACKSIZE_DEFAULT)
#endif
#ifndef AT86RF233_MAC_PRIO
#define AT86RF233_MAC_PRIO          (GNRC_NETIF_PRIO)
#endif
/** @} */

static at86rf233_t _at86rf233_devs[AT86RF233_NUM_OF];
static gnrc_netif_t _netif[AT86RF233_NUM_OF];
static char _at86rf233_stacks[AT86RF233_NUM_OF][AT86RF233_MAC_STACKSIZE];

static inline void _setup_netif(gnrc_netif_t *netif, void* netdev, void* stack,
                                int prio)
{
    if (netif == NULL || netdev == NULL) {
        return;
    }

#if defined(MODULE_GNRC_GOMACH)
        gnrc_netif_gomach_create(netif, stack,
                                 AT86RF233_MAC_STACKSIZE,
                                 prio, "at86rf233-gomach",
                                 netdev);
#elif defined(MODULE_GNRC_LWMAC)
        gnrc_netif_lwmac_create(netif, stack,
                                AT86RF233_MAC_STACKSIZE,
                                prio, "at86rf233-lwmac",
                                netdev);
#else
        gnrc_netif_ieee802154_create(netif, stack,
                                     AT86RF233_MAC_STACKSIZE,
                                     prio, "at86rf233",
                                     netdev);
#endif
}

void auto_init_at86rf233(void)
{
    for (unsigned i = 0; i < AT86RF233_NUM_OF; i++) {
        at86rf233_setup(&_at86rf233_devs[i], &at86rf233_params[i]);
        _setup_netif(&_netif[i],
                     &_at86rf233_devs[i].base.netdev,
                     &_at86rf233_stacks[i],
                     AT86RF233_MAC_PRIO);
    }
}

#else
typedef int dont_be_pedantic;
#endif /* MODULE_AT86RF233 */
/** @} */
