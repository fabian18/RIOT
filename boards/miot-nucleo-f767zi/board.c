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
 * @brief       Board initialization code for the MIOT nucleo-f767zi board
 *
 * @author      Maximilian Deubel <maximilian.deubel@ovgu.de>
 *
 * @}
 */

#include "board.h"
#include "periph/gpio.h"

void board_init(void)
{
    /* initialize the CPU */
    cpu_init();
#ifdef LED1_PIN
    gpio_init(LED1_PIN, GPIO_OUT);
#endif
#ifdef LED2_PIN
    gpio_init(LED2_PIN, GPIO_OUT);
#endif
    /* initialize SPI chip select lines */
    gpio_init(CC110X_PARAM_CS, GPIO_OUT);
    gpio_set(CC110X_PARAM_CS);
    gpio_init(SX127X_PARAM_SPI_NSS, GPIO_OUT);
    gpio_set(SX127X_PARAM_SPI_NSS);
    gpio_init(NRF24L01P_NG_PARAM_CS, GPIO_OUT);
    gpio_set(NRF24L01P_NG_PARAM_CS);
    gpio_init(AT86RF2XX_PARAM_CS, GPIO_OUT);
    gpio_set(AT86RF2XX_PARAM_CS);
}
