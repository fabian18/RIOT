/* Copyright (C) 2013 Alaeddine Weslati <alaeddine.weslati@inria.fr>
 *               2015 Freie Universität Berlin
 *               2015 Kaspar Schleiser <kaspar@schleiser.de>
 *               2015 Hauke Petersen <hauke.petersen@fu-berlin.de>
 *               2017 HAW Hamburg
 *               2020 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_at86rf2xx_common
 * @{
 *
 * @file
 * @brief       Implementation of common at86rf2xx netdev functions

 * @author      Alaeddine Weslati <alaeddine.weslati@inria.fr>
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Martine Lenders <mlenders@inf.fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Sebastian Meiling <s@mlng.net>
 * @author      Baptiste Clenet <bapclenet@gmail.com>
 * @author      Daniel Krebs <github@daniel-krebs.net>
 * @author      Kévin Roussel <Kevin.Roussel@inria.fr>
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 * @author      Josua Arndt <jarndt@ias.rwth-aachen.de>
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 * @}
 */

#include <assert.h>

#include "kernel_defines.h"
#include "include/at86rf2xx_common_registers.h"
#include "include/at86rf2xx_common_communication.h"
#include "include/at86rf2xx_common_internal.h"
#include "include/at86rf2xx_common_states.h"
#include "include/at86rf2xx_common_netdev.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

netopt_state_t at86rf2xx_state_to_netopt(const at86rf2xx_t *dev, uint8_t state)
{
    if (state == AT86RF2XX_STATE_P_ON) {
        return NETOPT_STATE_OFF;
    }
    if (state == AT86RF2XX_STATE_SLEEP) {
        return NETOPT_STATE_SLEEP;
    }
    if (state == AT86RF2XX_STATE_TRX_OFF) {
        return NETOPT_STATE_STANDBY;
    }
    if (at86rf2xx_is_rx_state(state)) {
        return NETOPT_STATE_RX;
    }
    if (at86rf2xx_is_tx_state(state)) {
        return NETOPT_STATE_TX;
    }
    if (state == dev->base.idle_state) {
        return NETOPT_STATE_IDLE;
    }

    assert(false);
    return NETOPT_STATE_IDLE;
}

uint8_t at86rf2xx_netopt_to_state(const at86rf2xx_t *dev, netopt_state_t state)
{
    if (state == NETOPT_STATE_OFF) {
        return AT86RF2XX_STATE_P_ON;
    }
    if (state == NETOPT_STATE_SLEEP) {
        return AT86RF2XX_STATE_SLEEP;
    }
    if (state == NETOPT_STATE_IDLE) {
        return dev->base.idle_state;
    }
    if (state == NETOPT_STATE_RX) {
        return AT86RF2XX_PHY_STATE_RX;
    }
    if (state == NETOPT_STATE_TX) {
        return AT86RF2XX_PHY_STATE_TX;
    }
    if (state == NETOPT_STATE_STANDBY) {
        return dev->base.idle_state;
    }

    assert(false);
    return dev->base.idle_state;
}

int at86rf2xx_netdev_get(at86rf2xx_t *dev, netopt_t opt, void *val, size_t max_len)
{
    /* assert that the transceiver is not sleeping,
       if register access over SPI is required */
    switch (opt) {
        case NETOPT_IS_CHANNEL_CLR:
            assert(max_len == sizeof(netopt_enable_t));
            *((netopt_enable_t *)val) = at86rf2xx_cca(dev);
            return sizeof(netopt_enable_t);

        case NETOPT_TX_POWER:
            assert(max_len == sizeof(uint16_t));
            *((uint16_t *)val) = dev->base.netdev.txpower;
            return sizeof(uint16_t);

        case NETOPT_PRELOADING:
            assert(max_len == sizeof(netopt_enable_t));
            *((netopt_enable_t *)val) =
                !!(dev->base.flags & AT86RF2XX_OPT_PRELOADING);
            return sizeof(netopt_enable_t);

        case NETOPT_PROMISCUOUSMODE:
            assert(max_len == sizeof(netopt_enable_t));
            *((netopt_enable_t *)val) =
                !!(at86rf2xx_reg_read(dev, AT86RF2XX_REG__XAH_CTRL_1) &
                   AT86RF2XX_XAH_CTRL_1_MASK__AACK_PROM_MODE);
            return sizeof(netopt_enable_t);

        case NETOPT_AUTOACK:
            assert(max_len == sizeof(netopt_enable_t));
            *((netopt_enable_t *)val) =
                !!(at86rf2xx_reg_read(dev, AT86RF2XX_REG__CSMA_SEED_1) &
                  AT86RF2XX_CSMA_SEED_1_MASK__AACK_DIS_ACK);
            return sizeof(netopt_enable_t);

        case NETOPT_RETRANS:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t *)val) = (at86rf2xx_reg_read(dev, AT86RF2XX_REG__XAH_CTRL_0) &
                                AT86RF2XX_XAH_CTRL_0_MASK__MAX_FRAME_RETRIES) >> 4;
            return sizeof(uint8_t);

        case NETOPT_STATE:
            assert(max_len >= sizeof(netopt_state_t));
            *((netopt_state_t *)val) = at86rf2xx_state_to_netopt(dev, dev->base.state);
            return sizeof(netopt_state_t);

        case NETOPT_RX_START_IRQ:
            assert(max_len == sizeof(netopt_enable_t));
            *((netopt_enable_t *)val) =
                !!(dev->base.flags & AT86RF2XX_OPT_TELL_RX_START);
            return sizeof(netopt_enable_t);

        case NETOPT_RX_END_IRQ:
            assert(max_len == sizeof(netopt_enable_t));
            *((netopt_enable_t *)val) =
                !!(dev->base.flags & AT86RF2XX_OPT_TELL_RX_END);
            return sizeof(netopt_enable_t);

        case NETOPT_TX_START_IRQ:
            assert(max_len == sizeof(netopt_enable_t));
            *((netopt_enable_t *)val) =
                !!(dev->base.flags & AT86RF2XX_OPT_TELL_TX_START);
            return sizeof(netopt_enable_t);

        case NETOPT_TX_END_IRQ:
            assert(max_len == sizeof(netopt_enable_t));
            *((netopt_enable_t *)val) =
                !!(dev->base.flags & AT86RF2XX_OPT_TELL_TX_END);
            return sizeof(netopt_enable_t);

        case NETOPT_AUTOCCA:
            assert(max_len == sizeof(netopt_enable_t));
            /* if extended operation mode */
            *((uint8_t *)val) = !!(dev->base.flags & AT86RF2XX_OPT_AUTOCCA);
            return sizeof(netopt_enable_t);

        case NETOPT_CSMA:
            assert(max_len == sizeof(netopt_enable_t));
            *((netopt_enable_t *)val) =
                (at86rf2xx_reg_read(dev, AT86RF2XX_REG__XAH_CTRL_0) &
                AT86RF2XX_XAH_CTRL_0_MASK__MAX_CSMA_RETRIES)
                    != AT86RF2XX_MAX_CSMA_RETRIES__NO_CSMA;
            return sizeof(netopt_enable_t);

        case NETOPT_CSMA_RETRIES:
            assert(max_len == sizeof(uint8_t));
            *((uint8_t *)val) = (at86rf2xx_reg_read(dev, AT86RF2XX_REG__XAH_CTRL_0) &
                                AT86RF2XX_XAH_CTRL_0_MASK__MAX_CSMA_RETRIES) >> 1;
            return sizeof(uint8_t);

        case NETOPT_CSMA_MAXBE:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t *)val) =
                (at86rf2xx_reg_read(dev, AT86RF2XX_REG__CSMA_BE) &
                AT86RF2XX_CSMA_BE_MASK__MAX_BE) >> 4;
            return sizeof(uint8_t);

        case NETOPT_CSMA_MINBE:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t *)val) =
                (at86rf2xx_reg_read(dev, AT86RF2XX_REG__CSMA_BE) &
                AT86RF2XX_CSMA_BE_MASK__MIN_BE);
            return sizeof(uint8_t);

        case NETOPT_CHANNEL_PAGE:
            assert(max_len >= sizeof(uint16_t));
            *((uint16_t *)val) = dev->base.netdev.page;
            return sizeof(uint16_t);

        case NETOPT_RANDOM:
            assert(max_len >= sizeof(uint32_t));
            at86rf2xx_get_random(dev, val, sizeof(uint32_t));
            return sizeof(uint32_t);

        default: break;
    }

    return netdev_ieee802154_get(&dev->base.netdev, opt, val, max_len);;
}

int at86rf2xx_netdev_set(at86rf2xx_t *dev, netopt_t opt, const void *val, size_t len)
{
    /* assert that the transceiver is not sleeping,
       if register access over SPI is required */
    switch (opt) {
        case NETOPT_CHANNEL:
            assert(len == sizeof(uint16_t));
            uint8_t chan = (uint8_t)(*((const uint16_t *)val));
            at86rf2xx_set_channel(dev, chan);
            return sizeof(uint16_t);

        case NETOPT_ADDRESS:
            assert(len == sizeof(network_uint16_t));
            at86rf2xx_set_addr_short(dev, val);
            return sizeof(network_uint16_t);

        case NETOPT_ADDRESS_LONG:
            assert(len == sizeof(eui64_t));
            at86rf2xx_set_addr_long(dev, val);
            return sizeof(eui64_t);

        case NETOPT_NID:
            assert(len == sizeof(uint16_t));
            at86rf2xx_set_pan(dev, *((const uint16_t *)val));
            return sizeof(uint16_t);

        case NETOPT_PRELOADING:
            assert(len == sizeof(netopt_enable_t));
            dev->base.flags = (*((netopt_enable_t *)val))
                ? dev->base.flags | AT86RF2XX_OPT_PRELOADING
                : dev->base.flags & ~AT86RF2XX_OPT_PRELOADING;
            return sizeof(netopt_enable_t);

        case NETOPT_PROMISCUOUSMODE:
            assert(len == sizeof(netopt_enable_t));
            uint8_t prom = (*((netopt_enable_t *)val))
                ? AT86RF2XX_AACK_PROM_MODE__EN
                : AT86RF2XX_AACK_PROM_MODE__DIS;
            at86rf2xx_reg_set(dev,
                              AT86RF2XX_REG__XAH_CTRL_1,
                              AT86RF2XX_XAH_CTRL_1_MASK__AACK_PROM_MODE,
                              prom);
            dev->base.flags = (prom == AT86RF2XX_AACK_PROM_MODE__EN)
                ? dev->base.flags | AT86RF2XX_OPT_PROMISCUOUS
                : dev->base.flags & ~AT86RF2XX_OPT_PROMISCUOUS;
            return sizeof(netopt_enable_t);

        case NETOPT_AUTOACK:
            assert(len == sizeof(netopt_enable_t));
            uint8_t auto_ack = (*((netopt_enable_t *)val))
                ? AT86RF2XX_AACK_DIS_ACK__YES
                : AT86RF2XX_AACK_DIS_ACK__NO;
            at86rf2xx_reg_set(dev,
                              AT86RF2XX_REG__CSMA_SEED_1,
                              AT86RF2XX_CSMA_SEED_1_MASK__AACK_DIS_ACK,
                              auto_ack);
            dev->base.flags  = (auto_ack == AT86RF2XX_AACK_DIS_ACK__YES)
                ? dev->base.flags | AT86RF2XX_OPT_AUTOACK
                : dev->base.flags & ~AT86RF2XX_OPT_AUTOACK;
            return sizeof(netopt_enable_t);

        case NETOPT_ACK_PENDING:
            assert(len == sizeof(netopt_enable_t));
            uint8_t ack_pend = (*((netopt_enable_t *)val))
                ? AT86RF2XX_AACK_SET_PD__YES
                : AT86RF2XX_AACK_SET_PD__NO;
            at86rf2xx_reg_set(dev,
                              AT86RF2XX_REG__CSMA_SEED_1,
                              AT86RF2XX_CSMA_SEED_1_MASK__AACK_SET_PD,
                              ack_pend);
            return sizeof(netopt_enable_t);

        case NETOPT_RETRANS:
            assert(len == sizeof(uint8_t));
            at86rf2xx_set_frame_max_retries(dev, *((const uint8_t *)val));
            return sizeof(uint8_t);

        case NETOPT_RX_START_IRQ:
            assert(len == sizeof(netopt_enable_t));
            dev->base.flags = (*((netopt_enable_t *)val))
                ? dev->base.flags | AT86RF2XX_OPT_TELL_RX_START
                : dev->base.flags & ~AT86RF2XX_OPT_TELL_RX_START;
            return sizeof(netopt_enable_t);

        case NETOPT_RX_END_IRQ:
            assert(len == sizeof(netopt_enable_t));
            dev->base.flags = (*((netopt_enable_t *)val))
                ? dev->base.flags | AT86RF2XX_OPT_TELL_RX_END
                : dev->base.flags & ~AT86RF2XX_OPT_TELL_RX_END;
            return sizeof(netopt_enable_t);

        case NETOPT_TX_START_IRQ:
            assert(len == sizeof(netopt_enable_t));
            dev->base.flags = (*((netopt_enable_t *)val))
                ? dev->base.flags | AT86RF2XX_OPT_TELL_TX_START
                : dev->base.flags & ~AT86RF2XX_OPT_TELL_TX_START;
            return sizeof(netopt_enable_t);

        case NETOPT_TX_END_IRQ:
            assert(len == sizeof(netopt_enable_t));
            dev->base.flags = (*((netopt_enable_t *)val))
                ? dev->base.flags | AT86RF2XX_OPT_TELL_TX_END
                : dev->base.flags & ~AT86RF2XX_OPT_TELL_TX_END;
            return sizeof(netopt_enable_t);

        case NETOPT_AUTOCCA:
            assert(len == sizeof(netopt_enable_t));
            dev->base.flags  = (*((netopt_enable_t *)val))
                ? dev->base.flags | AT86RF2XX_OPT_AUTOCCA
                : dev->base.flags & ~AT86RF2XX_OPT_AUTOCCA;
            return sizeof(netopt_enable_t);

        case NETOPT_CSMA_RETRIES:
            assert(len == sizeof(uint8_t));
            int8_t csma_retr = ((int8_t)(*((uint8_t *)val)));
            if (csma_retr < 0) { /* overflow */
                csma_retr = AT86RF2XX_MAX_CSMA_RETRIES;
            }
            at86rf2xx_set_csma_max_retries(dev, *((uint8_t *)val));
            return sizeof(int8_t);

        case NETOPT_CSMA_MAXBE:
            assert(len == sizeof(uint8_t));
            at86rf2xx_reg_set(dev,
                              AT86RF2XX_REG__CSMA_BE,
                              AT86RF2XX_CSMA_BE_MASK__MAX_BE,
                              (*((uint8_t *)val)) << 4);
            return sizeof(uint8_t);

        case NETOPT_CSMA_MINBE:
            assert(len == sizeof(uint8_t));
            at86rf2xx_reg_set(dev,
                              AT86RF2XX_REG__CSMA_BE,
                              AT86RF2XX_CSMA_BE_MASK__MIN_BE,
                              *((uint8_t *)val));
            return sizeof(uint8_t);

        default: break;
    }

    return netdev_ieee802154_set(&dev->base.netdev, opt, val, len);
}
