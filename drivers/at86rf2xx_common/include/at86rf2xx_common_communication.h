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
 * @brief       SPI based and peripheral communication interface
 *              for AT86RF2xx drivers
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
 *
 */
#ifndef AT86RF2XX_COMMON_COMMUNICATION_H
#define AT86RF2XX_COMMON_COMMUNICATION_H

#include <stdint.h>

#include "at86rf2xx_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Read from a register at address `addr` from device `dev`.
 *
 * @param[in] dev       device to read from
 * @param[in] addr      address of the register to read
 *
 * @return              the value of the specified register
 */
uint8_t at86rf2xx_reg_read(const at86rf2xx_t *dev, uint8_t addr);

/**
 * @brief   Write to a register at address `addr` from device `dev`.
 *
 * @param[in] dev       device to write to
 * @param[in] addr      address of the register to write
 * @param[in] value     value to write to the given register
 */
void at86rf2xx_reg_write(const at86rf2xx_t *dev, uint8_t addr, uint8_t value);

/**
 * @brief   Read a chunk of data from the SRAM of the given device
 *
 * @param[in]  dev      device to read from
 * @param[in]  offset   starting address to read from [valid 0x00-0x7f]
 * @param[out] data     buffer to read data into
 * @param[in]  len      number of bytes to read from SRAM
 */
void at86rf2xx_sram_read(const at86rf2xx_t *dev, uint8_t offset,
                         void *data, size_t len);
/**
 * @brief   Write a chunk of data into the SRAM of the given device
 *
 * @param[in] dev       device to write to
 * @param[in] offset    address in the SRAM to write to [valid 0x00-0x7f]
 * @param[in] data      data to copy into SRAM
 * @param[in] len       number of bytes to write to SRAM
 */
void at86rf2xx_sram_write(const at86rf2xx_t *dev, uint8_t offset,
                          const void *data, size_t len);

/**
 * @brief   Start a read transcation internal frame buffer of the given device
 *
 * @param[in]  dev      device to start read
 */
void at86rf2xx_fb_start_read(const at86rf2xx_t *dev);

/**
 * @brief   Start a write transcation internal frame buffer of the given device
 *
 * @param[in]  dev      device to start read
 */
void at86rf2xx_fb_start_write(const at86rf2xx_t *dev);

/**
 * @brief   Read from the internal frame buffer of the given device
 *
 * Each read or write of a data byte automatically increments
 * the address counter of the Frame Buffer until the access
 * is terminated by setting /SEL = H.
 *
 * @param[in]  dev      device to read from
 * @param[out] data     buffer to copy the data to
 * @param[in]  len      number of bytes to read from the frame buffer
 */
void at86rf2xx_fb_read(const at86rf2xx_t *dev, void *data, size_t len);

/**
 * @brief   Write to the internal frame buffer of the given device
 *
 * Each read or write of a data byte automatically increments
 * the address counter of the Frame Buffer until the access
 * is terminated by setting /SEL = H.
 *
 * @param[in]  dev      device to read from
 * @param[out] data     buffer to read the data from
 * @param[in]  len      number of bytes to write to the frame buffer
 */
void at86rf2xx_fb_write(const at86rf2xx_t *dev, const void *data, size_t len);

/**
 * @brief   Stop a read/write transcation internal frame buffer of the given device
 *
 * Unlock frame buffer protection.
 *
 * @param[in]  dev      device to stop read
 */
void at86rf2xx_fb_stop(const at86rf2xx_t *dev);

/**
 * @brief   Clear bits in the register at address @p addr,
 *          according to the set bits in @p mask
 *
 * @param[in] dev   device write to
 * @param[in] addr  register address
 * @param[in] mask  bits to be cleared
 *
 * @return new register value
 */
uint8_t at86rf2xx_reg_clear(const at86rf2xx_t *dev, uint8_t addr, uint8_t mask);

/**
 * @brief   Clear bits in the register at address @p addr,
 *          according to the set bits in @p mask and set bits according to the set bits
 *          in @p value
 *
 * @param[in] dev   device write to
 * @param[in] addr  register address
 * @param[in] mask  bits to be cleared
 * @param[in] value bits to be set
 *
 * @return new register value
 */
uint8_t at86rf2xx_reg_set(const at86rf2xx_t *dev, uint8_t addr, uint8_t mask, uint8_t value);


#ifdef __cplusplus
}
#endif

#endif /* AT86RF2XX_COMMON_COMMUNICATION_H */
/** @} */
