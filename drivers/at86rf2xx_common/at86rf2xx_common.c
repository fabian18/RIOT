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
 * @brief       Implementation of common at86rf2xx functions
 *
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

#include <string.h>

#include "kernel_defines.h"
#include "luid.h"
#include "include/at86rf2xx_common_registers.h"
#include "include/at86rf2xx_common_communication.h"
#include "include/at86rf2xx_common_internal.h"
#include "include/at86rf2xx_common_states.h"
#if IS_USED(MODULE_AT86RF2XX_SPI)
#include "include/at86rf2xx_common_communication_spi.h"
#endif
#if IS_USED(MODULE_AT86RF2XX_PERIPH)
#include "include/at86rf2xx_common_communication_periph.h"
#endif

#define ENABLE_DEBUG    (0)
#include "debug.h"

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

uint8_t at86rf2xx_reg_read(const at86rf2xx_t *dev, uint8_t addr)
{
    /* only SPI for now */
    return at86rf2xx_spi_reg_read(dev, addr);
}

void at86rf2xx_reg_write(const at86rf2xx_t *dev, uint8_t addr, uint8_t value)
{
    /* only SPI for now */
    at86rf2xx_spi_reg_write(dev, addr, value);
}

void at86rf2xx_sram_read(const at86rf2xx_t *dev, uint8_t offset, void *data, size_t len)
{
    /* only SPI for now */
    at86rf2xx_spi_sram_read(dev, offset, data, len);
}

void at86rf2xx_sram_write(const at86rf2xx_t *dev, uint8_t offset, const void *data, size_t len)
{
    /* only SPI for now */
    at86rf2xx_spi_sram_write(dev, offset, data, len);
}

void at86rf2xx_fb_start_read(const at86rf2xx_t *dev)
{
    /* only SPI for now */
    at86rf2xx_spi_fb_start_read(dev);
}

void at86rf2xx_fb_start_write(const at86rf2xx_t *dev)
{
    /* only SPI for now */
    at86rf2xx_spi_fb_start_write(dev);
}

void at86rf2xx_fb_read(const at86rf2xx_t *dev, void *data, size_t len)
{
    /* only SPI for now */
    at86rf2xx_spi_fb_read(dev, data, len);
}

void at86rf2xx_fb_write(const at86rf2xx_t *dev, const void *data, size_t len)
{
    /* only SPI for now */
    at86rf2xx_spi_fb_write(dev, data, len);
}

void at86rf2xx_fb_stop(const at86rf2xx_t *dev)
{
    /* only SPI for now */
    at86rf2xx_spi_fb_stop(dev);
}

uint8_t at86rf2xx_reg_clear(const at86rf2xx_t *dev, uint8_t addr, uint8_t mask)
{
    /* only SPI for now */
    return at86rf2xx_spi_reg_clear(dev, addr, mask);
}

uint8_t at86rf2xx_reg_set(const at86rf2xx_t *dev, uint8_t addr, uint8_t mask, uint8_t value)
{
    /* only SPI for now */
    return at86rf2xx_spi_reg_set(dev, addr, mask, value);
}

static
void _set_state(at86rf2xx_t *dev, uint8_t state, uint8_t cmd)
{
    at86rf2xx_reg_write(dev, AT86RF2XX_REG__TRX_STATE, cmd);

    /* To prevent a possible race condition when changing to
     * RX_AACK_ON state the state doesn't get read back in that
     * case. See discussion
     * in https://github.com/RIOT-OS/RIOT/pull/5244
     */
    if (state != AT86RF2XX_STATE_RX_AACK_ON &&
        state != AT86RF2XX_STATE_RX_ON) {
        while (at86rf2xx_get_state(dev) != state) {}
    }
    /* Although RX_AACK_ON state doesn't get read back,
     * at least make sure if state transition is in progress or not
     */
    else {
        while (at86rf2xx_get_state(dev) == AT86RF2XX_STATE_IN_PROGRESS) {}
    }

    dev->base.state = state;
}

static
int _check_state(const at86rf2xx_t *dev, uint8_t state)
{
    /* Check state (be very paranoid):
       This should only be used inside an assert()
       after a state transission, to check if a state
       transition was successful. So in productive use,
       this should not be linked. */
    uint8_t trx_status;
    do {
        trx_status =
            at86rf2xx_reg_read(dev, AT86RF2XX_REG__TRX_STATUS);
        trx_status &= AT86RF2XX_TRX_STATUS_MASK__TRX_STATUS;
    } while (trx_status == AT86RF2XX_STATE_IN_PROGRESS);
    DEBUG("input state: 0x%02X -- device state: 0x%02X -- trx_staus: 0x%02X\n",
          state, dev->base.state, trx_status);

    if (dev->base.state == AT86RF2XX_STATE_RX_ON) {
        return ((trx_status == AT86RF2XX_STATE_RX_ON) ||
               (trx_status == AT86RF2XX_STATE_BUSY_RX));
    }
    else if (dev->base.state == AT86RF2XX_STATE_RX_AACK_ON) {
        return ((trx_status == AT86RF2XX_STATE_RX_AACK_ON) ||
               (trx_status == AT86RF2XX_STATE_BUSY_RX_AACK));
    }
    else if (dev->base.state == AT86RF2XX_STATE_SLEEP) {
        return (state == AT86RF2XX_STATE_SLEEP);
    }
    else {
        return (trx_status == dev->base.state);
    }
}

void at86rf2xx_power_on(at86rf2xx_t * dev)
{
    if (dev->base.state == AT86RF2XX_STATE_P_ON) {
        at86rf2xx_reg_write(dev, AT86RF2XX_REG__TRX_STATE,
                                 AT86RF2XX_STATE_FORCE_TRX_OFF);
        /* If no transceiver is connected, this assertion will fire.
           If you use at86rf2xx_set_state(), the driver will hang in
           a loop because TRX_OFF will never be read back. */
        assert(at86rf2xx_get_state(dev) == AT86RF2XX_STATE_TRX_OFF);
        dev->base.state = AT86RF2XX_STATE_TRX_OFF;
    }
}

uint8_t at86rf2xx_get_state(const at86rf2xx_t *dev)
{
    /* if sleeping immediately return state */
    if (dev->base.state == AT86RF2XX_STATE_SLEEP) {
        return dev->base.state;
    }

    return (at86rf2xx_reg_read(dev, AT86RF2XX_REG__TRX_STATUS)
            & AT86RF2XX_TRX_STATUS_MASK__TRX_STATUS);
}

uint8_t at86rf2xx_set_state(at86rf2xx_t *dev, uint8_t cmd)
{
    /* cmd:                         state:
     * TX_START:            0x02    BUSY_TX or BUSY_TX_ARET     0x02 or 0x12
     * FORCE_TRX_OFF:       0x03    TRX_OFF                     0x08
     * FORCE_PLL_ON:        0x04    PLL_ON                      0x09
     * RX_ON:               0x06    RX_ON                       0x06
     * TRX_OFF:             0x08    TRX_OFF                     0x08
     * PLL_ON:              0x09    PLL_ON                      0x09
     * PREP_DEEP_SLEEP:     0x10    PREP_DEEP_SLEEP             0x10
     * RX_AACK_ON:          0x16    RX_AACK_ON                  0x16
     * TX_ARET_ON:          0x19    TX_ARET_ON                  0x19 */
    assert(cmd == AT86RF2XX_TRX_CMD__FORCE_TRX_OFF      ||
           cmd == AT86RF2XX_TRX_CMD__FORCE_PLL_ON       ||
           cmd == AT86RF2XX_TRX_CMD__RX_ON              ||
           cmd == AT86RF2XX_TRX_CMD__TRX_OFF            ||
           cmd == AT86RF2XX_TRX_CMD__PLL_ON             ||
           cmd == AT86RF2XX_TRX_CMD__PREP_DEEP_SLEEP    ||
           cmd == AT86RF2XX_TRX_CMD__RX_AACK_ON         ||
           cmd == AT86RF2XX_TRX_CMD__TX_ARET_ON);

    uint8_t old_state;
    uint8_t state = cmd;
    if ((old_state = at86rf2xx_get_state(dev)) != AT86RF2XX_STATE_SLEEP) {
        while (old_state == AT86RF2XX_STATE_IN_PROGRESS) {
            old_state = at86rf2xx_get_state(dev);
        }
        if (cmd == AT86RF2XX_STATE_FORCE_TRX_OFF) {
            _set_state(dev, AT86RF2XX_STATE_TRX_OFF, AT86RF2XX_TRX_CMD__FORCE_TRX_OFF);
        }
        else if (cmd == AT86RF2XX_TRX_CMD__FORCE_PLL_ON) {
            _set_state(dev, AT86RF2XX_STATE_PLL_ON, AT86RF2XX_TRX_CMD__FORCE_PLL_ON);
        }
        else {
            /* make sure there is no ongoing transmission, or state transmission already
               in progress */
            while (at86rf2xx_is_busy_state(old_state)) {
                old_state = at86rf2xx_get_state(dev);
            }
            if (state != old_state) {
                /* we need to go via PLL_ON if we are moving between RX_AACK_ON <-> TX_ARET_ON */
                if (old_state == AT86RF2XX_STATE_RX_AACK_ON ||
                    old_state == AT86RF2XX_STATE_TX_ARET_ON) {
                    _set_state(dev, AT86RF2XX_STATE_PLL_ON, AT86RF2XX_TRX_CMD__PLL_ON);
                }
                if (state == AT86RF2XX_TRX_CMD__PREP_DEEP_SLEEP) {
                    _set_state(dev, AT86RF2XX_STATE_TRX_OFF, AT86RF2XX_TRX_CMD__TRX_OFF);
                }
                _set_state(dev, state, cmd);
            }
            assert(_check_state(dev, state));
        }
    }

    return old_state;
}


void at86rf2xx_get_addr_short(const at86rf2xx_t *dev, network_uint16_t *addr)
{
    memcpy(addr, dev->base.netdev.short_addr, sizeof(*addr));
}

void at86rf2xx_set_addr_short(at86rf2xx_t *dev, const network_uint16_t *addr)
{
    memcpy(dev->base.netdev.short_addr, addr, sizeof(*addr));
#if IS_USED(MODULE_SIXLOWPAN)
    /* https://tools.ietf.org/html/rfc4944#section-12 requires the first bit to
     * 0 for unicast addresses */
    dev->base.netdev.short_addr[0] &= 0x7F;
#endif
    /* device use lsb first, not network byte order */
    at86rf2xx_reg_write(dev, AT86RF2XX_REG__SHORT_ADDR_0,
                        dev->base.netdev.short_addr[1]);
    at86rf2xx_reg_write(dev, AT86RF2XX_REG__SHORT_ADDR_1,
                        dev->base.netdev.short_addr[0]);
}

void at86rf2xx_get_addr_long(const at86rf2xx_t *dev, eui64_t *addr)
{
    memcpy(addr, dev->base.netdev.long_addr, sizeof(*addr));
}

void at86rf2xx_set_addr_long(at86rf2xx_t *dev, const eui64_t *addr)
{
    memcpy(dev->base.netdev.long_addr, addr, sizeof(*addr));
    for (int i = 0; i < 8; i++) {
        /* device use lsb first, not network byte order */
        at86rf2xx_reg_write(dev, (AT86RF2XX_REG__IEEE_ADDR_0 + i),
            dev->base.netdev.long_addr[IEEE802154_LONG_ADDRESS_LEN - 1 - i]);
    }
}

void at86rf2xx_address_init_auto(at86rf2xx_t *dev)
{
    eui64_t addr_long;
    network_uint16_t addr_short;

    /* generate EUI-64 and short address */
    luid_get_eui64(&addr_long);
    luid_get_short(&addr_short);

    /* set short and long address */
    at86rf2xx_set_addr_long((at86rf2xx_t *)dev, &addr_long);
    at86rf2xx_set_addr_short((at86rf2xx_t *)dev, &addr_short);
}

uint8_t at86rf2xx_get_channel(const at86rf2xx_t *dev)
{
    return dev->base.netdev.chan;
}

void at86rf2xx_set_channel(at86rf2xx_t *dev, uint8_t channel)
{
    if (channel < IEEE802154_CHANNEL_MIN) {
        channel = IEEE802154_CHANNEL_MIN;
    }
    else if (channel > IEEE802154_CHANNEL_MAX) {
        channel = IEEE802154_CHANNEL_MAX;
    }
    at86rf2xx_reg_set(dev,
                      AT86RF2XX_REG__PHY_CC_CCA,
                      AT86RF2XX_PHY_CC_CCA_MASK__CHANNEL,
                      channel);
    dev->base.netdev.chan = AT86RF2XX_PHY_CC_CCA_MASK__CHANNEL & channel;
}

uint16_t at86rf2xx_get_pan(const at86rf2xx_t *dev)
{
    return dev->base.netdev.pan;
}

void at86rf2xx_set_pan(at86rf2xx_t *dev, uint16_t pan)
{
    uint8_t lower = pan & 0x00FF;
    uint8_t higher = (pan & 0xFF00) >> 8;
    DEBUG("pan1: %u, pan0: %u\n", lower, higher);
    at86rf2xx_reg_write(dev, AT86RF2XX_REG__PAN_ID_0, lower);
    at86rf2xx_reg_write(dev, AT86RF2XX_REG__PAN_ID_1, higher);

    dev->base.netdev.pan = pan;
}

uint8_t at86rf2xx_get_csma_max_retries(const at86rf2xx_t *dev)
{
    uint8_t xah_ctrl_0 = at86rf2xx_reg_read(dev, AT86RF2XX_REG__XAH_CTRL_0);
    return (xah_ctrl_0 & AT86RF2XX_XAH_CTRL_0_MASK__MAX_CSMA_RETRIES) >> 1;
}

void at86rf2xx_set_csma_max_retries(const at86rf2xx_t *dev, int8_t retries)
{
    if (retries > AT86RF2XX_MAX_CSMA_RETRIES) {
        /* valid values: 0-5 */
        retries = AT86RF2XX_MAX_CSMA_RETRIES;
    }
    else if (retries < 0) {
        /* max < 0 => disable CSMA (set to 7) */
        retries = AT86RF2XX_MAX_CSMA_RETRIES__NO_CSMA >> 1;
    }
    at86rf2xx_reg_set(dev,
                      AT86RF2XX_REG__XAH_CTRL_0,
                      AT86RF2XX_XAH_CTRL_0_MASK__MAX_CSMA_RETRIES,
                      retries << 1);
}

void at86rf2xx_set_frame_max_retries(at86rf2xx_t *dev, uint8_t retries)
{
    retries = MIN(retries, AT86RF2XX_MAX_FRAME_RETRIES);
    at86rf2xx_reg_set(dev,
                      AT86RF2XX_REG__XAH_CTRL_0,
                      AT86RF2XX_XAH_CTRL_0_MASK__MAX_FRAME_RETRIES,
                      retries << 5);
}

void at86rf2xx_get_csma_backoff_exp(const at86rf2xx_t *dev,
                                    uint8_t *min, uint8_t *max)
{
    *min = at86rf2xx_reg_read(dev, AT86RF2XX_REG__CSMA_BE) &
           AT86RF2XX_CSMA_BE_MASK__MIN_BE;
    *min = MIN(*min, AT86RF2XX_MAX_BE);

    *max = (at86rf2xx_reg_read(dev, AT86RF2XX_REG__CSMA_BE) &
           AT86RF2XX_CSMA_BE_MASK__MAX_BE) >> 4;
    *max = MIN(*max, AT86RF2XX_MAX_BE);
}

void at86rf2xx_set_csma_backoff_exp(const at86rf2xx_t *dev,
                                    uint8_t min, uint8_t max)
{
    min = MIN(min, AT86RF2XX_MAX_BE);
    max = MIN(max, AT86RF2XX_MAX_BE);
    if (min > max) {
        uint8_t t = min;
        min = max;
        max = t;
    }
    at86rf2xx_reg_write(dev, AT86RF2XX_REG__CSMA_BE, (max << 4) | min);
}

void at86rf2xx_set_csma_seed(const at86rf2xx_t *dev, const uint8_t entropy[2])
{
    at86rf2xx_reg_write(dev, AT86RF2XX_REG__CSMA_SEED_0, entropy[0]);
    at86rf2xx_reg_set(dev,
                      AT86RF2XX_REG__CSMA_SEED_1,
                      AT86RF2XX_CSMA_SEED_1_MASK__CSMA_SEED_1,
                      entropy[1]);
}

void at86rf2xx_get_random(const at86rf2xx_t *dev, uint8_t *data, size_t len)
{
    for (size_t byteCount = 0; byteCount < len; ++byteCount) {
        uint8_t rnd = 0;
        for (uint8_t i = 0; i < 4; ++i) {
            /* bit 5 and 6 of the AT86RF2XX_REG__PHY_RSSI register contain the RND_VALUE */
            uint8_t phy_rssi = at86rf2xx_reg_read(dev, AT86RF2XX_REG__PHY_RSSI)
                               & AT86RF2XX_PHY_RSSI_MASK__RND_VALUE;
            /* shift the two random bits first to the right and then to the correct position of the return byte */
            phy_rssi = phy_rssi >> 5;
            phy_rssi = phy_rssi << 2 * i;
            rnd |= phy_rssi;
        }
        data[byteCount] = rnd;
    }
}

bool at86rf2xx_cca(at86rf2xx_t *dev)
{
    uint8_t old_state = at86rf2xx_set_state(dev, AT86RF2XX_TRX_CMD__TRX_OFF);
    /* Disable RX path */
    uint8_t rx_syn = at86rf2xx_reg_read(dev, AT86RF2XX_REG__RX_SYN);
    uint8_t tmp = rx_syn | AT86RF2XX_RX_SYN_MASK__RX_PDT_DIS;
    at86rf2xx_reg_write(dev, AT86RF2XX_REG__RX_SYN, tmp);

    /* Manually triggered CCA is only possible in RX_ON (basic operating mode) */
    at86rf2xx_set_state(dev, AT86RF2XX_TRX_CMD__RX_ON);

    /* trigger CCA */
    at86rf2xx_reg_set(dev,
                      AT86RF2XX_REG__PHY_CC_CCA,
                      AT86RF2XX_PHY_CC_CCA_MASK__CCA_REQUEST,
                      AT86RF2XX_PHY_CC_CCA_MASK__CCA_REQUEST);

    /* Spin until done (8 symbols + 12 µs = 128 µs + 12 µs for O-QPSK)*/
    uint8_t trx_status;
    do {
        trx_status = at86rf2xx_reg_read(dev, AT86RF2XX_REG__TRX_STATUS);
    } while (!(trx_status & AT86RF2XX_TRX_STATUS_MASK__CCA_DONE));
    bool cca = !!(trx_status & AT86RF2XX_CCA_STATUS__CLEAR);

    /* re-enable RX */
    at86rf2xx_reg_write(dev, AT86RF2XX_REG__RX_SYN, rx_syn);

    /* Step back to the old state */
    at86rf2xx_set_state(dev, AT86RF2XX_TRX_CMD__TRX_OFF);
    at86rf2xx_set_state(dev, old_state);

    return cca;
}
