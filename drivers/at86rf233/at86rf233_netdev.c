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
 * @brief       Implementation of RIOT's netdev_driver API for the at86rf233
 *              transceiver
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
#include <assert.h>
#include <errno.h>

#include "xtimer.h"
#include "iolist.h"
#include "net/eui64.h"
#include "net/ieee802154.h"
#include "net/netdev.h"
#include "net/netdev/ieee802154.h"
#include "../at86rf2xx_common/include/at86rf2xx_common_registers.h"
#include "../at86rf2xx_common/include/at86rf2xx_common_communication_spi.h"
#include "../at86rf2xx_common/include/at86rf2xx_common_internal.h"
#include "../at86rf2xx_common/include/at86rf2xx_common_states.h"
#include "../at86rf2xx_common/include/at86rf2xx_common_netdev.h"
#include "../at86rf2xx_common/include/at86rf2xx_common_aes.h"
#include "include/at86rf233_defines.h"
#include "include/at86rf233_hardware.h"
#include "include/at86rf233_internal.h"
#include "include/at86rf233_netdev.h"
#include "include/at86rf233_types.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define AT86RF233_DEBUG(...)                                    \
    DEBUG("[at863f233_netdev]: "__VA_ARGS__)
#define AT86RF2XX_DEBUG_PUTS(...)                               \
    DEBUG_PUTS("[at863f233_netdev]: "__VA_ARGS__);

#define AT86RF233_DEFAULT_FLAGS                                 \
    (AT86RF2XX_OPT_AUTOACK      |                               \
    AT86RF2XX_OPT_AUTOCCA       |                               \
    AT86RF2XX_OPT_TELL_RX_END)

static int _init(netdev_t *netdev);
static int _send(netdev_t *netdev, const iolist_t *iolist);
static int _recv(netdev_t *netdev, void *buf, size_t len, void *info);
static void _isr(netdev_t *netdev);
static int _get(netdev_t *netdev, netopt_t opt, void *val, size_t max_len);
static int _set(netdev_t *netdev, netopt_t opt, const void *val, size_t len);

const netdev_driver_t at86rf233_driver = {
    .send = _send,
    .recv = _recv,
    .init = _init,
    .isr = _isr,
    .get = _get,
    .set = _set,
};

static
int _set_state_netopt(at86rf233_t *dev, netopt_state_t state)
{
    switch (state) {
        case NETOPT_STATE_STANDBY:
            at86rf2xx_set_state((at86rf2xx_t *)dev, AT86RF2XX_TRX_CMD__TRX_OFF);
            break;
        case NETOPT_STATE_SLEEP:
            at86rf233_sleep(dev);
            break;
        case NETOPT_STATE_IDLE:
            if (dev->base.idle_state == AT86RF2XX_STATE_SLEEP) {
                at86rf233_sleep(dev);
            }
            else {
                at86rf2xx_set_state((at86rf2xx_t *)dev, dev->base.idle_state);
            }
            break;
        case NETOPT_STATE_RX:
            at86rf2xx_set_state((at86rf2xx_t *)dev, AT86RF2XX_PHY_STATE_RX);
            break;
        case NETOPT_STATE_TX:
            /* The netdev driver ISR switches the transceiver back to the
            * previous idle state after a completed TX. If the user tries
            * to initiate another transmission (retransmitting the same data)
            * without first going to TX_ARET_ON, the command to start TX
            * would be ignored, leading to a deadlock in this netdev driver
            * thread.
            * Additionally, avoids driver thread deadlock when PRELOADING
            * is set and the user tries to initiate TX without first calling
            * send() to write some frame data.
            */
            at86rf2xx_set_state((at86rf2xx_t *)dev, AT86RF2XX_STATE_PLL_ON);
            if (dev->base.pending_tx) {
                /* retransmission of old data, at86rf2xx_tx_prepare normally
                 * increments this and the ISR for TX_END decrements it, to
                 * know when to switch back to the idle state. */
                while(_send((netdev_t *)dev, NULL) == -EINTR){};
            }
            break;
        case NETOPT_STATE_RESET:
            _init((netdev_t *)dev);
            break;
        default:
            return -ENOTSUP;
    }
    return sizeof(netopt_state_t);
}

static
void _irq_handler(void *arg)
{
    at86rf233_t *dev = (at86rf233_t *)arg;
    dev->base.flags |= AT86RF2XX_FLG_PENDING_IRQ;
    /* do not congest IRQ queue with further IRQ messages,
       after the IRQ pin has been trigegred once.
       _isr() reanables interrupts */
    gpio_irq_disable(dev->params.base_params.int_pin);
    netdev_trigger_event_isr(arg);
}

static
int _init(netdev_t *netdev)
{
    at86rf233_t *dev = (at86rf233_t *)netdev;

    if (dev->base.state == AT86RF2XX_STATE_P_ON) {
        AT86RF233_DEBUG("init() device at %p\n", dev);
        gpio_init(dev->params.base_params.sleep_pin, GPIO_OUT);
        gpio_clear(dev->params.base_params.sleep_pin);
        gpio_init(dev->params.base_params.reset_pin, GPIO_OUT);
        gpio_set(dev->params.base_params.reset_pin);
        spi_init_cs(dev->params.base_params.spi, dev->params.base_params.cs_pin);
        if (gpio_init_int(dev->params.base_params.int_pin,
                          GPIO_IN, GPIO_RISING, _irq_handler, dev) != 0) {
            AT86RF2XX_DEBUG_PUTS("init() gpio error\n");
            return -EIO;
        }
        /* Intentionally check if bus can be acquired,
            since getbus() drops the return value */
        if (spi_acquire(dev->params.base_params.spi,
                        dev->params.base_params.cs_pin,
                        SPI_MODE_0,
                        dev->params.base_params.spi_clk) < 0) {
            AT86RF2XX_DEBUG_PUTS("init() unable to acquire SPI bus\n");
            return -EIO;
        }
        spi_release(dev->params.base_params.spi);
        at86rf2xx_power_on((at86rf2xx_t *)dev);
    }
    at86rf233_assert_awake(dev);
    at86rf233_hardware_reset(dev);
    if (at86rf2xx_spi_reg_read((at86rf2xx_t *)dev, AT86RF2XX_REG__PART_NUM)
        != AT86RF233_PART_NUM) {
        AT86RF2XX_DEBUG_PUTS("init() device is not an at86rf233\n");
        return -ENODEV;
    }

    dev->base.flags = AT86RF233_DEFAULT_FLAGS;
    dev->base.idle_state = AT86RF2XX_PHY_STATE_RX;
    dev->base.pending_tx = 0;
    dev->base.tx_frame_len = 0;

    netdev_ieee802154_reset(&dev->base.netdev);
    static const netopt_enable_t en = NETOPT_ENABLE;
    netdev_ieee802154_set(&dev->base.netdev, NETOPT_ACK_REQ, &en, sizeof(en));

    at86rf2xx_address_init_auto((at86rf2xx_t *)dev);

    uint8_t csma_seed[2];
    at86rf2xx_set_csma_seed((at86rf2xx_t *)dev, csma_seed);

    at86rf2xx_spi_reg_set((at86rf2xx_t *)dev,
        AT86RF2XX_REG__TRX_CTRL_2,
        AT86RF2XX_TRX_CTRL_2_MASK__RX_SAFE_MODE,
        AT86RF2XX_RX_SAVE_MODE__EN);

    /* smart receive */
    at86rf2xx_spi_reg_write((at86rf2xx_t *)dev, AT86RF2XX_REG__TRX_RPC, 0xFF);

    /* Enable all interrupts -
     * By default all IRQ are also visible in  TRX_STATUS as well */
    at86rf2xx_spi_reg_write((at86rf2xx_t *)dev,
        AT86RF2XX_REG__IRQ_MASK,
        AT86RF2XX_REG__IRQ_MASK);

#if IS_USED(MODULE_IEEE802154_SECURITY)
    _set(&dev->base.netdev.netdev, NETOPT_ENCRYPTION, &en, sizeof(en));
    _set(&dev->base.netdev.netdev, NETOPT_ENCRYPTION_KEY,
         dev->base.netdev.sec_ctx.cipher.context.context, IEEE802154_SEC_KEY_LENGTH);
#if IS_USED(MODULE_AT86RF2XX_COMMON_AES_SPI)
    dev->base.netdev.sec_ctx.cipher_ops = &_at86rf2xx_cipher_ops;
#endif
#endif

    /* idle */
    _set_state_netopt(dev, NETOPT_STATE_IDLE);

    return 0;
}

static
int _send(netdev_t *netdev, const iolist_t *iolist)
{
    at86rf233_t *dev = (at86rf233_t *)netdev;
    uint8_t state;

    /* block while busy */
    do {
        state = at86rf2xx_get_state((at86rf2xx_t *)dev);
    }
    while (at86rf2xx_is_busy_state(state));

    at86rf233_assert_awake(dev);
    if (dev->base.flags & AT86RF2XX_FLG_PENDING_IRQ) {
        AT86RF2XX_DEBUG_PUTS("send() handle pending interrupt\n");
        _isr(netdev);
        return -EINTR; /* just recall _send() */
    }
    at86rf2xx_set_state((at86rf2xx_t *)dev, AT86RF2XX_TRX_CMD__PLL_ON);  /* lock */
    if (dev->base.pending_tx) {
        /* A frame has been preloaded.
           Before a new frame can be send, the preloaded frame must be
           fired first. */
       AT86RF2XX_DEBUG_PUTS("send() send pending frame\n");
       at86rf2xx_set_state((at86rf2xx_t *)dev, AT86RF2XX_PHY_STATE_TX);
       at86rf233_trigger_send_gpio(dev);
       return -EAGAIN;  /* just recall _send() */
    }
    /* Check what state is going to becomes the idle state
       (the state we return to, after sending). */
    if (state == AT86RF2XX_STATE_RX_AACK_ON ||
        state == AT86RF2XX_STATE_RX_ON      ||
        state == AT86RF2XX_STATE_TRX_OFF    ||
        state == AT86RF2XX_STATE_SLEEP) {
        dev->base.idle_state = state;
    }

    /* If I write PHR after writing the data, I get TRX_UR interrupt */
    uint8_t len = IEEE802154_FCS_LEN;
    for (const iolist_t *iol = iolist; iol; iol = iol->iol_next) {
        if ((len += iol->iol_len) > IEEE802154_FRAME_LEN_MAX) {
            AT86RF2XX_DEBUG_PUTS("send() frame too big\n");
            _set_state_netopt(dev,NETOPT_STATE_IDLE);
            return -EOVERFLOW;
        }
    }
    at86rf2xx_spi_sram_write((at86rf2xx_t *)dev, 0, &len, 1);
    len = 0;
    for (const iolist_t *iol = iolist; iol; iol = iol->iol_next) {
        if (iol->iol_len) {
            at86rf2xx_spi_sram_write((at86rf2xx_t *)dev,
                                     1 + len,
                                     iol->iol_base,
                                     iol->iol_len);
            len +=  iol->iol_len;
        }
    }
    dev->base.tx_frame_len = len + IEEE802154_FCS_LEN;
    dev->base.pending_tx++;
    /* send data out directly if pre-loading id disabled */
    if (!(dev->base.flags & AT86RF2XX_OPT_PRELOADING)) {
        at86rf2xx_set_state((at86rf2xx_t *)dev, AT86RF2XX_PHY_STATE_TX);
        at86rf233_trigger_send_gpio(dev);
        if (dev->base.flags & AT86RF2XX_OPT_TELL_TX_START) {
            netdev->event_callback(netdev, NETDEV_EVENT_TX_STARTED);
        }
    }
    return dev->base.tx_frame_len;
}


static
int _recv(netdev_t *netdev, void *buf, size_t len, void *info)
{
    at86rf233_t *dev = (at86rf233_t *)netdev;

    /* frame buffer protection will be unlocked as soon as
       at86rf2xx_fb_stop() is called.
       Set receiver to PLL_ON state to be able to free the
       SPI bus and avoid losing data. */
    at86rf2xx_set_state((at86rf2xx_t *)dev, AT86RF2XX_TRX_CMD__PLL_ON);

    uint8_t phr = at86rf233_fb_read_phr(dev);
    size_t frame_len = (phr & 0x7F) - IEEE802154_FCS_LEN;

    /* return length when buf == NULL */
    if (!buf) {
        /* drop packet, continue receiving */
        if (len) {
            _set_state_netopt(dev, NETOPT_STATE_IDLE);
        }
        return frame_len;
    }
    /* not enough space in buf */
    if (frame_len > len) {
        _set_state_netopt(dev, NETOPT_STATE_IDLE);
        return -ENOBUFS;
    }

    at86rf233_fb_t fb;
    at86rf233_fb_read(dev, &fb, buf, len);
    if (info) {
        ((netdev_ieee802154_rx_info_t *)info)->lqi = fb.lqi;
        ((netdev_ieee802154_rx_info_t *)info)->rssi = fb.ed +
                                                      AT86RF233_RSSI_BASE_VAL;
        AT86RF233_DEBUG("LQI: %d\n",
                        ((netdev_ieee802154_rx_info_t *)info)->lqi);
        AT86RF233_DEBUG("RSSI: %d\n",
                        ((netdev_ieee802154_rx_info_t *)info)->rssi);
    }
    _set_state_netopt(dev,NETOPT_STATE_IDLE);
    return frame_len;
}

static
void _isr(netdev_t *netdev)
{
    at86rf233_t *dev = (at86rf233_t *)netdev;

    gpio_irq_enable(dev->params.base_params.int_pin);
    dev->base.flags &= ~AT86RF2XX_FLG_PENDING_IRQ;

    /* If transceiver is sleeping register access is impossible and frames are
     * lost anyway, so return immediately. */
    uint8_t state = at86rf2xx_get_state((at86rf2xx_t *)dev);
    if (state == AT86RF2XX_STATE_SLEEP) {
        return;
    }

    uint8_t irq = at86rf2xx_spi_reg_read((at86rf2xx_t *)dev,
                                         AT86RF2XX_REG__IRQ_STATUS);

    if (irq & AT86RF2XX_BAT_LOW__HIGH) {
        AT86RF2XX_DEBUG_PUTS("IRQ: BAT_LOW");
    }
    if (irq & AT86RF2XX_TRX_UR__HIGH) {
        AT86RF2XX_DEBUG_PUTS("IRQ: TRX_UR");
    }
    if (irq & AT86RF2XX_AMI__HIGH) {
        AT86RF2XX_DEBUG_PUTS("IRQ: AMI");
    }
    if (irq & AT86RF2XX_CCA_ED_DONE__HIGH) {
        AT86RF233_DEBUG("IRQ: CCA_ED_DONE");
    }
    if (irq & AT86RF2XX_RX_START__HIGH) {
        AT86RF2XX_DEBUG_PUTS("IRQ: RX_START");
        if (dev->base.flags & AT86RF2XX_OPT_TELL_RX_START) {
            netdev->event_callback(netdev, AT86RF2XX_OPT_TELL_RX_START);
        }
    }
    if (irq & AT86RF2XX_PLL_UNLOCK__HIGH) {
        AT86RF2XX_DEBUG_PUTS("IRQ: PLL_UNLOCK");
    }
    if (irq & AT86RF2XX_PLL_LOCK__HIGH) {
        AT86RF2XX_DEBUG_PUTS("IRQ: PLL_LOCK");
    }
    if (irq & AT86RF2XX_TRX_END__HIGH) {
        AT86RF2XX_DEBUG_PUTS("IRQ: TRX_END");
        uint8_t trac =
            at86rf2xx_spi_reg_read((at86rf2xx_t *)dev, AT86RF2XX_REG__TRX_STATE) &
            AT86RF2XX_TRX_STATE_MASK__TRAC_STATUS;
        if (trac == AT86RF2XX_TRAC_STATUS__TRAC_SUCCESS) {
            AT86RF2XX_DEBUG_PUTS("TRAC: SUCCESS");
            if (at86rf2xx_is_rx_state(state)) {
                if (dev->base.flags & AT86RF2XX_OPT_TELL_RX_END) {
                    netdev->event_callback(netdev, NETDEV_EVENT_RX_COMPLETE);
                }
            }
            else if (at86rf2xx_is_tx_state(state)) {
                assert(dev->base.pending_tx);
                dev->base.pending_tx--;
                if (dev->base.flags & AT86RF2XX_OPT_TELL_TX_END) {
                    netdev->event_callback(netdev, NETDEV_EVENT_TX_COMPLETE);
                }
            }
        }
        else if (trac == AT86RF2XX_TRAC_STATUS__TRAC_SUCCESS_DATA_PENDING) {
            AT86RF2XX_DEBUG_PUTS("TRAC: SUCCESS_DATA_PENDING");
            assert(dev->base.pending_tx);
            dev->base.pending_tx--;
            if (dev->base.flags & AT86RF2XX_OPT_TELL_TX_END) {
                netdev->event_callback(netdev, NETDEV_EVENT_TX_COMPLETE_DATA_PENDING);
            }
        }
        else if (trac == AT86RF2XX_TRAC_STATUS__TRAC_SUCCESS_WAIT_FOR_ACK) {
            AT86RF2XX_DEBUG_PUTS("TRAC: SUCCESS_WAIT_FOR_ACK");
            /* Indicates an ACK frame is about to be sent in RX_AACK
               slotted acknowledgement. */
                if (dev->base.flags & AT86RF2XX_OPT_TELL_RX_END) {
                    netdev->event_callback(netdev, NETDEV_EVENT_RX_COMPLETE);
                }
        }
        else if (trac == AT86RF2XX_TRAC_STATUS__TRAC_CHANNEL_ACCESS_FAILURE) {
            AT86RF2XX_DEBUG_PUTS("TRAC: CHANNEL_ACCESS_FAILURE");
            assert(dev->base.pending_tx);
            dev->base.pending_tx--;
            netdev->event_callback(netdev, NETDEV_EVENT_TX_MEDIUM_BUSY);
        }
        else if (trac == AT86RF2XX_TRAC_STATUS__TRAC_NO_ACK) {
            AT86RF2XX_DEBUG_PUTS("TRAC: NO_ACK");
            assert(dev->base.pending_tx);
            dev->base.pending_tx--;
            netdev->event_callback(netdev, NETDEV_EVENT_TX_NOACK);
        }
        else if (trac == AT86RF2XX_TRAC_STATUS__TRAC_INVALID) {
            /* Even though the reset value for register bits TRAC_STATUS is zero,
               the RX_AACK and TX_ARET procedures set the register bits to TRAC_STATUS = 7 (INVALID)
               when they are started. */
            AT86RF2XX_DEBUG_PUTS("TRAC: INVALID");
            if (at86rf2xx_is_rx_state(state)) {
                if (irq & AT86RF2XX_AMI__HIGH ||
                    dev->base.flags | AT86RF2XX_OPT_PROMISCUOUS) {
                    uint8_t crc =
                        at86rf2xx_spi_reg_read((at86rf2xx_t *)dev, AT86RF2XX_REG__PHY_RSSI) &
                        AT86RF2XX_PHY_RSSI_MASK__RX_CRC_VALID;
                    if (dev->base.flags & AT86RF2XX_OPT_TELL_RX_END) {
                        netdev->event_callback(netdev, crc
                                                    ? NETDEV_EVENT_RX_COMPLETE
                                                    : NETDEV_EVENT_CRC_ERROR);
                    }
                }
            }
            else if (at86rf2xx_is_tx_state(state)) {
                assert(dev->base.pending_tx);
                dev->base.pending_tx--;
            }

        }
        if (!dev->base.pending_tx && (dev->base.idle_state != dev->base.state)) {
            /* go back to idle state which has been saved in _send() */
            _set_state_netopt(dev,NETOPT_STATE_IDLE);
        }
    }
}

static
bool _netopt_get_require_wakeup(netopt_t opt)
{
    switch (opt) {
        default: return false;
        case NETOPT_IS_CHANNEL_CLR:
        case NETOPT_PROMISCUOUSMODE:
        case NETOPT_AUTOACK:
        case NETOPT_RETRANS:
        case NETOPT_CSMA:
        case NETOPT_CSMA_MAXBE:
        case NETOPT_CSMA_MINBE:
        case NETOPT_CCA_THRESHOLD:
        case NETOPT_LAST_ED_LEVEL:
        case NETOPT_TX_RETRIES_NEEDED:
            return true;
    }
}

static
int _get(netdev_t *netdev, netopt_t opt, void *val, size_t max_len)
{
    at86rf233_t *dev = (at86rf233_t *)netdev;

    /* wake up because we need SPI */
    uint8_t old_state = at86rf2xx_get_state((at86rf2xx_t *)dev);
    if (_netopt_get_require_wakeup(opt)) {
        at86rf233_assert_awake(dev);
    }

    int res = -ENOTSUP;
    switch (opt) {
        case NETOPT_CCA_THRESHOLD:
            assert(max_len >= sizeof(int8_t));
            *((int8_t *)val) = at86rf233_get_cca_threshold(dev);
            res = sizeof(int8_t);
            break;

        case NETOPT_LAST_ED_LEVEL:
            assert(max_len >= sizeof(int8_t));
            *((int8_t *)val) =
                at86rf2xx_spi_reg_read((at86rf2xx_t *)dev, AT86RF2XX_REG__PHY_ED_LEVEL) +
                AT86RF233_RSSI_BASE_VAL;
            res = sizeof(int8_t);
            break;

        case NETOPT_TX_RETRIES_NEEDED:
            assert(max_len >= sizeof(uint8_t));
            *((uint8_t *)val) =
                (at86rf2xx_spi_reg_read((at86rf2xx_t *)dev, AT86RF2XX_REG__XAH_CTRL_2) &
                AT86RF2XX_XAH_CTRL_2_MASK__ARET_FRAME_RETRIES) >> 4;
            res = sizeof(uint8_t);
            break;

        default: break;
    }

    if (res == -ENOTSUP) {
        res = at86rf2xx_netdev_get((at86rf2xx_t *)dev, opt, val, max_len);
    }

    /* go back to sleep if were sleeping */
    if (old_state == AT86RF2XX_STATE_SLEEP) {
        at86rf233_sleep(dev);
    }

    if (res == -ENOTSUP) {
        AT86RF233_DEBUG("Unsupportet option: %d\n", opt);
    }
    return res;
}

static
bool _netopt_set_require_wakeup(netopt_t opt)
{
    switch (opt) {
        default: return false;
        case NETOPT_CHANNEL:
        case NETOPT_ADDRESS:
        case NETOPT_ADDRESS_LONG:
        case NETOPT_NID:
        case NETOPT_TX_POWER:
        case NETOPT_PROMISCUOUSMODE:
        case NETOPT_AUTOACK:
        case NETOPT_ACK_PENDING:
        case NETOPT_RETRANS:
        case NETOPT_STATE:
        case NETOPT_CSMA:
        case NETOPT_CSMA_RETRIES:
        case NETOPT_CSMA_MAXBE:
        case NETOPT_CSMA_MINBE:
        case NETOPT_CCA_THRESHOLD:
        case NETOPT_ENCRYPTION_KEY:
            return true;
    }
}

static
int _set(netdev_t *netdev, netopt_t opt, const void *val, size_t len)
{
    at86rf233_t *dev = (at86rf233_t *)netdev;

    /* wake up because we need SPI */
    uint8_t old_state = at86rf2xx_get_state((at86rf2xx_t *)dev);
    if (_netopt_set_require_wakeup(opt)) {
        at86rf233_assert_awake(dev);
    }

    int res = -ENOTSUP;
    switch (opt) {
        case NETOPT_TX_POWER:
            assert(len == sizeof(int16_t));
            at86rf233_set_tx_power(dev, *((const int16_t *)val));
            res = sizeof(int16_t);
            break;

        case NETOPT_STATE:
            assert(len == sizeof(netopt_state_t));
            res = _set_state_netopt(dev, *((const netopt_state_t *)val));
            break;

        case NETOPT_CSMA:
            assert(len == sizeof(netopt_enable_t));
            int8_t frame_retr = (*((netopt_enable_t *)val))
                ? AT86RF233_MAX_CSMA_RETRIES_DEFAULT : -1;
            at86rf2xx_set_csma_max_retries((at86rf2xx_t *)dev, frame_retr);
            res = sizeof(netopt_enable_t);
            break;

        case NETOPT_CCA_THRESHOLD:
            assert(len == sizeof(int8_t));
            at86rf233_set_cca_threshold(dev, *((int8_t *)val));
            res = sizeof(int8_t);
            break;
#if IS_USED(MODULE_AT86RF2XX_COMMON_AES_SPI) && \
    IS_USED(MODULE_IEEE802154_SECURITY)
        /* override NETOPT_ENCRYPTION_KEY from netdev_ieee802154_t */
        case NETOPT_ENCRYPTION_KEY:
            assert(len >= IEEE802154_SEC_KEY_LENGTH);
            at86rf2xx_aes_key_write_encrypt((at86rf2xx_t *)dev, val);
            if (memcmp(dev->base.netdev.sec_ctx.cipher.context.context, val, len)) {
                /* If the key changes, the frame conter can be reset to 0*/
                dev->base.netdev.sec_ctx.frame_counter = 0;
            }
            memcpy(dev->base.netdev.sec_ctx.cipher.context.context, val,
                   IEEE802154_SEC_KEY_LENGTH);
            res = IEEE802154_SEC_KEY_LENGTH;
            break;
#endif
        default: break;
    }

    if (res == -ENOTSUP) {
        res = at86rf2xx_netdev_set((at86rf2xx_t *)dev, opt, val, len);
    }

    /* go back to sleep if were sleeping */
    if (old_state == AT86RF2XX_STATE_SLEEP) {
        at86rf233_sleep(dev);
    }

    if (res == -ENOTSUP) {
        AT86RF233_DEBUG("Unsupportet option: %d\n", opt);
    }
    return res;
}
