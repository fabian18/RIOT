/*
 * Copyright (C) 2015-2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_rp2040
 * @{
 *
 * @file
 * @brief           RP2040 atomic register access macros
 * @details         This allows individual fields of a control register to be
 *                  modified without performing a read-modify-write sequence
 *                  [datasheet 2.1.12].
 *
 * @author          Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 * @author          Fabian Hüßler <fabian.huessler@ovgu.de>
 */

#ifndef CPU_REG_ATOMIC_H
#define CPU_REG_ATOMIC_H

#include <stdint.h>

#include "core_cm0plus.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Bit to be set if an atomic XOR operation shall be done
 */
#define REG_ATOMIC_XOR_BIT      (0x1000U)
/**
 * @brief   Bit to be set if an atomic set operation shall be done
 */
#define REG_ATOMIC_SET_BIT      (0x2000U)
/**
 * @brief   Bit to be set if an atomic clear operation shall be done
 */
#define REG_ATOMIC_CLEAR_BIT    (0x3000U)

/**
 * @brief   The operation to be performed to the register at address @p reg
 *          will be an atomic XOR of the bits of the right-hand-side operand
 */
#define REG_ATOMIC_XOR(reg)     ((__IOM uint32_t *)(((uintptr_t)(reg)) | REG_ATOMIC_XOR_BIT))
/**
 * @brief   The operation to be performed to the register at address @p reg
 *          will be an atomic set of the bits of the right-hand-side operand
 */
#define REG_ATOMIC_SET(reg)     ((__IOM uint32_t *)(((uintptr_t)(reg)) | REG_ATOMIC_SET_BIT))
/**
 * @brief   The operation to be performed to the register at address @p reg
 *          will be an atomic clear of the bits of the right-hand-side operand
 */
#define REG_ATOMIC_CLEAR(reg)   ((__IOM uint32_t *)(((uintptr_t)(reg)) | REG_ATOMIC_CLEAR_BIT))
/**
 * @brief   Performed an atomic XOR of the set bits in @p op
 *          with the bits in the register at address @p reg
 *
 * @param   reg     Register address
 * @param   mask    Mask of bits to be XORed
 */
static inline void reg_atomic_xor(__IOM void *reg, uint32_t mask)
{
    *REG_ATOMIC_XOR(reg) = mask;
}
/**
 * @brief   Set the bits in the register at address @p reg as given by
 *          the set bits in operand @p op
 *
 * @param   reg     Register address
 * @param   mask    Mask of bits to be set
 */
static inline void reg_atomic_set(__IOM void *reg, uint32_t mask)
{
    *REG_ATOMIC_SET(reg) = mask;
}
/**
 * @brief   Clear the bits in the register at address @p reg as given by
 *          the set bits in operand @p op
 *
 * @param   reg     Register address
 * @param   mask    Mask of bits to be cleared
 */
static inline void reg_atomic_clear(__IOM void *reg, uint32_t mask)
{
    *REG_ATOMIC_CLEAR(reg) = mask;
}
/**
 * @brief   Assign the masked bits in the register at address @p reg to
 *          the value @p value
 *
 * @param   reg     Register address
 * @param   value   Value to be assigned
 * @param   mask    Mask of bits to be assigned
 */
static inline void reg_atomic_assign(__IOM void *reg, uint32_t value, uint32_t mask)
{
    *REG_ATOMIC_XOR(reg) = ((*((__IOM uint32_t *)reg) ^ value) & mask);
}

#ifdef __cplusplus
}
#endif

#endif /* CPU_REG_ATOMIC_H */
/** @} */
