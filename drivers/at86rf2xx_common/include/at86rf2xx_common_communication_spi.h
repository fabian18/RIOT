/* Copyright (C) 2013 Alaeddine Weslati <alaeddine.weslati@inria.fr>
 *               2015 Freie Universität Berlin
 *               2015 Kaspar Schleiser <kaspar@schleiser.de>
 *               2015 Hauke Petersen <hauke.petersen@fu-berlin.de>
 *               2017 HAW Hamburg
 *               2020 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     drivers_at86rf2xx_common
 * @{
 *
 * @file
 * @brief       AT86RFxx SPI communication interface
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
 */
#ifndef AT86RF2XX_COMMON_COMMUNICATION_SPI_H
#define AT86RF2XX_COMMON_COMMUNICATION_SPI_H

#include "periph/gpio.h"
#include "periph/spi.h"
#include "at86rf2xx_common.h"

#define AT86RF2XX_REG_READ                  (0b10000000)
#define AT86RF2XX_REG_WRITE                 (0b11000000)
#define AT86RF2XX_REG_MASK                  (0b00111111)
#define AT86RF2XX_CMD_REG_READ(reg)         (AT86RF2XX_REG_READ | \
                                            (AT86RF2XX_REG_MASK & (reg)))
#define AT86RF2XX_CMD_REG_WRITE(reg)        (AT86RF2XX_REG_WRITE | \
                                            (AT86RF2XX_REG_MASK & (reg)))
#define AT86RF2XX_CMD_FB_READ               (0b00100000)
#define AT86RF2XX_CMD_FB_WRITE              (0b01100000)
#define AT86RF2XX_CMD_SRAM_READ             (0b00000000)
#define AT86RF2XX_CMD_SRAM_WRITE            (0b01000000)

/**
 * @name Functions to communicate with transceivers via SPI
 * @{
 */
static inline
void at86rf2xx_spi_get_bus(const at86rf2xx_t *dev)
{
    spi_acquire(dev->params.spi, dev->params.cs_pin, SPI_MODE_0, dev->params.spi_clk);
}

static inline
void at86rf2xx_spi_release_bus(const at86rf2xx_t *dev)
{
    spi_release(dev->params.spi);
}

static inline
uint8_t at86rf2xx_spi_reg_read(const at86rf2xx_t *dev, uint8_t addr)
{
    at86rf2xx_spi_get_bus(dev);
    uint8_t value = spi_transfer_reg(dev->params.spi, dev->params.cs_pin, AT86RF2XX_CMD_REG_READ(addr), 0);
    at86rf2xx_spi_release_bus(dev);
    return value;
}

static inline
void at86rf2xx_spi_reg_write(const at86rf2xx_t *dev, uint8_t addr, uint8_t value)
{
    at86rf2xx_spi_get_bus(dev);
    spi_transfer_reg(dev->params.spi, dev->params.cs_pin, AT86RF2XX_CMD_REG_WRITE(addr), value);
    at86rf2xx_spi_release_bus(dev);
}

static inline
void at86rf2xx_spi_sram_read(const at86rf2xx_t *dev, uint8_t offset, void *data, size_t len)
{
    at86rf2xx_spi_get_bus(dev);
    spi_transfer_byte(dev->params.spi, dev->params.cs_pin, true, AT86RF2XX_CMD_SRAM_READ);
    spi_transfer_byte(dev->params.spi, dev->params.cs_pin, true, offset);
    spi_transfer_bytes(dev->params.spi, dev->params.cs_pin, false, NULL, data, len);
    at86rf2xx_spi_release_bus(dev);
}

static inline
void at86rf2xx_spi_sram_write(const at86rf2xx_t *dev, uint8_t offset, const void *data, size_t len)
{
    at86rf2xx_spi_get_bus(dev);
    spi_transfer_byte(dev->params.spi, dev->params.cs_pin, true, AT86RF2XX_CMD_SRAM_WRITE);
    spi_transfer_byte(dev->params.spi, dev->params.cs_pin, true, offset);
    spi_transfer_bytes(dev->params.spi, dev->params.cs_pin, false, data, NULL, len);
    at86rf2xx_spi_release_bus(dev);
}

static inline
void at86rf2xx_spi_fb_start_read(const at86rf2xx_t *dev)
{
    at86rf2xx_spi_get_bus(dev);
    spi_transfer_byte(dev->params.spi, dev->params.cs_pin, true, AT86RF2XX_CMD_FB_READ);
}

static inline
void at86rf2xx_spi_fb_start_write(const at86rf2xx_t *dev)
{
    at86rf2xx_spi_get_bus(dev);
    spi_transfer_byte(dev->params.spi, dev->params.cs_pin, true, AT86RF2XX_CMD_FB_WRITE);
}

static inline
void at86rf2xx_spi_fb_read(const at86rf2xx_t *dev, void *data, size_t len)
{
    spi_transfer_bytes(dev->params.spi, dev->params.cs_pin, true, NULL, data, len);
}

static inline
void at86rf2xx_spi_fb_write(const at86rf2xx_t *dev, const void *data, size_t len)
{
    spi_transfer_bytes(dev->params.spi, dev->params.cs_pin, true, data, NULL, len);
}

static inline
void at86rf2xx_spi_fb_stop(const at86rf2xx_t *dev)
{
    /* transfer one byte (which we ignore) to release the chip select */
    spi_transfer_byte(dev->params.spi, dev->params.cs_pin, false, 1);
    at86rf2xx_spi_release_bus(dev);
}

static inline
uint8_t at86rf2xx_spi_reg_clear(const at86rf2xx_t *dev, uint8_t addr, uint8_t mask)
{
    at86rf2xx_spi_get_bus(dev);
    uint8_t reg = spi_transfer_reg(dev->params.spi, dev->params.cs_pin, AT86RF2XX_CMD_REG_READ(addr), 0);
    reg &= ~(mask);
    spi_transfer_reg(dev->params.spi, dev->params.cs_pin, AT86RF2XX_CMD_REG_WRITE(addr), reg);
    at86rf2xx_spi_release_bus(dev);
    return reg;
}

static inline
uint8_t at86rf2xx_spi_reg_set(const at86rf2xx_t *dev, uint8_t addr, uint8_t mask, uint8_t value)
{
    at86rf2xx_spi_get_bus(dev);
    uint8_t reg = spi_transfer_reg(dev->params.spi, dev->params.cs_pin, AT86RF2XX_CMD_REG_READ(addr), 0);
    reg &= ~(mask);
    reg |= (mask & value);
    spi_transfer_reg(dev->params.spi, dev->params.cs_pin, AT86RF2XX_CMD_REG_WRITE(addr), reg);
    at86rf2xx_spi_release_bus(dev);
    return reg;
}
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AT86RF2XX_COMMON_COMMUNICATION_SPI_H */
/** @} */
