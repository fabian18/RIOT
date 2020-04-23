
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
 * @ingroup     drivers_at86rf233
 * @{
 *
 * @file
 * @brief       Implementation of at86rf233 specific functions
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

#include "xtimer.h"
#include "../at86rf2xx_common/include/at86rf2xx_common_registers.h"
#include "../at86rf2xx_common/include/at86rf2xx_common_communication_spi.h"
#include "../at86rf2xx_common/include/at86rf2xx_common_states.h"
#include "../at86rf2xx_common/include/at86rf2xx_common_netdev.h"
#include "include/at86rf233_defines.h"
#include "include/at86rf233_hardware.h"
#include "include/at86rf233_internal.h"
#include "include/at86rf233_types.h"
#include "include/at86rf233_netdev.h"

#define AT86RF233_TX_PWR_VALUES                     \
{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,   \
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }

#define AT86RF233_TX_PWR_DBM                        \
{ 4, 4 /* 3.7 */, 3 /* 3.4 */, 3, 3 /* 2.5 */, 2,   \
  1, 0, -1, -2, -3, -4, -6, -8, -12, -17 }

#define AT86RF233_RX_SENSE_VALUES                   \
{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,   \
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }

#define AT86RF233_RX_SENSE_DBM                      \
{ -101, -94, -91, -88, -85, -82, -79,  -76, -73,    \
  -70, -67, -64, -61,  -58, -55, -52 }

/* Is there a netopt to configure modulation/datarate? */
#define AT86RF233_PHY_DATA_RATES_KBPS { 250, 500, 1000, 2000 }

static inline
void at86rf233_init_default(at86rf233_t *dev)
{
    {
        uint8_t addr[] = AT86RF233_LONG_ADDR_DEFAULT;
        memmove(dev->base.netdev.long_addr,
                addr,
                IEEE802154_LONG_ADDRESS_LEN);
    }
    {
        const uint8_t addr[] = AT86RF233_SHORT_ADDR_DEFAULT;
        memmove(dev->base.netdev.short_addr,
                addr,
                IEEE802154_SHORT_ADDRESS_LEN);
    }
    {
        const uint8_t pan[] = AT86RF233_PAN_ID_DEFAULT;
        memmove(&dev->base.netdev.pan, pan, 2);
    }
    dev->base.netdev.chan = AT86RF233_CHANNEL_DEFAULT;
    dev->base.netdev.rxsens = AT86RF233_RX_SENSITIVITY_DEFAULT_DBM;
    dev->base.netdev.txpower = AT86RF233_TX_POWER_DEFAULT_DBM;
}

void at86rf233_setup(at86rf233_t *dev, const at86rf233_params_t *params)
{
    assert(dev);
    assert(params);
    memset(dev, 0, sizeof(*dev));
    dev->base.netdev.netdev.driver = &at86rf233_driver;
    dev->base.idle_state = AT86RF2XX_STATE_RX_AACK_ON;
    dev->base.state = AT86RF2XX_STATE_P_ON;
    dev->params = *params;
}

void at86rf233_sleep(at86rf233_t *dev)
{
    if (dev->base.state != AT86RF2XX_STATE_SLEEP) {
        /* First go to TRX_OFF */
        if (dev->base.state != AT86RF2XX_STATE_PREP_DEEP_SLEEP &&
            dev->base.state != AT86RF2XX_STATE_TRX_OFF) {
            at86rf2xx_set_state((at86rf2xx_t *)dev, AT86RF2XX_TRX_CMD__TRX_OFF);
        }
        /* Discard all IRQ flags, framebuffer is lost anyway */
        at86rf2xx_spi_reg_read((at86rf2xx_t *)dev, AT86RF2XX_REG__IRQ_STATUS);
        dev->base.flags &= ~AT86RF2XX_FLG_PENDING_IRQ;
        gpio_set(dev->params.base_params.sleep_pin);
        dev->base.state = AT86RF2XX_STATE_SLEEP;
    }
}

void at86rf233_deep_sleep(at86rf233_t *dev)
{
    if (!(dev->base.flags & AT86RF2XX_FLG_DEEP_SLEEP)) {
        if (dev->base.state == AT86RF2XX_STATE_SLEEP) {
            /* need to wake up to go from sleep to deep sleep */
            at86rf233_assert_awake(dev);
        }
        at86rf2xx_set_state((at86rf2xx_t *)dev, AT86RF2XX_TRX_CMD__PREP_DEEP_SLEEP);
        at86rf233_sleep(dev);
        dev->base.flags |= AT86RF2XX_FLG_DEEP_SLEEP;
    }
    assert(dev->base.state == AT86RF2XX_STATE_SLEEP);
}

void at86rf233_assert_awake(at86rf233_t *dev)
{
    if (dev->base.state == AT86RF2XX_STATE_SLEEP) {
        gpio_clear(dev->params.base_params.sleep_pin);
        xtimer_usleep(AT86RF233_WAKEUP_DELAY_US);

        /* update state: on some platforms, the timer behind xtimer
         * may be inaccurate or the radio itself may take longer
         * to wake up due to extra capacitance on the oscillator.
         * Spin until we are actually awake
         */
        do {
            dev->base.state =
                at86rf2xx_spi_reg_read((at86rf2xx_t *)dev, AT86RF2XX_REG__TRX_STATUS) &
                AT86RF2XX_TRX_STATUS_MASK__TRX_STATUS;
        } while (dev->base.state != AT86RF2XX_TRX_STATUS__TRX_OFF);

        if (dev->base.flags & AT86RF2XX_FLG_DEEP_SLEEP) {
            /* register content has been lost during deep sleep,
               so re-init() */
            dev->base.flags &= ~AT86RF2XX_FLG_DEEP_SLEEP;
            dev->base.netdev.netdev.driver->init(&dev->base.netdev.netdev);
        }
    }
}

void at86rf233_hardware_reset(at86rf233_t *dev)
{
    gpio_clear(dev->params.base_params.reset_pin);
    xtimer_usleep(AT86RF233_RESET_PULSE_WIDTH_US);
    gpio_set(dev->params.base_params.reset_pin);
    xtimer_usleep(AT86RF233_RESET_DELAY_US);
    assert(at86rf2xx_get_state((at86rf2xx_t *)dev) == AT86RF2XX_STATE_TRX_OFF);
    dev->base.state = AT86RF2XX_STATE_TRX_OFF;
    at86rf233_init_default(dev);
}

void at86rf233_trigger_send_gpio(at86rf233_t *dev)
{
    gpio_set(dev->params.base_params.sleep_pin);
    xtimer_usleep(AT86RF233_SLP_TR_PULSE_WIDTH_US);
    gpio_clear(dev->params.base_params.sleep_pin);
}

void at86rf233_set_tx_power(at86rf233_t *dev, int8_t tx_power)
{
    uint8_t p;
    {
        uint8_t dbm[] = AT86RF233_TX_PWR_DBM;
        for (p = 0; p < ARRAY_SIZE(dbm); p++) {
            if (dbm[p] <= tx_power) {
                break;
            }
        }
        dev->base.netdev.txpower = dbm[p];
    }
    at86rf2xx_spi_reg_write((at86rf2xx_t *)dev,
                            AT86RF2XX_REG__PHY_TX_PWR,
                            ((uint8_t[])AT86RF233_TX_PWR_VALUES)[p]);
}

int8_t at86rf233_get_tx_power(const at86rf233_t *dev)
{
    return dev->base.netdev.txpower;
}

void at86rf233_set_rx_sensibility(at86rf233_t *dev, int8_t rx_sense)
{
    uint8_t s;
    {
        uint8_t dbm[] = AT86RF233_RX_SENSE_DBM;
        for (s = 0; s < ARRAY_SIZE(dbm); s++) {
            if (dbm[s] >= rx_sense) {
                break;
            }
        }
        dev->base.netdev.rxsens = dbm[s];
    }
    at86rf2xx_spi_reg_set((at86rf2xx_t *)dev,
                          AT86RF2XX_REG__RX_SYN,
                          AT86RF2XX_RX_SYN_MASK__RX_PDT_LEVEL,
                          ((uint8_t[])AT86RF233_RX_SENSE_VALUES)[s]);

}

int8_t at86rf233_get_rx_sensesibility(const at86rf233_t *dev)
{
    return dev->base.netdev.rxsens;
}

int8_t at86rf233_get_cca_threshold(const at86rf233_t *dev)
{
    uint8_t thresh = at86rf2xx_spi_reg_read((const at86rf2xx_t *)dev,
                                         AT86RF2XX_REG__CCA_THRES);

    /* multiply by 2 because precesion is 2 dbm */
    return AT86RF233_RSSI_BASE_VAL + (thresh << 1);
}

void at86rf233_set_cca_threshold(const at86rf233_t *dev, int8_t thresh)
{
    /* ensure the given value is negative, since a CCA threshold > 0 is
       just impossible: thus, any positive value given is considered
       to be the absolute value of the actually wanted threshold */
    if (thresh > 0) {
        thresh = -thresh;
    }
    if (thresh < AT86RF233_RSSI_BASE_VAL) {
        thresh = AT86RF233_RSSI_BASE_VAL;
    }
    /* transform the dBm value in the form
       that will fit in the AT86RF2XX_REG__CCA_THRES register */
    thresh -= AT86RF233_RSSI_BASE_VAL;
    thresh >>= 1; /* divide by 2 because precesion is 2 dbm */
    thresh &= AT86RF2XX_CCA_THRES_MASK__CCA_ED_THRES;

    at86rf2xx_spi_reg_write((const at86rf2xx_t *)dev,
                            AT86RF2XX_REG__CCA_THRES,
                            (uint8_t)thresh);
}

void at86rf233_configure_phy(at86rf233_t *dev, at86rf233_phy_mode_t mode)
{
    at86rf2xx_spi_reg_set((at86rf2xx_t *)dev,
                          AT86RF2XX_REG__TRX_CTRL_2,
                          AT86RF2XX_TRX_CTRL_2_MASK__OQPSK_SCRAM_EN |
                          AT86RF233_TRX_CTRL_2_MASK__OQPSK_DATA_RATE,
                          mode);
}

uint8_t at86rf233_fb_read_phr(const at86rf233_t *dev)
{
    uint8_t phr;
    at86rf2xx_spi_sram_read((at86rf2xx_t *)dev, 0, &phr, 1);
    return phr;
}

ssize_t at86rf233_fb_read(const at86rf233_t *dev, at86rf233_fb_t *fb,
                          void *buf, size_t buf_size)
{
    at86rf2xx_spi_fb_start_read((at86rf2xx_t *)dev);
    at86rf2xx_spi_fb_read((at86rf2xx_t *)dev, &fb->phr, 1);
    int payload_length = (fb->phr & 0x7F) - IEEE802154_FCS_LEN;
    if (payload_length > 0) {
        if ((size_t)payload_length > buf_size) {
            return -ENOBUFS;
        }
        at86rf2xx_spi_fb_read((at86rf2xx_t *)dev, buf, payload_length);
        fb->payload = buf;
        at86rf2xx_spi_fb_read((at86rf2xx_t *)dev, &fb->fcs, 2);
        at86rf2xx_spi_fb_read((at86rf2xx_t *)dev, &fb->lqi, 1);
        at86rf2xx_spi_fb_read((at86rf2xx_t *)dev, &fb->ed, 1);
        at86rf2xx_spi_fb_read((at86rf2xx_t *)dev, &fb->rx_status, 1);
    }
    at86rf2xx_spi_fb_stop((at86rf2xx_t *)dev);
    return payload_length;
}