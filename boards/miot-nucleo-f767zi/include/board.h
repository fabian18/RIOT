/*
 * Copyright (C) 2017 Inria
 *               2017 OTAKeys
 *               2018 Freie Universität Berlin
 *               2019 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    miot_board_nucleo-f767zi MIOT nucleo-f767zi
 * @ingroup     boards
 * @brief       Support for the MIOT nucleo-f767zi board
 * @{
 *
 * @file
 * @brief       Common pin definitions and board configuration options
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @author      Vincent Dupont <vincent@otakeys.com>
 * @author      Sebastian Meiling <s@mlng.net>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Maximilian Deubel <maximilian.deubel@ovgu.de>
 */

#ifndef BOARD_H
#define BOARD_H

#include "board_nucleo.h"
#include "arduino_pinmap.h"
#include "periph_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    LED pin definitions and handlers
 * @{
 */
#if defined(CPU_MODEL_STM32L496ZG) || defined(CPU_MODEL_STM32L4R5ZI)
#define LED0_PORT           GPIOC
#define LED0_PIN            GPIO_PIN(PORT_C, 7)
#define LED0_MASK           (1 << 7)
#else
#define LED0_PORT           GPIOB
#define LED0_PIN            GPIO_PIN(PORT_B, 0)
#define LED0_MASK           (1 << 0)
#endif
#define LED0_ON             (LED0_PORT->BSRR = LED0_MASK)
#define LED0_OFF            (LED0_PORT->BSRR = (LED0_MASK << 16))
#define LED0_TOGGLE         (LED0_PORT->ODR  ^= LED0_MASK)

#define LED1_PIN            GPIO_PIN(PORT_B, 7)
#define LED1_MASK           (1 << 7)
#define LED1_ON             (GPIOB->BSRR = LED1_MASK)
#define LED1_OFF            (GPIOB->BSRR = (LED1_MASK << 16))
#define LED1_TOGGLE         (GPIOB->ODR  ^= LED1_MASK)

#define LED2_PIN            GPIO_PIN(PORT_B, 14)
#define LED2_MASK           (1 << 14)
#define LED2_ON             (GPIOB->BSRR = LED2_MASK)
#define LED2_OFF            (GPIOB->BSRR = (LED2_MASK << 16))
#define LED2_TOGGLE         (GPIOB->ODR  ^= LED2_MASK)

/* the Nucleo144 boards always use LED0, as there is no dual use of its pin */
#ifndef AUTO_INIT_LED0
#define AUTO_INIT_LED0
#endif
/** @} */

/**
 * @name    User button
 * @{
 */
#define BTN0_PIN            GPIO_PIN(PORT_C, 13)
#define BTN0_MODE           GPIO_IN_PD
/** @} */

/**
 * @name    CC1101 @ 433MHz
 * @{
 */
#define CC110X_PARAM_SPI                    (SPI_DEV(0))
#define CC110X_PARAM_SPI_CLOCK              (SPI_CLK_5MHZ)
#define CC110X_PARAM_CS                     (GPIO_PIN(PORT_D, 15))
#define CC110X_PARAM_GDO0                   (GPIO_PIN(PORT_A, 4))
#define CC110X_PARAM_GDO2                   (GPIO_PIN(PORT_D, 14))
#define CC110X_PARAM_L2ADDR                 (CC110X_L2ADDR_AUTO)
#define CC110X_PARAM_PATABLE                (&cc110x_patable_433mhz)
#define CC110X_PARAM_CONFIG                 (&cc110x_config_433mhz_250kbps_300khz)
#define CC110X_PARAM_CHANNELS               (&cc110x_chanmap_433mhz_300khz)
/** @} */

/**
 * @name    RMF95W @ 868MHz
 * @{
 */
#define SX127X_PARAM_SPI                    (SPI_DEV(1))
#define SX127X_PARAM_SPI_NSS                (GPIO_PIN(PORT_C, 8))
#define SX127X_PARAM_RESET                  (GPIO_PIN(PORT_D, 2))
#define SX127X_PARAM_DIO0                   (GPIO_PIN(PORT_F, 5))
#define SX127X_PARAM_DIO1                   (GPIO_PIN(PORT_F, 10))
#define SX127X_PARAM_DIO2                   (GPIO_PIN(PORT_F, 2))
#define SX127X_PARAM_DIO3                   (GPIO_PIN(PORT_F, 3))
#define SX127X_PARAM_DIO4                   (GPIO_PIN(PORT_D, 3))
#define SX127X_PARAM_DIO5                   (GPIO_PIN(PORT_C, 9))
#define SX127X_PARAM_PASELECT               (SX127X_PA_BOOST)
/** @} */

/**
 * @name    NRF24L01+ (no usable driver currently)
 * @{
 */
#define NRF24L01P_NG_ADDR_WIDTH                (5)
#define NRF24L01P_NG_PARAM_SPI                 (SPI_DEV(2))
#define NRF24L01P_NG_PARAM_SPI_CLK             (SPI_CLK_5MHZ)
#define NRF24L01P_NG_PARAM_CS                  (GPIO_PIN(PORT_A, 15))
#define NRF24L01P_NG_PARAM_CE                  (GPIO_PIN(PORT_B, 12))
#define NRF24L01P_NG_PARAM_IRQ                 (GPIO_PIN(PORT_C, 7))
#define NRF24L01P_NG_PARAM_RX_P0               NRF24L01P_NG_L2ADDR_AUTO
#define NRF24L01P_NG_PARAM_RX_P1               NRF24L01P_NG_BROADCAST_ADDR
/** @} */

/**
 * @name    AT86RF233
 * @{
 */
#define AT86RF2XX_PARAM_SPI                 (SPI_DEV(3))
#define AT86RF2XX_PARAM_SPI_CLK             (SPI_CLK_5MHZ)
#define AT86RF2XX_PARAM_CS                  (GPIO_PIN(PORT_F, 1))
#define AT86RF2XX_PARAM_INT                 (GPIO_PIN(PORT_G, 1))
#define AT86RF2XX_PARAM_SLEEP               (GPIO_PIN(PORT_E, 3))
#define AT86RF2XX_PARAM_RESET               (GPIO_PIN(PORT_F, 0))
/** @} */

/**
 * @name    ESP32-SLIP
 * @{
 */
#define SLIP_BAUDRATE                       (115200)
#define SLIP_UART                           (UART_DEV(2))
/** @} */

/**
 * @name    BME680 (I2C)
 * @{
 */
#define BME680_PARAM_I2C_DEV                (I2C_DEV(0))
#define BME680_PARAM_I2C_ADDR               (BME680_I2C_ADDR_2)
/** @} */

/**
 * @name    SHT31
 * @{
 */
#define SHT3X_PARAM_I2C_DEV                 (I2C_DEV(0))
#define SHT3X_PARAM_I2C_ADDR                (SHT3X_I2C_ADDR_1)
#define SHT3X_PARAM_MODE                    (SHT3X_PERIODIC_2_MPS)
#define SHT3X_PARAM_REPEAT                  (SHT3X_HIGH)
/** @} */

/**
 * @name AT24C256
 * @{
 */
#define AT24CXXX_PARAM_I2C                  (I2C_DEV(0))
#define AT24CXXX_PARAM_I2C_ADDR             (AT24CXXX_DEF_DEV_ADDR)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
