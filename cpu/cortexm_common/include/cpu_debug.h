/*
 * Copyright (C) 2021 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_stm32
 * @{
 *
 * @file
 * @brief           Debugging utilities of Cortex-M CPU
 *                  referred to as Data Watchpoint and Trace (DWT)
 *
 * @author          Fabian Hüßler <fabian.huessler@ovgu.de>
 */

#ifndef CPU_DEBUG_H
#define CPU_DEBUG_H

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include "cpu_conf.h"
#include "kernel_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/* defined in core_cmX.h */
#if defined(DWT_CTRL_CYCCNTENA_Pos) || defined(DOXYGEN)
/**
 * @brief   This is defined if the CPU implements debugging capabilities
 */
#define CPU_HAS_DBG_CYCCNT
#endif

#if !defined(ENABLE_DEBUG_CPU) || defined(DOXYGEN)
/**
 * @brief   This macro can be defined as 0 or other on a file-based level.
 *          @ref DEBUG_CPU() will generate output only if ENABLE_DEBUG_CPU is non-zero.
 */
#define ENABLE_DEBUG_CPU 0
#endif

/**
 * @def DEBUG_CPU
 *
 * @brief Print DWT related information to stdout
 */
#define DEBUG_CPU(...) do { if (ENABLE_DEBUG_CPU) { printf(__VA_ARGS__); } } while (0)

/**
 * @brief   CPU debug structure to measure CPU cycles
 *
 * CYCCNT   increments on each clock cycle when the processor is not halted in debug state
 * CPINT    additional cycles required to execute multi-cycle instructions and instruction fetch stalls
 * EXCCNT   cycles spent performing exception entry and exit procedures
 * SLEEPCNT cycles spent sleeping
 * LSUCNT   cycles spent waiting for loads and stores to complete
 * FOLDCNT  cycles saved by instructions which execute in zero cycles
 */
typedef struct cpu_dbg_cyccnt {
    uint32_t cyccnt_diff;      /**< CYCCNT */
    uint8_t cpint_diff;        /**< CPINT */
    uint8_t exccnt_diff;       /**< EXCCNT */
    uint8_t sleepcnt_diff;     /**< SLEEPCNT */
    uint8_t lsucnt_diff;       /**< LSUCNT */
    uint8_t foldcnt_diff;      /**< FOLDCNT */
    unsigned irq_stat;         /**< saved IRQ status */
} cpu_dbg_cyccnt_t;

#if (IS_USED(MODULE_CPU_DEBUG_CYCCNT) && defined(CPU_HAS_DBG_CYCCNT)) || defined(DOXYGEN)
/**
 * @brief   Initializes a CPU cycle debugging structure
 */
#define CPU_DBG_CYCCNT_INIT(...)    (cpu_dbg_cyccnt_t){ 0 };
#else
#define CPU_DBG_CYCCNT_INIT(...)
#endif

#if (IS_USED(MODULE_CPU_DEBUG_CYCCNT) && defined(CPU_HAS_DBG_CYCCNT)) || defined(DOXYGEN)
/**
 * @brief   Defines a CPU cycle debugging structure
 *
 * @param   name    Variable name
 */
#define CPU_DBG_CYCCNT(name) cpu_dbg_cyccnt_t name = CPU_DBG_CYCCNT_INIT();
/**
 * @brief   Defines a static CPU cycle debugging structure
 *
 * @param   name    Variable name
 */
#define CPU_DBG_CYCCNT_STATIC(name) static cpu_dbg_cyccnt_t name = CPU_DBG_CYCCNT_INIT();
#else
#define CPU_DBG_CYCCNT(name)
#define CPU_DBG_CYCCNT_STATIC(name)
#endif

/**
 * @brief   Enables CPU cycle debugging capability
 *
 * @retval  0 on success
 * @retval  -ENOTSUP counting of CPU cycles is not supported
 */
static inline int cpu_dbg_cyccnt_enable(void)
{
#ifdef CPU_HAS_DBG_CYCCNT
    CoreDebug->DEMCR |= (1u << CoreDebug_DEMCR_TRCENA_Pos);
    DWT->LAR = 0xC5ACCE55;
    DWT->CYCCNT = 0;
    DWT->CTRL |= ((1u << DWT_CTRL_CYCCNTENA_Pos) |
                  (1u << DWT_CTRL_CYCEVTENA_Pos) |
                  (1u << DWT_CTRL_CPIEVTENA_Pos) |
                  (1u << DWT_CTRL_EXCEVTENA_Pos) |
                  (1u << DWT_CTRL_SLEEPEVTENA_Pos) |
                  (1u << DWT_CTRL_LSUEVTENA_Pos) |
                  (1u << DWT_CTRL_FOLDEVTENA_Pos));
    return 0;
#else
    return -ENOTSUP;
#endif
}

/**
 * @brief   Disables CPU cycle debugging capability
 */
static inline void cpu_dbg_cyccnt_disable(void)
{
#ifdef CPU_HAS_DBG_CYCCNT
    CoreDebug->DEMCR &= ~(1u << CoreDebug_DEMCR_TRCENA_Pos);
    DWT->CTRL &= ~(1u << DWT_CTRL_CYCCNTENA_Pos);
#endif
}

#if (IS_USED(MODULE_CPU_DEBUG_CYCCNT) && defined(CPU_HAS_DBG_CYCCNT)) || defined(DOXYGEN)
/**
 * @brief   Starts a new iteration to measure CPU cycles
 *
 * @param   cyccnt  Pointer to CPU cycle debugging structure
 *
 * @note    instructions = CYCCNT - CPICNT - EXCCNT - SLEEPCNT - LSUCNT + FOLDCNT
 */
#define CPU_DBG_CYCCNT_START(cyccnt)                                            \
if (ENABLE_DEBUG_CPU) {                                                         \
    (cyccnt)->cyccnt_diff = DWT->CYCCNT;                                        \
    (cyccnt)->cpint_diff = DWT->CPICNT;                                         \
    (cyccnt)->exccnt_diff = DWT->EXCCNT;                                        \
    (cyccnt)->sleepcnt_diff = DWT->SLEEPCNT;                                    \
    (cyccnt)->lsucnt_diff = DWT->LSUCNT;                                        \
    (cyccnt)->foldcnt_diff = DWT->FOLDCNT;                                      \
}
/**
 * @brief   Dame as @ref CPU_DBG_CYCCNT_START() with disabling IRQ
 */
#define CPU_DBG_CYCCNT_START_DISABLE_IRQ(cyccnt)                                \
if (ENABLE_DEBUG_CPU) {                                                         \
        (cyccnt)->irq_stat = irq_disable();                                     \
        CPU_DBG_CYCCNT_START(cyccnt);                                           \
}
#else
#define CPU_DBG_CYCCNT_START(cyccnt)
#define CPU_DBG_CYCCNT_START_DISABLE_IRQ(cyccnt)
#endif

#if (IS_USED(MODULE_CPU_DEBUG_CYCCNT) && defined(CPU_HAS_DBG_CYCCNT)) || defined(DOXYGEN)
/**
 * @brief   Finishes the current CPU cycle count iteration
 *
 * @param   cyccnt  Pointer to CPU cycle debugging structure
 *
 */
#define CPU_DBG_CYCCNT_STOP(cyccnt)                                             \
if (ENABLE_DEBUG_CPU) {                                                         \
    (cyccnt)->cyccnt_diff = DWT->CYCCNT - (cyccnt)->cyccnt_diff;                \
    (cyccnt)->cpint_diff = DWT->CPICNT - (cyccnt)->cpint_diff;                  \
    (cyccnt)->exccnt_diff = DWT->EXCCNT - (cyccnt)->exccnt_diff;                \
    (cyccnt)->sleepcnt_diff = DWT->SLEEPCNT - (cyccnt)->sleepcnt_diff;          \
    (cyccnt)->lsucnt_diff = DWT->LSUCNT - (cyccnt)->lsucnt_diff;                \
    (cyccnt)->foldcnt_diff = DWT->FOLDCNT - (cyccnt)->foldcnt_diff;             \
}
/**
 * @brief   Same as @ref CPU_DBG_CYCCNT_STOP() with restoring IRQ state
 */
#define CPU_DBG_CYCCNT_STOP_RESTORE_IRQ(cyccnt)                                 \
if (ENABLE_DEBUG_CPU) {                                                         \
    CPU_DBG_CYCCNT_STOP(cyccnt)                                                 \
    irq_restore((cyccnt)->irq_stat);                                            \
}
#else
#define CPU_DBG_CYCCNT_STOP(cyccnt)
#define CPU_DBG_CYCCNT_STOP_RESTORE_IRQ(cyccnt)
#endif

#if (IS_USED(MODULE_CPU_DEBUG_CYCCNT) && defined(CPU_HAS_DBG_CYCCNT)) || defined(DOXYGEN)
/**
 * @brief   Prints the elapsed CPU cycles between
 *          @ref CPU_DBG_CYCCNT_START() and @ref CPU_DBG_CYCCNT_STOP()
 *
 * @param   cyccnt  Pointer to CPU cycle debugging structure
 * @param   tag_fmt String tag printed before cycles output
 */
#define CPU_DBG_PRINT_CYCLES(cyccnt, tag_fmt, ...)                                      \
if (ENABLE_DEBUG_CPU) {                                                                 \
    uint32_t diff = (cyccnt)->cyccnt_diff                                               \
                  - (cyccnt)->exccnt_diff                                               \
                  - (cyccnt)->sleepcnt_diff                                             \
                  + (cyccnt)->foldcnt_diff                                              \
                  ;                                                                     \
    DEBUG_CPU(tag_fmt " cycles=%"PRIu32"\n", ##__VA_ARGS__, diff);                      \
}
/**
 * @brief   Prints the number of executed instructions between
 *          @ref CPU_DBG_CYCCNT_START() and @ref CPU_DBG_CYCCNT_STOP()
 *
 * @param   cyccnt  Pointer to CPU cycle debugging structure
 * @param   tag_fmt String tag printed before cycles output
 */
#define CPU_DBG_PRINT_INSTRUCTIONS(cyccnt, tag_fmt, ...)                                \
if (ENABLE_DEBUG_CPU) {                                                                 \
    uint32_t diff = (cyccnt)->cyccnt_diff                                               \
                  - (cyccnt)->cpint_diff                                                \
                  - (cyccnt)->exccnt_diff                                               \
                  - (cyccnt)->sleepcnt_diff                                             \
                  - (cyccnt)->lsucnt_diff                                               \
                  + (cyccnt)->foldcnt_diff                                              \
                  ;                                                                     \
    DEBUG_CPU(tag_fmt " instructions=%"PRIu32"\n", ##__VA_ARGS__, diff);                \
}
#else
#define CPU_DBG_PRINT_CYCLES(cyccnt, tag_fmt, ...)
#define CPU_DBG_PRINT_INSTRUCTIONS(cyccnt, tag_fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* CPU_DEBUG_H */
/** @} */
