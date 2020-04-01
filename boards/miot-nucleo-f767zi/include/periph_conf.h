/*
 * Copyright (C) 2019 Otto-von-Guericke Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     miot_board_nucleo-f767zi
 * @{
 *
 * @file
 * @brief       Peripheral MCU configuration for the MIOT nucleo-f767zi board
 *
 * @author      Maximilian Deubel <maximilian.deubel@ovgu.de>
 * @author      Fabian Hüßler <fabian.huessler@ovgu.de>
 */

#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include "periph_cpu.h"
#include "f7/cfg_clock_216_8_1.h"
#include "cfg_spi_divtable.h"
#include "cfg_rtt_default.h"
#include "cfg_timer_tim2.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    DMA streams configuration
 * @{
 */
#ifdef MODULE_PERIPH_DMA
static const dma_conf_t dma_config[] = {
    { .stream = 4 },    /* DMA1 Stream 4 - USART3_TX */
    { .stream = 14 },   /* DMA2 Stream 6 - USART6_TX */
    { .stream = 6 },    /* DMA1 Stream 6 - USART2_TX */
    { .stream = 8 },    /* DMA2 Stream 0 - ETH_TX    */
};

#define DMA_0_ISR  isr_dma1_stream4
#define DMA_1_ISR  isr_dma2_stream6
#define DMA_2_ISR  isr_dma1_stream6
#define DMA_3_ISR  isr_dma2_stream0

#define DMA_NUMOF           ARRAY_SIZE(dma_config)
#endif
/** @} */

/**
 * @name I2C configuration
 * @{
 */
static const i2c_conf_t i2c_config[] = {
#if 0
    {
        .dev            = I2C1,
        .speed          = I2C_SPEED_NORMAL,
        .scl_pin        = GPIO_PIN(PORT_B, 8),  /* alt. PB6 */
        .sda_pin        = GPIO_PIN(PORT_B, 9),  /* alt. PB7 */
        .scl_af         = GPIO_AF4,
        .sda_af         = GPIO_AF4,
        .bus            = APB1,
        .rcc_mask       = RCC_APB1ENR_I2C1EN,
        .irqn           = I2C1_ER_IRQn,
    },
#endif
    {
        .dev            = I2C2,                 /* BME680 (I2C), SHT31, AT24C256 */
        .speed          = I2C_SPEED_NORMAL,
        .scl_pin        = GPIO_PIN(PORT_B, 10), /* alt. PF1 */
        .sda_pin        = GPIO_PIN(PORT_B, 11), /* alt. PF0 */
        .scl_af         = GPIO_AF4,
        .sda_af         = GPIO_AF4,
        .bus            = APB1,
        .rcc_mask       = RCC_APB1ENR_I2C2EN,
        .irqn           = I2C2_EV_IRQn,
    },
#if 0
    {
        .dev            = I2C3,
        .speed          = I2C_SPEED_NORMAL,
        .scl_pin        = GPIO_PIN(PORT_A, 8),  /* no bridge on board! */
        .sda_pin        = GPIO_PIN(PORT_C, 9),
        .scl_af         = GPIO_AF4,
        .sda_af         = GPIO_AF4,
        .bus            = APB1,
        .rcc_mask       = RCC_APB1ENR_I2C3EN,
        .irqn           = I2C3_EV_IRQn,
    },
#endif
    {
        .dev            = I2C4,                 /* I2C connector */
        .speed          = I2C_SPEED_NORMAL,
        .scl_pin        = GPIO_PIN(PORT_F, 14), /* alt. PD12 PH11 PB6 PB8 */
        .sda_pin        = GPIO_PIN(PORT_F, 15), /* alt. PD13 PH12 PB7 PB9 */
        .scl_af         = GPIO_AF4,
        .sda_af         = GPIO_AF4,
        .bus            = APB1,
        .rcc_mask       = RCC_APB1ENR_I2C4EN,
        .irqn           = I2C4_ER_IRQn,
    }
};

#define I2C_0_ISR           isr_i2c2_er
#define I2C_1_ISR           isr_i2c4_er

#define I2C_NUMOF           ARRAY_SIZE(i2c_config)
/** @} */

/**
 * @name    UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
#if 0
    {
        .dev        = USART1, /* gpio´s not on board */
        .rcc_mask   = RCC_APB2ENR_USART1EN,
        .rx_pin     = GPIO_PIN(PORT_B, 15), /* alt. PG10 PA10 PB7 */
        .tx_pin     = GPIO_PIN(PORT_B, 14), /* alt. PH10 PA9 PB6 */
        .rx_af      = GPIO_AF4,
        .tx_af      = GPIO_AF4,
        .bus        = APB2,
        .irqn       = USART1_IRQn,
#ifdef MODULE_PERIPH_DMA
#endif
    },
#endif
    {
        .dev        = USART3,                   /* STLink UART  */
        .rcc_mask   = RCC_APB1ENR_USART3EN,
        .rx_pin     = GPIO_PIN(PORT_D, 9),      /* alt. PC11 PB11 */
        .tx_pin     = GPIO_PIN(PORT_D, 8),      /* alt. PC10 PB10 */
        .rx_af      = GPIO_AF7,
        .tx_af      = GPIO_AF7,
        .bus        = APB1,
        .irqn       = USART3_IRQn,
#ifdef MODULE_PERIPH_DMA
        .dma        = 0,
        .dma_chan   = 7
#endif
    },
#if 0
    {
        .dev        = UART4,
        .rcc_mask   = RCC_APB1ENR_UART4EN,
        .rx_pin     = GPIO_PIN(PORT_D, 0), /* alt. PH14 PC11 PA11 PA1 PI1 */
        .tx_pin     = GPIO_PIN(PORT_D, 1), /* alt. PH13 PC10 PA12 */
        .rx_af      = GPIO_AF8,
        .tx_af      = GPIO_AF8,
        .bus        = APB1,
        .irqn       = UART4_IRQn,
#ifdef MODULE_PERIPH_DMA
#endif
    },
#endif
#if 0
    {
        .dev        = UART5,
        .rcc_mask   = RCC_ABP1ENR_UART5EN,
        .rx_pin     = GPIO_PIN(PORT_D, 2), /* alt. PB12 PB8 PB5*/
        .tx_pin     = GPIO_PIN(PORT_C, 12), /* alt. PB13 PB9 PB6 */
        .rx_af      = GPIO_AF8,
        .tx_af      = GPIO_AF8,
        .bus        = APB1,
        .irqn       = UART5_IRQn,
#ifdef MODULE_PERIPH_DMA
#endif
    },
#endif
    {
        .dev        = USART6, // Not connected
        .rcc_mask   = RCC_APB2ENR_USART6EN,
        .rx_pin     = GPIO_PIN(PORT_G, 9),
        .tx_pin     = GPIO_PIN(PORT_G, 14),
        .rx_af      = GPIO_AF8,
        .tx_af      = GPIO_AF8,
        .bus        = APB2,
        .irqn       = USART6_IRQn,
#ifdef MODULE_PERIPH_DMA
        .dma        = 1,
        .dma_chan   = 5
#endif
    },
    {
        .dev        = USART2,               /* ESP32    */
        .rcc_mask   = RCC_APB1ENR_USART2EN,
        .rx_pin     = GPIO_PIN(PORT_D, 6),
        .tx_pin     = GPIO_PIN(PORT_D, 5),
        .rx_af      = GPIO_AF7,
        .tx_af      = GPIO_AF7,
        .bus        = APB1,
        .irqn       = USART2_IRQn,
#ifdef MODULE_PERIPH_DMA
        .dma        = 2,
        .dma_chan   = 4
#endif
    },
#if 0
    {
        .dev        = UART7,
        .rcc_mask   = RCC_APB1ENR_UART7EN,
        .rx_pin     = GPIO_PIN(PORT_F, 6), /* alt. PE7 PB3 PA8 */
        .tx_pin     = GPIO_PIN(PORT_F, 7), /* alt. PE8 PB4 PA15 */
        .rx_af      = GPIO_AF8,
        .tx_af      = GPIO_AF8,
        .bus        = APB1,
        .irqn       = UART7_IRQn,
#ifdef MODULE_PERIPH_DMA
#endif
    },
#endif
#if 0
    {
        .dev        = UART8,
        .rcc_mask   = RCC_APB1ENR_UART8,
        .rx_pin     = GPIO_PIN(PORT_E, 0),
        .tx_pin     = GPIO_PIN(PORT_E, 1),
        .rx_af      = GPIO_AF8,
        .tx_af      = GPIO_AF8,
        .bus        = APB1,
        .irqn       = UART8_IRQn,
    },
#endif
};

#define UART_0_ISR          (isr_usart3)
#define UART_1_ISR          (isr_usart6)
#define UART_2_ISR          (isr_usart2)

#define UART_NUMOF          ARRAY_SIZE(uart_config)
/** @} */

/**
 * @name   SPI configuration
 * @{
 */
static const spi_conf_t spi_config[] = {
    {
        .dev      = SPI1,                   /* CC1101       */
        .mosi_pin = GPIO_PIN(PORT_B, 5),    /* alt. PA7 PD7 */
        .miso_pin = GPIO_PIN(PORT_A, 6),    /* alt. PG9 PB4 */
        .sclk_pin = GPIO_PIN(PORT_A, 5),    /* alt. PG11 PB3*/
        .cs_pin   = GPIO_UNDEF,
        .mosi_af  = GPIO_AF5,
        .miso_af  = GPIO_AF5,
        .sclk_af  = GPIO_AF5,
        .cs_af    = GPIO_AF5,
        .rccmask  = RCC_APB2ENR_SPI1EN,
        .apbbus   = APB2,
#ifdef MODULE_PERIPH_DMA
        .tx_dma   = DMA_STREAM_UNDEF,
        .tx_dma_chan = 0,
        .rx_dma   = DMA_STREAM_UNDEF,
        .rx_dma_chan = 0,
#endif
    },
#if 0
    {
        .dev      = SPI2,
        .mosi_pin = GPIO_PIN(PORT_B, 15), /* alt. PC1 PC3 PI3 */
        .miso_pin = GPIO_PIN(PORT_C, 2),  /* alt. PB14 PI2 */
        .sclk_pin = GPIO_PIN(PORT_D, 3),  /* alt. PA9 PA12 PB10 PB13 PI1 */
        .cs_pin   = GPIO_UNDEF,
        .mosi_af  = GPIO_AF5,
        .miso_af  = GPIO_AF5,
        .sclk_af  = GPIO_AF5,
        .cs_af    = GPIO_AF5,
        .rccmask  = RCC_APB1ENR_SPI2EN,
        .apbbus   = APB1
    },
#endif
    {
        .dev      = SPI3,                   /* RMF95W-868S2     */
        .mosi_pin = GPIO_PIN(PORT_C, 12),   /* alt. PB2 PB5 PD6 */
        .miso_pin = GPIO_PIN(PORT_C, 11),   /* alt. PB4         */
        .sclk_pin = GPIO_PIN(PORT_C, 10),   /* alt. PB3         */
        .cs_pin   = GPIO_UNDEF,
        .mosi_af  = GPIO_AF6,
        .miso_af  = GPIO_AF6,
        .sclk_af  = GPIO_AF6,
        .cs_af    = GPIO_AF6,
        .rccmask  = RCC_APB1ENR_SPI3EN,
        .apbbus   = APB1,
#ifdef MODULE_PERIPH_DMA
        .tx_dma   = DMA_STREAM_UNDEF,
        .tx_dma_chan = 0,
        .rx_dma   = DMA_STREAM_UNDEF,
        .rx_dma_chan = 0,
#endif
    },
    {
        .dev      = SPI4,                   /* NRF24L01+ | SPI_CONN */
        .mosi_pin = GPIO_PIN(PORT_E, 6),    /* alt. PE14            */
        .miso_pin = GPIO_PIN(PORT_E, 5),    /* alt. PE13            */
        .sclk_pin = GPIO_PIN(PORT_E, 2),    /* alt. PE12            */
        .cs_pin   = GPIO_UNDEF,
        .mosi_af  = GPIO_AF5,
        .miso_af  = GPIO_AF5,
        .sclk_af  = GPIO_AF5,
        .cs_af    = GPIO_AF5,
        .rccmask  = RCC_APB2ENR_SPI4EN,
        .apbbus   = APB2,
#ifdef MODULE_PERIPH_DMA
        .tx_dma   = DMA_STREAM_UNDEF,
        .tx_dma_chan = 0,
        .rx_dma   = DMA_STREAM_UNDEF,
        .rx_dma_chan = 0,
#endif
    },
    {
        .dev      = SPI5,                   /* AT86RF233    */
        .mosi_pin = GPIO_PIN(PORT_F, 9),    /* alt. PF11    */
        .miso_pin = GPIO_PIN(PORT_F, 8),    /* alt. PH7     */
        .sclk_pin = GPIO_PIN(PORT_F, 7),    /* alt. PH6     */
        .cs_pin   = GPIO_UNDEF,
        .mosi_af  = GPIO_AF5,
        .miso_af  = GPIO_AF5,
        .sclk_af  = GPIO_AF5,
        .cs_af    = GPIO_AF5,
        .rccmask  = RCC_APB2ENR_SPI5EN,
        .apbbus   = APB2,
#ifdef MODULE_PERIPH_DMA
        .tx_dma   = DMA_STREAM_UNDEF,
        .tx_dma_chan = 0,
        .rx_dma   = DMA_STREAM_UNDEF,
        .rx_dma_chan = 0,
#endif
    },
#if 0
    {
        .dev      = SPI6,                   /* ESP32            */
        .mosi_pin = GPIO_PIN(PORT_B, 5),    /* alt. PA7 PG14    */
        .miso_pin = GPIO_PIN(PORT_B, 4),    /* alt. PA6 PG12    */
        .sclk_pin = GPIO_PIN(PORT_B, 3),    /* alt. PA5 PG13    */
        .cs_pin   = GPIO_UNDEF,
        .mosi_af  = GPIO_AF8,
        .miso_af  = GPIO_AF8,
        .sclk_af  = GPIO_AF8,
        .cs_af    = GPIO_AF8,
        .rccmask  = RCC_APB2ENR_SPI6EN,
        .apbbus   = APB2,
#ifdef MODULE_PERIPH_DMA
        .tx_dma   = DMA_STREAM_UNDEF,
        .tx_dma_chan = 0,
        .rx_dma   = DMA_STREAM_UNDEF,
        .rx_dma_chan = 0,
#endif
    },
#endif
};

#define SPI_NUMOF           ARRAY_SIZE(spi_config)
/** @} */

/**
 * @name ETH configuration
 * @{
 */
static const eth_conf_t eth_config = {
    .mode = RMII,
    .mac = { 0 },
    .speed = ETH_SPEED_100TX_FD,
    .dma = 3,
    .dma_chan = 8,
    .phy_addr = 0x01,
    .pins = {
        GPIO_PIN(PORT_G, 13),
        GPIO_PIN(PORT_B, 13),
        GPIO_PIN(PORT_G, 11),
        GPIO_PIN(PORT_C, 4),
        GPIO_PIN(PORT_C, 5),
        GPIO_PIN(PORT_A, 7),
        GPIO_PIN(PORT_C, 1),
        GPIO_PIN(PORT_A, 2),
        GPIO_PIN(PORT_A, 1),
    }
};

#define ETH_RX_BUFFER_COUNT (4)
#define ETH_TX_BUFFER_COUNT (4)

#define ETH_RX_BUFFER_SIZE (1524)
#define ETH_TX_BUFFER_SIZE (1524)

#define ETH_DMA_ISR        isr_dma2_stream0

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */
