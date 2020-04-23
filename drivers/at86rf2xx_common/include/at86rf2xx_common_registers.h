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
 * @file
 * @brief       Register drfinitions of at86rf2xx transceivers
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
#ifndef AT86RF2XX_COMMON_REGISTERS_H
#define AT86RF2XX_COMMON_REGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif

#if IS_USED(MODULE_AT86RF2XX_PERIPH)
#include <avr/io.h>
/**
 * @brief   Perepherie frame buffer start address
 */
#define AT86RF2XX_REG__TRXFBST                                  (&TRXFBST)      /* AT86RFA1, AT86RFR2 */
/**
 * @brief   Perepherie frame buffer end address
 */
#define AT86RF2XX_REG__TRXFBEND                                 (&TRXFBEND)     /* AT86RFA1, AT86RFR2 */
/**
 * @brief   Transceiver pin register
 */
#define AT86RF2XX_REG__TRXPR                                    (&TRXPR)        /* AT86RFA1, AT86RFR2 */
/**
 * @brief   Extended interrupt mask regiser
 */
#define AT86RF2XX_PERIPH_REG__IRQ_MASK1                         (&IRQ_MASK1)    /* AT86RFR2 */
/**
 * @brief   Extended interrupt status register
 */
#define AT86RF2XX_PERIPH_REG__IRQ_STATUS1                       (&IRQ_STATUS1)  /* AT86RFR2 */
/**
 * @brief   Perepherie register offset
 */
#define AT86R2XX_PERIPH_REG_OFF                                 (0x140)
/**
 * @brief Access perepherie transceiver register
 */
#define AT86RF2XX_PERIPH_REG(reg)                               (&(_SFR_MEM8((reg) + AT86RF2XX_PERIPH_REG_OFF)))

/**
 * @name    Bitfield definitions for the TRXPR register
 * @{
 */
#define AT86RF2XX_TRXPR_MASK__ATBE                              (0x08)  /* AT86RFR2 */
#define AT86RF2XX_TRXPR_MASK__TRXTST                            (0x04)  /* AT86RFR2 */
#define AT86RF2XX_TRXPR_MASK__SLPTR                             (0x02)  /* AT86RFA1, AT86RFR2 */
#define AT86RF2XX_TRXPR_MASK__TRXRST                            (0x01)  /* AT86RFA1, AT86RFR2 */
/** @} */

/**
 * @name   Bitfield definitions for the IRQ_MASK1 register
 * @{
 */
#define AT86RF2XX_IRQ_MASK1__TX_START                           (0x01)  /* AT86RFR2 */
#define AT86RF2XX_IRQ_MASK1__MAF_0_AMI                          (0x02)  /* AT86RFR2 */
#define AT86RF2XX_IRQ_MASK1__MAF_1_AMI                          (0x04)  /* AT86RFR2 */
#define AT86RF2XX_IRQ_MASK1__MAF_2_AMI                          (0x08)  /* AT86RFR2 */
#define AT86RF2XX_IRQ_MASK1__MAF_3_AMI                          (0x10)  /* AT86RFR2 */
/** @} */

/**
 * @name   Bitfield definitions for the IRQ_STATUS1 register
 * @{
 */
#define AT86RF2XX_IRQ_STATUS1__TX_START                         (0x01)  /* AT86RFR2 */
#define AT86RF2XX_IRQ_STATUS1__MAF_0_AMI                        (0x02)  /* AT86RFR2 */
#define AT86RF2XX_IRQ_STATUS1__MAF_1_AMI                        (0x04)  /* AT86RFR2 */
#define AT86RF2XX_IRQ_STATUS1__MAF_2_AMI                        (0x08)  /* AT86RFR2 */
#define AT86RF2XX_IRQ_STATUS1__MAF_3_AMI                        (0x10)  /* AT86RFR2 */
/** @} */

#endif

/**
 * @name    Register addresses
 * @{
 */
#define AT86RF2XX_REG__TRX_STATUS                               (0x01)  /* all */
#define AT86RF2XX_REG__TRX_STATE                                (0x02)  /* all */
#define AT86RF2XX_REG__TRX_CTRL_0                               (0x03)  /* all */
#define AT86RF2XX_REG__TRX_CTRL_1                               (0x04)  /* all */
#define AT86RF2XX_REG__PHY_TX_PWR                               (0x05)  /* all */
#define AT86RF2XX_REG__PHY_RSSI                                 (0x06)  /* all */
#define AT86RF2XX_REG__PHY_ED_LEVEL                             (0x07)  /* all */
#define AT86RF2XX_REG__PHY_CC_CCA                               (0x08)  /* all */
#define AT86RF2XX_REG__CCA_THRES                                (0x09)  /* all */
#define AT86RF2XX_REG__RX_CTRL                                  (0x0A)  /* all */
#define AT86RF2XX_REG__SFD_VALUE                                (0x0B)  /* all */
#define AT86RF2XX_REG__TRX_CTRL_2                               (0x0C)  /* all */
#define AT86RF2XX_REG__ANT_DIV                                  (0x0D)  /* all */
#define AT86RF2XX_REG__IRQ_MASK                                 (0x0E)  /* all */
#define AT86RF2XX_REG__IRQ_STATUS                               (0x0F)  /* all */
#define AT86RF2XX_REG__VREG_CTRL                                (0x10)  /* all */
#define AT86RF2XX_REG__BATMON                                   (0x11)  /* all */
#define AT86RF2XX_REG__XOSC_CTRL                                (0x12)  /* all */
#define AT86RF2XX_REG__CC_CTRL_0                                (0x13)  /* AT86RF212B, AT86RF233, AT86RFR2 */
#define AT86RF2XX_REG__CC_CTRL_1                                (0x14)  /* AT86RF212B, AT86RF233, AT86RFR2 */
#define AT86RF2XX_REG__RX_SYN                                   (0x15)  /* all */
#define AT86RF2XX_REG__RF_CTRL_0                                (0x16)  /* AT86RF212B */
#define AT86RF2XX_REG__TRX_RPC                                  (0x16)  /* AT86RF233, AT86RFR2 */
#define AT86RF2XX_REG__XAH_CTRL_1                               (0x17)  /* all */
#define AT86RF2XX_REG__FTN_CTRL                                 (0x18)  /* all */
#define AT86RF2XX_REG__XAH_CTRL_2                               (0x19)  /* AT86RF232, AT86RF233 */
#define AT86RF2XX_REG__PLL_CF                                   (0x1A)  /* all */
#define AT86RF2XX_REG__PLL_DCU                                  (0x1B)  /* all */
#define AT86RF2XX_REG__PART_NUM                                 (0x1C)  /* all */
#define AT86RF2XX_REG__VERSION_NUM                              (0x1D)  /* all */
#define AT86RF2XX_REG__MAN_ID_0                                 (0x1E)  /* all */
#define AT86RF2XX_REG__MAN_ID_1                                 (0x1F)  /* all */
#define AT86RF2XX_REG__SHORT_ADDR_0                             (0x20)  /* all */
#define AT86RF2XX_REG__SHORT_ADDR_1                             (0x21)  /* all */
#define AT86RF2XX_REG__PAN_ID_0                                 (0x22)  /* all */
#define AT86RF2XX_REG__PAN_ID_1                                 (0x23)  /* all */
#define AT86RF2XX_REG__IEEE_ADDR_0                              (0x24)  /* all */
#define AT86RF2XX_REG__IEEE_ADDR_1                              (0x25)  /* all */
#define AT86RF2XX_REG__IEEE_ADDR_2                              (0x26)  /* all */
#define AT86RF2XX_REG__IEEE_ADDR_3                              (0x27)  /* all */
#define AT86RF2XX_REG__IEEE_ADDR_4                              (0x28)  /* all */
#define AT86RF2XX_REG__IEEE_ADDR_5                              (0x29)  /* all */
#define AT86RF2XX_REG__IEEE_ADDR_6                              (0x2A)  /* all */
#define AT86RF2XX_REG__IEEE_ADDR_7                              (0x2B)  /* all */
#define AT86RF2XX_REG__XAH_CTRL_0                               (0x2C)  /* all */
#define AT86RF2XX_REG__CSMA_SEED_0                              (0x2D)  /* all */
#define AT86RF2XX_REG__CSMA_SEED_1                              (0x2E)  /* all */
#define AT86RF2XX_REG__CSMA_BE                                  (0x2F)  /* all */
#define AT86RF2XX_REG__TST_CTRL_DIGI                            (0x36)  /* AT86RF232, AT86RF233, AT86RFA1, AT86RFR2 */
#define AT86RF2XX_REG__TST_AGC                                  (0x3C)  /* AT86RF233 */
#define AT86RF2XX_REG__TST_SDM                                  (0x3D)  /* AT86RF233 */
#define AT86RF2XX_REG__PHY_TX_TIME                              (0x3B)  /* AT86RF233 */
#define AT86RF2XX_REG__PHY_PMU_VALUE                            (0x3B)  /* AT86RF233 */
/** @} */

/**
 * @name    Bitfield definitions for the TRX_STATUS register
 * @{
 */
#define AT86RF2XX_TRX_STATUS_MASK__CCA_DONE                     (0x80)  /* (R), all */
#define AT86RF2XX_TRX_STATUS_MASK__CCA_STATUS                   (0x40)  /* (R), all */
#define AT86RF2XX_TRX_STATUS_MASK__TST_STATUS                   (0x20)  /* AT86RFA1, AT86RFR2 */
#define AT86RF2XX_TRX_STATUS_MASK__TRX_STATUS                   (0x1F)  /* (R), all */

#define AT86RF2XX_CCA_DONE__NO                                  (0x00) /* Default */
#define AT86RF2XX_CCA_DONE__YES                                 (0x80)

#define AT86RF2XX_CCA_STATUS__BUSY                              (0x00) /* Default */
#define AT86RF2XX_CCA_STATUS__CLEAR                             (0x40)

#define AT86RF2XX_TRX_STATUS__P_ON                              (0x00)
#define AT86RF2XX_TRX_STATUS__BUSY_RX                           (0x01)
#define AT86RF2XX_TRX_STATUS__BUSY_TX                           (0x02)
#define AT86RF2XX_TRX_STATUS__RX_ON                             (0x06)
#define AT86RF2XX_TRX_STATUS__TRX_OFF                           (0x08)
#define AT86RF2XX_TRX_STATUS__PLL_ON                            (0x09)
#define AT86RF2XX_TRX_STATUS__SLEEP                             (0x0F)
#define AT86RF2XX_TRX_STATUS__BUSY_RX_AACK                      (0x11)
#define AT86RF2XX_TRX_STATUS__BUSY_TX_ARET                      (0x12)
#define AT86RF2XX_TRX_STATUS__RX_AACK_ON                        (0x16)
#define AT86RF2XX_TRX_STATUS__TX_ARET_ON                        (0x19)
#define AT86RF2XX_TRX_STATUS__RX_ON_NOCLK                       (0x1C)
#define AT86RF2XX_TRX_STATUS__RX_AACK_ON_NOCLK                  (0x1D)
#define AT86RF2XX_TRX_STATUS__BUSY_RX_AACK_NOCLK                (0x1E)
#define AT86RF2XX_TRX_STATUS__STATE_TRANSITION_IN_PROGRESS      (0x1F)
/** @} */

/**
 * @name    Bitfield definitions for the TRX_STATE register
 * @{
 */
#define AT86RF2XX_TRX_STATE_MASK__TRAC_STATUS                   (0xE0)  /* (R), all */
#define AT86RF2XX_TRX_STATE_MASK__TRX_CMD                       (0x1F)  /* (RW), all */

#define AT86RF2XX_TRAC_STATUS__TRAC_SUCCESS                     (0x00)
#define AT86RF2XX_TRAC_STATUS__TRAC_SUCCESS_DATA_PENDING        (0x20)
#define AT86RF2XX_TRAC_STATUS__TRAC_SUCCESS_WAIT_FOR_ACK        (0x40)
#define AT86RF2XX_TRAC_STATUS__TRAC_CHANNEL_ACCESS_FAILURE      (0x60)
#define AT86RF2XX_TRAC_STATUS__TRAC_NO_ACK                      (0xA0)
#define AT86RF2XX_TRAC_STATUS__TRAC_INVALID                     (0xE0)

#define AT86RF2XX_TRX_CMD__NOP                                  (0x00)  /* Default */
#define AT86RF2XX_TRX_CMD__TX_START                             (0x02)
#define AT86RF2XX_TRX_CMD__FORCE_TRX_OFF                        (0x03)
#define AT86RF2XX_TRX_CMD__FORCE_PLL_ON                         (0x04)
#define AT86RF2XX_TRX_CMD__RX_ON                                (0x06)
#define AT86RF2XX_TRX_CMD__TRX_OFF                              (0x08)
#define AT86RF2XX_TRX_CMD__PLL_ON                               (0x09)
#define AT86RF2XX_TRX_CMD__PREP_DEEP_SLEEP                      (0x10)
#define AT86RF2XX_TRX_CMD__RX_AACK_ON                           (0x16)
#define AT86RF2XX_TRX_CMD__TX_ARET_ON                           (0x19)
/** @} */

/**
 * @name    Bitfield definitions for the TRX_CTRL_0 register
 * @{
 */
#define AT86RF2XX_TRX_CTRL_0_MASK__PAD_IO                       (0xC0)  /* AT86RF212B, AT86RF231, AT86RF232 */
#define AT86RF2XX_TRX_CTRL_0_MASK__PAD_IO_CLKM                  (0x30)  /* AT86RF212B, AT86RF231, AT86RF232 */
#define AT86RF2XX_TRX_CTRL_0_MASK__TOM_EN                       (0x80)  /* AT86RF233 */
#if IS_USED(MODULE_AT86RF233)
#define AT86RF233_TRX_CTRL_0_MASK__PMU_EN                       (0x20)  /* AT86RF233 [5, 5] */
#endif
#if IS_USED(MODULE_AT86RFR2)
#define AT86RFR2_TRX_CTRL_0_MASK__PMU_EM                        (0x40)  /* AT86RFR2 [6, 6] */
#endif
#define AT86RF2XX_TRX_CTRL_0_MASK__PMU_START                    (0x20)  /* AT86RFR2 */
#define AT86RF2XX_TRX_CTRL_0_MASK__PMU_IF_INVERSE               (0x10)  /* AT86RF233, AT86RFR2 */
#define AT86RF2XX_TRX_CTRL_0_MASK__CLKM_SHA_SEL                 (0x08)  /* AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */
#define AT86RF2XX_TRX_CTRL_0_MASK__CLKM_CTRL                    (0x07)  /* AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */

#define AT86RF2XX_TRX_CTRL_0_DEFAULT__PAD_IO                    (0x00)
#define AT86RF2XX_TRX_CTRL_0_DEFAULT__PAD_IO_CLKM               (0x10)

#define AT86RF2XX_TOM_EN__YES                                   (0x80)
#define AT86RF2XX_TOM_EN__NO                                    (0x00) /* Default */

#define AT86RF2XX_PMU_EN__YES                                   (0x20)
#define AT86RF2XX_PMU_EN__NO                                    (0x00) /* Default */

#define AT86RF2XX_PMU_IF_INVERSE__YES                           (0x10)
#define AT86RF2XX_PMU_IF_INVERSE__NO                            (0x00) /* Default */

#define AT86RF2XX_CLKM_SHA_SEL__AFTER_SLEEP                     (0x08) /* Default */
#define AT86RF2XX_CLKM_SHA_SEL__IMMEDIATELY                     (0x00)

#define AT86RF2XX_CLKM_CTRL__OFF                                (0x00)
#define AT86RF2XX_CLKM_CTRL__1MHz                               (0x01) /* Default */
#define AT86RF2XX_CLKM_CTRL__2MHz                               (0x02)
#define AT86RF2XX_CLKM_CTRL__4MHz                               (0x03)
#define AT86RF2XX_CLKM_CTRL__8MHz                               (0x04)
#define AT86RF2XX_CLKM_CTRL__16MHz                              (0x05)
#define AT86RF2XX_CLKM_CTRL__250kHz                             (0x06)
#define AT86RF2XX_CLKM_CTRL__62_5kHz                            (0x07)
/** @} */

/**
 * @name    Bitfield definitions for the TRX_CTRL_1 register
 * @{
 */
#define AT86RF2XX_TRX_CTRL_1_MASK__PA_EXT_EN                    (0x80)  /* (RW), AT86RF212B, AT86RF231, AT86RF233, AT86RFA1, AT86RFR2 */
#define AT86RF2XX_TRX_CTRL_1_MASK__IRQ_2_EXT_EN                 (0x40)  /* (RW), all */
#define AT86RF2XX_TRX_CTRL_1_MASK__TX_AUTO_CRC_ON               (0x20)  /* (RW), all */
#define AT86RF2XX_TRX_CTRL_1_MASK__RX_BL_CTRL                   (0x10)  /* (RW), AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */
#define AT86RF2XX_TRX_CTRL_1_MASK__PLL_TX_FLT                   (0x10)  /* (RW), AT86RFR2 */
#define AT86RF2XX_TRX_CTRL_1_MASK__SPI_CMD_MODE                 (0x0C)  /* (RW), AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */
#define AT86RF2XX_TRX_CTRL_1_MASK__IRQ_MASK_MODE                (0x02)  /* (RW), AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */
#define AT86RF2XX_TRX_CTRL_1_MASK__IRQ_POLARITY                 (0x01)  /* (RW), AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */

#define AT86RF2XX_PA_EXT_EN__NO                                 (0x00) /* Default */
#define AT86RF2XX_PA_EXT_EN__YES                                (0x80)

#define AT86RF2XX_IRQ_2_EXT_EN__NO                              (0x00)  /* Default */
#define AT86RF2XX_IRQ_2_EXT_EN__YES                             (0x40)

#define AT86RF2XX_TX_AUTO_CRC_ON__NO                            (0x00)
#define AT86RF2XX_TX_AUTO_CRC_ON__YES                           (0x20)  /* Default */

#define AT86RF2XX_RX_BL_CTRL__DIS                               (0x00) /* Default */
#define AT86RF2XX_RX_BL_CTRL__EN                                (0x10)

#define AT86RF2XX_SPI_CMD_MODE__ZERO                            (0x00) /* Default */
#define AT86RF2XX_SPI_CMD_MODE__TRX_STATUS                      (0x04)
#define AT86RF2XX_SPI_CMD_MODE__PHY_RSSI                        (0x08)
#define AT86RF2XX_SPI_CMD_MODE__IRQ_STATUS                      (0x0C)

#define AT86RF2XX_IRQ_MASK_MODE__DIS                            (0x00)
#define AT86RF2XX_IRQ_MASK_MODE__EN                             (0x02)  /* Default */

#define AT86RF2XX_IRQ_POLARITY__AH                              (0x00)  /* Default */
#define AT86RF2XX_IRQ_POLARITY__AL                              (0x01)
/** @} */

/**
 * @name    Bitfield definitions for the PHY_TX_PWR register
 * @{
 */
#define AT86RF2XX_PHY_TX_PWR_MASK__PA_BOOST                     (0x80)  /* AT86RF212B */
#define AT86RF2XX_PHY_TX_PWR_MASK__GC_PA                        (0x60)  /* AT86RF212B */
#define AT86RF2XX_PHY_TX_PWR_MASK__PA_BUF_LT                    (0xC0)  /* AT86RF231, AT86RFA1 */
#define AT86RF2XX_PHY_TX_PWR_MASK__PA_LT                        (0x30)  /* AT86RF231, AT86RFA1 */
#if IS_USED(MODULE_AT86RF212B)
    #define AT86RF212B_PHY_TX_PWR_MASK__TX_PWR                  (0x1F)  /* AT86RF212B [4, 0] */
#endif
#if IS_USED(MODULE_AT86RF231)
    #define AT86RF231_PHY_TX_PWR_MASK__TX_PWR                   (0x0F) /* AT86RF231 [3, 0] */
#endif
#if IS_USED(MODULE_AT86RF232)
    #define AT86RF232_PHY_TX_PWR_MASK__TX_PWR                   (0x0F) /* AT86RF232 [3, 0] */
#endif
#if IS_USED(MODULE_AT86RF233)
    #define AT86RF233_PHY_TX_PWR_MASK__TX_PWR                   (0x0F) /* (RW), AT86RF233 [3, 0] */
#endif
#if IS_USED(MODULE_AT86RFA1)
    #define AT86RFA1_PHY_TX_PWR_MASK__TX_PWR                    (0x0F) /* AT86RFA1 [3, 0] */
#endif
#if IS_USED(MODULE_AT86RFR2)
    #define AT86RFR2_PHY_TX_PWR_MASK__TX_PWR                    (0x0F) /* AT86RFR2 [3, 0] */
#endif

#define AT86RF2XX_PHY_TX_PWR_DEFAULT__PA_BUF_LT                 (0xC0)
#define AT86RF2XX_PHY_TX_PWR_DEFAULT__PA_LT                     (0x00)
#define AT86RF2XX_PHY_TX_PWR_DEFAULT__TX_PWR                    (0x00)
/** @} */

/**
 * @name    Bitfield definitions for the PHY_RSSI register
 * @{
 */
#define AT86RF2XX_PHY_RSSI_MASK__RX_CRC_VALID                   (0x80)  /* (R), all */
#define AT86RF2XX_PHY_RSSI_MASK__RND_VALUE                      (0x60)  /* (R), all */
#define AT86RF2XX_PHY_RSSI_MASK__RSSI                           (0x1F)  /* (R), all */

#define AT86RF2XX_RX_CRC_VALID__NO                              (0x00) /* Default */
#define AT86RF2XX_RX_CRC_VALID__YES                             (0x80)
/** @} */

/**
 * @name    Bitfield definitions for the PHY_ED_LEVEL register
 * @{
 */
#define AT86RF2XX_PHY_ED_LEVEL_MASK__PHY_ED_LEVEL               (0xFF) /* (R), all */
/** @} */

/**
 * @name    Bitfield definitions for the PHY_CC_CCA register
 * @{
 */
#define AT86RF2XX_PHY_CC_CCA_MASK__CCA_REQUEST                  (0x80) /* (RW), all */
#define AT86RF2XX_PHY_CC_CCA_MASK__CCA_MODE                     (0x60) /* (RW), all */
#define AT86RF2XX_PHY_CC_CCA_MASK__CHANNEL                      (0x1F) /* (RW), all */

#define AT86RF2XX_CCA_REQUEST__NO                               (0x00) /* Default */
#define AT86RF2XX_CCA_REQUEST__YES                              (0x80)

#define AT86RF2XX_CCA_MODE__CARRIER_OR_THRESHOLD                (0x00)
#define AT86RF2XX_CCA_MODE__THRESHOLD                           (0x20) /* Default */
#define AT86RF2XX_CCA_MODE__CARRIER                             (0x40)
#define AT86RF2XX_CCA_MODE__CARRIER_AND_THRESHOLD               (0x60)
/** @} */

/**
 * @name    Bitfield definitions for the CCA_THRES register
 * @{
 */
#define AT86RF2XX_CCA_THRES_MASK__CCA_CS_THRES                  (0xF0)  /* AT86RF212B, AT86RF231, AT86RFA1, AT86RFR2 */
#define AT86RF2XX_CCA_THRES_MASK__CCA_ED_THRES                  (0x0F)  /* (RW), all */
/** @} */

/**
 * @name    Bitfield definitions for the RX_CTRL register
 * @{
 */
#define AT86RF2XX_RX_CTRL_MASK__PEL_SHIFT_VALUE                 (0xC0)  /* (R), AT86RF233 */
#define AT86RF2XX_RX_CTRL_MASK__JCM_EN                          (0x20)  /* AT86RF212B */
#define AT86RF2XX_RX_CTRL_MASK__PDT_THRES                       (0x0F)  /* (RW), AT86RF231, AT86RF232, AT86RF233, AT86RFA1, AT86RFR2 */

#define AT86RF2XX_PEL_SHIFT_VALUE__NORMAL                       (0x00) /* Default */
#define AT86RF2XX_PEL_SHIFT_VALUE__EARLY                        (0x40)
#define AT86RF2XX_PEL_SHIFT_VALUE__LATE                         (0x80)

#define AT86RF2XX_PDT_THRES__ANTENNA_DIVERSITY                  (0x03)
#define AT86RF2XX_PDT_THRES__DEFAULT                            (0x07) /* Default */
/** @} */

/**
 * @name    Bitfield definitions for the SFD_VALUE register
 * @{
 */
#define AT86RF2XX_SFD_VALUE_MASK__SFD_VALUE                     (0xFF) /* (RW), AT86RF212B, AT86RF231, AT86RF233, AT86RFA1, AT86RFR2 */

#define AT86RF2XX_SFD_VALUE__IEEE802154                         (0xA7) /* Default */
/** @} */

/**
 * @name    Bitfield definitions for the TRX_CTRL_2 register
 * @{
 */
#define AT86RF2XX_TRX_CTRL_2_MASK__RX_SAFE_MODE                 (0x80)  /* (RW), all */
#define AT86RF2XX_TRX_CTRL_2_MASK__FREQ_MODE                    (0x3F)  /* ? */
#define AT86RF2XX_TRX_CTRL_2_MASK__TRX_OFF_AVDD_EN              (0x40)  /* AT86RF212B */
#define AT86RF2XX_TRX_CTRL_2_MASK__OQPSK_SCRAM_EN               (0x20)  /* (RW), AT86RF212B, AT86RF233 */
#define AT86RF2XX_TRX_CTRL_2_MASK__ALT_SPECTRUM                 (0x10)  /* AT86RF212B */
#define AT86RF2XX_TRX_CTRL_2_MASK__BPSK_OQPSK                   (0x08)  /* AT86RF212B */
#define AT86RF2XX_TRX_CTRL_2_MASK__SUB_MODE                     (0x04)  /* AT86RF212B */
#if IS_USED(MODULE_AT86RF212B)
#define AT86RF212B_TRX_CTRL_2_MASK__OQPSK_DATA_RATE             (0x03)  /* AT86RF212B [1, 0] */
#endif
#if IS_USED(MODULE_AT86RF231)
#define AT86RF231_TRX_CTRL_2_MASK__OQPSK_DATA_RATE              (0x03) /* AT86RF231 [1, 0] */
#endif
#if IS_USED(MODULE_AT86RF233)
#define AT86RF233_TRX_CTRL_2_MASK__OQPSK_DATA_RATE              (0x07) /* (RW), AT86RF233 [2, 0] */

#define AT86RF233_OQPSK_DATA_RATE__250KBPS                      (0x00) /* Default */
#define AT86RF233_OQPSK_DATA_RATE__500KBPS                      (0x01)
#define AT86RF233_OQPSK_DATA_RATE__1000KBPS                     (0x02)
#define AT86RF233_OQPSK_DATA_RATE__2000KBPS                     (0x03)
#endif
#if IS_USED(MODULE_AT86RFA1)
#define AT86RFA1_TRX_CTRL_2_MASK__OQPSK_DATA_RATE               (0x03) /* AT86RF2A1 [1, 0] */
#endif
#if IS_USED(MODULE_AT86RFR2)
#define AT86RFR2_TRX_CTRL_2_MASK__OQPSK_DATA_RATE               (0x03) /* AT86RFR2 [1, 0] */
#endif

#define AT86RF2XX_RX_SAVE_MODE__EN                              (0x80)
#define AT86RF2XX_RX_SAVE_MODE__DIS                             (0x00)

#define AT86RF2XX_OQPSK_SCRAM_EN__NO                            (0x00)
#define AT86RF2XX_OQPSK_SCRAM_EN__YES                           (0x20) /* Default */


/** @} */

/**
 * @name    Bitfield definitions for the ANT_DIV register
 * @{
 */
#define AT86RF2XX_ANT_DIV_MASK__ANT_SEL                         (0x80)  /* (R), all */
#define AT86RF2XX_ANT_DIV_MASK__ANT_DIV_EN                      (0x08)  /* (RW), all */
#define AT86RF2XX_ANT_DIV_MASK__ANT_EXT_SW_EN                   (0x04)  /* (RW), all */
#define AT86RF2XX_ANT_DIV_MASK__ANT_CTRL                        (0x03)  /* (RW), all */

#define AT86RF2XX_ANT_SEL__ANT0                                 (0x00)
#define AT86RF2XX_ANT_SEL__ANT1                                 (0x80) /* Default */

#define AT86RF2XX_ANT_DIV_EN__NO                                (0x00) /* Default */
#define AT86RF2XX_ANT_DIV_EN__YES                               (0x08)

#define AT86RF2XX_ANT_EXT_SW_EN__NO                             (0x00) /* Default */
#define AT86RF2XX_ANT_EXT_SW_EN__YES                            (0x04)

#define AT86RF2XX_ANT_CTRL__AUTO                                (0x00) /* Default */
#define AT86RF2XX_ANT_CTRL__ANT0                                (0x01)
#define AT86RF2XX_ANT_CTRL__ANT1                                (0x02)

/** @} */

/**
 * @name    Bitfield definitions for the IRQ_MASK register
 * @{
 */
#define AT86RF2XX_IRQ_MASK_MASK__BAT_LOW                        (0x80)  /* (RW), AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */
#define AT86RF2XX_IRQ_MASK_MASK__AWAKE                          (0x80)  /* (RW), AT86RFA1, AT86RFR2 */
#define AT86RF2XX_IRQ_MASK_MASK__TRX_UR                         (0x40)  /* (RW), AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */
#define AT86RF2XX_IRQ_MASK_MASK__TX_END                         (0x40)  /* (RW), AT86RFA1, AT86RFR2 */
#define AT86RF2XX_IRQ_MASK_MASK__AMI                            (0x20)  /* (RW), all */
#define AT86RF2XX_IRQ_MASK_MASK__CCA_ED_DONE                    (0x10)  /* (RW), all */
#define AT86RF2XX_IRQ_MASK_MASK__TRX_END                        (0x08)  /* (RW), AT86RF212B, AT86RF231, AT86RF232, AT86RF231 */
#define AT86RF2XX_IRQ_MASK_MASK__RX_END                         (0x08)  /* (RW), AT86RFA1, AT86RFR2 */
#define AT86RF2XX_IRQ_MASK_MASK__RX_START                       (0x04)  /* (RW), all */
#define AT86RF2XX_IRQ_MASK_MASK__PLL_UNLOCK                     (0x02)  /* (RW), all */
#define AT86RF2XX_IRQ_MASK_MASK__PLL_LOCK                       (0x01)  /* (RW), all */

#define AT86RF2XX_BAT_LOW__DIS                                  (0x00) /* Default */
#define AT86RF2XX_BAT_LOW__EN                                   (0x80)

#define AT86RF2XX_TRX_UR__DIS                                   (0x00) /* Default */
#define AT86RF2XX_TRX_UR__EN                                    (0x40)

#define AT86RF2XX_AMI__DIS                                      (0x00) /* Default */
#define AT86RF2XX_AMI__EN                                       (0x20)

#define AT86RF2XX_CCA_ED_DONE__DIS                              (0x00) /* Default */
#define AT86RF2XX_CCA_ED_DONE__EN                               (0x10)

#define AT86RF2XX_TRX_END__DIS                                  (0x00) /* Default */
#define AT86RF2XX_TRX_END__EN                                   (0x08)

#define AT86RF2XX_RX_START__DIS                                 (0x00) /* Default */
#define AT86RF2XX_RX_START__EN                                  (0x04)

#define AT86RF2XX_PLL_UNLOCK__DIS                               (0x00) /* Default */
#define AT86RF2XX_PLL_UNLOCK__EN                                (0x02)

#define AT86RF2XX_PLL_LOCK__DIS                                 (0x00) /* Default */
#define AT86RF2XX_PLL_LOCK__EN                                  (0x01)
/** @} */

/**
 * @name    Bitfield definitions for the IRQ_STATUS register
 * @{
 */
#define AT86RF2XX_IRQ_STATUS_MASK__BAT_LOW                      (0x80)  /* (R), AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */
#define AT86RF2XX_IRQ_STATUS_MASK__AWAKE                        (0x80)  /* AT86RFA1, AT86RFR2 */
#define AT86RF2XX_IRQ_STATUS_MASK__TRX_UR                       (0x40)  /* (R), AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */
#define AT86RF2XX_IRQ_STATUS_MASK__TX_END                       (0x40)  /* AT86RFA1, AT86RFR2 */
#define AT86RF2XX_IRQ_STATUS_MASK__AMI                          (0x20)  /* (R), all */
#define AT86RF2XX_IRQ_STATUS_MASK__CCA_ED_DONE                  (0x10)  /* (R), all */
#define AT86RF2XX_IRQ_STATUS_MASK__TRX_END                      (0x08)  /* AT86RF212B, AT86RF231, AT86RF232, AT86RF231 */
#define AT86RF2XX_IRQ_STATUS_MASK__RX_END                       (0x08)  /* AT86RFA1, AT86RFR2 */
#define AT86RF2XX_IRQ_STATUS_MASK__RX_START                     (0x04)  /* (R), all */
#define AT86RF2XX_IRQ_STATUS_MASK__PLL_UNLOCK                   (0x02)  /* (R), all */
#define AT86RF2XX_IRQ_STATUS_MASK__PLL_LOCK                     (0x01)  /* (R), all */

#define AT86RF2XX_BAT_LOW__LOW                                  (0x00) /* Default */
#define AT86RF2XX_BAT_LOW__HIGH                                 (0x80)

#define AT86RF2XX_TRX_UR__LOW                                   (0x00) /* Default */
#define AT86RF2XX_TRX_UR__HIGH                                  (0x40)

#define AT86RF2XX_AMI__LOW                                      (0x00) /* Default */
#define AT86RF2XX_AMI__HIGH                                     (0x20)

#define AT86RF2XX_CCA_ED_DONE__LOW                              (0x00) /* Default */
#define AT86RF2XX_CCA_ED_DONE__HIGH                             (0x10)

#define AT86RF2XX_TRX_END__LOW                                  (0x00) /* Default */
#define AT86RF2XX_TRX_END__HIGH                                 (0x08)

#define AT86RF2XX_RX_START__LOW                                 (0x00) /* Default */
#define AT86RF2XX_RX_START__HIGH                                (0x04)

#define AT86RF2XX_PLL_UNLOCK__LOW                               (0x00) /* Default */
#define AT86RF2XX_PLL_UNLOCK__HIGH                              (0x02)

#define AT86RF2XX_PLL_LOCK__LOW                                 (0x00) /* Default */
#define AT86RF2XX_PLL_LOCK__HIGH                                (0x01)
/** @} */

/**
 * @name    Bitfield definitions for the VREG_CTRL register
 * @{
 */
#define AT86RF2XX_VREG_CTRL_MASK__AVREG_EXT                     (0x80) /* (RW), all */
#define AT86RF2XX_VREG_CTRL_MASK__AVDD_OK                       (0x40) /* (R), all */
#define AT86RF2XX_VREG_CTRL_MASK__DVREG_EXT                     (0x08) /* (RW), AT86RF212B, AT86RF231, AT86RF232, AT86RF233 */
#define AT86RF2XX_VREG_CTRL_MASK__DVDD_OK                       (0x04) /* (R), all */

#define AT86RF2XX_AVREG_EXT__EN                                 (0x00) /* Default */
#define AT86RF2XX_AVREG_EXT__DIS                                (0x80)

#define AT86RF2XX_AVDD_OK__NO                                   (0x00) /* Default */
#define AT86RF2XX_AVDD_OK__YES                                  (0x40)

#define AT86RF2XX_DVREG_EXT__EN                                 (0x00) /* Default */
#define AT86RF2XX_DVREG_EXT__DIS                                (0x08)

#define AT86RF2XX_DVDD_OK__NO                                   (0x00) /* Default */
#define AT86RF2XX_DVDD_OK__YES                                  (0x04)
/** @} */

/**
 * @name    Bitfield definitions for the BATMON register
 * @{
 */
#define AT86RF2XX_BATMON_MASK__PLL_LOCK_CP                      (0x80)  /* AT86RF212B */
#define AT86RF2XX_BATMON_MASK__BAT_LOW                          (0x80)  /* AT86RFA1, AT86RFR2 */
#define AT86RF2XX_BATMON_MASK__BAT_LOW_EN                       (0x40)  /* AT86RFA1, AT86RFR2 */
#define AT86RF2XX_BATMON_MASK__BATMON_OK                        (0x20)  /* (R), all */
#define AT86RF2XX_BATMON_MASK__BATMON_HR                        (0x10)  /* (RW), all */
#define AT86RF2XX_BATMON_MASK__BATMON_VTH                       (0x0F)  /* (RW), all */

#define AT86RF2XX_BATMON_OK__NO                                 (0x00)  /* Default */
#define AT86RF2XX_BATMON_OK__YES                                (0x20)

#define AT86RF2XX_BATMON_HR__NO                                 (0x00)  /* Default */
#define AT86RF2XX_BATMON_HR__YES                                (0x10)
/** @} */

/**
 * @name    Bitfield definitions for the XOSC_CTRL register
 * @{
 */
#define AT86RF2XX_XOSC_CTRL_MASK__XTAL_MODE                     (0xF0)  /* (RW), all */
#define AT86RF2XX_XOSC_CTRL_MASK__XTAL_TRIM                     (0x0F)  /* (RW), all */

#if IS_USED(MODULE_AT86RF212B)
#define AT86RF212B_XTAL_MODE__EXTERNAL                          (0x40)
#endif
#if IS_USED(MODULE_AT86RF231)
#define AT86RF231_XTAL_MODE__EXTERNAL                           (0x40)
#endif
#if IS_USED(MODULE_AT86RF232)
#define AT86RF232_XTAL_MODE__EXTERNAL                           (0x50)
#endif
#if IS_USED(MODULE_AT86RF233)
#define AT86RF233_XTAL_MODE__EXTERNAL                           (0x50)
#endif
#if IS_USED(MODULE_AT86RFA1)
#define AT86RFA1_XTAL_MODE__EXTERNAL                            (0x50)
#endif
#if IS_USED(MODULE_AT86RFR2)
#define AT86RFR2_XTAL_MODE__EXTERNAL                            (0x50)
#endif

#define AT86RF2XX_XTAL_MODE__INTERNAL                           (0xF0)  /* Default */

#define AT86RF2XX_XTAL_TRIM__DEFAULT                            (0x00)  /* Deafult */
/** @} */

/**
 * @name    Bitfield definitions for the CC_CTRL_0 register
 * @{
 */
#define AT86RF2XX_CC_CTRL_0_MASK__CC_NUMBER                     (0xFF)  /* (RW), AT86RF212B, AT86RF233, AT86RFR2 */

#define AT86RF2XX_CC_NUMBER__DEFAULT                            (0x00)  /* Default */
/** @} */

/**
 * @name    Bitfield definitions for the CC_CTRL_1 register
 * @{
 */
#if IS_USED(MODULE_AT86RF212B)
#define AT86RF212B_CC_CTRL_1_MASK__CC_BAND                      (0x07)  /* AT86RF212B [2, 0] */
#endif
#if IS_USED(MODULE_AT86RF233)
#define AT86RF233_CC_CTRL_1_MASK__CC_BAND                       (0x0F)  /* AT86RF233 [3, 0] */

#define AT86RF233_CC_BAND__UNUSED                               (0x00)  /* Default */
#define AT86RF233_CC_BAND__0X8                                  (0x08)
#define AT86RF233_CC_BAND__0X9                                  (0x09)
#endif
#if IS_USED(MODULE_AT86RFR2)
#define AT86RFR2_CC_CTRL_1_MASK__CC_BAND                        (0x0F) /* AT86RFR2 [3, 0] */
#endif
/** @} */

/**
 * @name    Bitfield definitions for the RX_SYN register
 * @{
 */
#define AT86RF2XX_RX_SYN_MASK__RX_PDT_DIS                       (0x80)  /* (RW), all */
#define AT86RF2XX_RX_SYN_MASK__RX_OVERRIDE                      (0x70)  /* AT86RF212B */
#define AT86RF2XX_RX_SYN_MASK__RX_PDT_LEVEL                     (0x0F)  /* (RW), all */

#define AT86RF2XX_RX_PDT_DIS__NO                                (0x00)  /* Default */
#define AT86RF2XX_RX_PDT_DIS__YES                               (0x80)
/** @} */

/**
 * @name    Bitfield definitions for the RF_CTRL_0 register
 * @{
 */
#define AT86RF2XX_RF_CTRL_0_MASK__PA_LT                         (0xC0)  /* AT86RF212B */
#define AT86RF2XX_RF_CTRL_0_MASK__IF_SHIFT_MODE                 (0x0C)  /* AT86RF212B */
#define AT86RF2XX_RF_CTRL_0_MASK__GC_TX_OFFS                    (0x03)  /* AT86RF212B */

#define AT86RF2XX_RF_CTRL_0_GC_TX_OFFS__0DB                     (0x01)
#define AT86RF2XX_RF_CTRL_0_GC_TX_OFFS__1DB                     (0x02)
#define AT86RF2XX_RF_CTRL_0_GC_TX_OFFS__2DB                     (0x03)
/** @} */

/**
 * @name    Bitfield definitions for the TRX_RPC register
 * @{
 */
#define AT86RF2XX_TRX_RPC_MASK__RX_RPC_CTRL                     (0xC0)  /* (RW), AT86RF233, AT86RFR2 */
#define AT86RF2XX_TRX_RPC_MASK__RX_RPC_EN                       (0x20)  /* (RW), AT86RF233, AT86RFR2 */
#define AT86RF2XX_TRX_RPC_MASK__PDT_RPC_EN                      (0x10)  /* (RW), AT86RF233, AT86RFR2 */
#define AT86RF2XX_TRX_RPC_MASK__PLL_RPC_EN                      (0x08)  /* (RW), AT86RF233, AT86RFR2 */
#define AT86RF2XX_TRX_RPC_MASK__XAH_TX_RPC_EN                   (0x04)  /* (RW), AT86RF233 */
#define AT86RF2XX_TRX_RPC_MASK__IPAN_RPC_EN                     (0x02)  /* (RW), AT86RF233, AT86RFR2 */
#define AT86RF2XX_TRX_RPC_MASK__XAH_RPC_EN                      (0x01)  /* AT86RFR2 */

#define AT86RF2XX_RX_RPC_CTRL__MAX_POWER_SAVING                 (0xC0) /* Default */
#define AT86RF2XX_RX_RPC_CTRL__MIN_POWER_SAVING                 (0x00)

#define AT86RF2XX_RX_RPC_EN__NO                                 (0x00) /* Default */
#define AT86RF2XX_RX_RPC_EN__YES                                (0x20)

#define AT86RF2XX_PDT_RPC_EN__NO                                (0x00)  /* Default */
#define AT86RF2XX_PDT_RPC_EN__YES                               (0x10)

#define AT86RF2XX_PLL_RPC_EN__NO                                (0x00)  /* Default */
#define AT86RF2XX_PLL_RPC_EN__YES                               (0x08)

#define AT86RF2XX_XAH_TX_RPC_EN__NO                             (0x00) /* Default */
#define AT86RF2XX_XAH_TX_RPC_EN__YES                            (0x04)

#define AT86RF2XX_IPAN_RPC_EN__NO                               (0x00) /* Default */
#define AT86RF2XX_IPAN_RPC_EN__YES                              (0x02)
/** @} */

/**
 * @name    Bitfield definitions for the XAH_CTRL_1 register
 * @{
 */
#define AT86RF2XX_XAH_CTRL_1_MASK__ARET_TX_TS_EN                (0x80)  /* (RW), AT86RF232, AT86RF233 */
#define AT86RF2XX_XAH_CTRL_1_MASK__CSMA_LBT_MODE                (0x40)  /* AT86RF212B */
#define AT86RF2XX_XAH_CTRL_1_MASK__AACK_FLTR_RES_FT             (0x20)  /* (RW), all */
#define AT86RF2XX_XAH_CTRL_1_MASK__AACK_UPLD_RES_FT             (0x10)  /* (RW), all */
#define AT86RF2XX_XAH_CTRL_1_MASK__AACK_ACK_TIME                (0x04)  /* (RW), all */
#define AT86RF2XX_XAH_CTRL_1_MASK__AACK_PROM_MODE               (0x02)  /* (RW), all */
#define AT86RF2XX_XAH_CTRL_1_MASK__AACK_SPC_EN                  (0x01)  /* (RW), AT86RF233 */

#define AT86RF2XX_ARET_TX_TS_EN__NO                             (0x00)  /* Default */
#define AT86RF2XX_ARET_TX_TS_EN__YES                            (0x80)

#define AT86RF2XX_AACK_FLTR_RES_FT__NO                          (0x00)  /* Default */
#define AT86RF2XX_AACK_FLTR_RES_FT__YES                         (0x20)

#define AT86RF2XX_AACK_UPLD_RES_FT__NO                          (0x00) /* Default */
#define AT86RF2XX_AACK_UPLD_RES_FT__YES                         (0x10)

#define AT86RF2XX_AACK_ACK_TIME__12                             (0x00) /* Default */
#define AT86RF2XX_AACK_ACK_TIME__2                              (0x04)

#define AT86RF2XX_AACK_PROM_MODE__DIS                           (0x00) /* Default */
#define AT86RF2XX_AACK_PROM_MODE__EN                            (0x02)

#define AT86RF2XX_AACK_SPC_EN__NO                               (0x00) /* Default */
#define AT86RF2XX_AACK_SPC_EN__YES                              (0x01)
/** @} */

/**
 * @name    Bitfield definitions for the FTN_CTRL register
 * @{
 */
#define AT86RF2XX_FTN_CTRL_MASK__FTN_START                      (0x80)  /* all */
#define AT86RF2XX_FTN_CTRL_MASK__FTN_FTNV                       (0x3F)  /* AT86RF233 */
/** @} */

/**
 * @name    Bitfield definitions for the XAH_CTRL_2 register
 *
 * This register contains both the CSMA-CA retry counter and the frame retry
 * counter. At this moment only the at86rf232 and the at86rf233 support this
 * register.
 * @{
 */
#define AT86RF2XX_XAH_CTRL_2_MASK__ARET_FRAME_RETRIES           (0xF0)  /* (R), AT86RF232, AT86RF233 */
#define AT86RF2XX_XAH_CTRL_2_MASK__ARET_CSMA_RETRIES            (0x0E)  /* (R), AT86RF232, AT86RF233 */
/** @} */

/**
 * @name    Bitfield definitions for the PLL_CF register
 * @{
 */
#define AT86RF2XX_PLL_CF_MASK__PLL_CF_START                     (0x80)  /* (RW), all */
#define AT86RF2XX_PLL_CF_MASK__PLL_CF                           (0x0F)  /* (RW), AT86RF212B, AT86RF232 */

#define AT86RF2XX_PLL_CF_START__FINISHED                        (0x00) /* Default */
#define AT86RF2XX_PLL_CF_START__TRIGGER                         (0x80)
/** @} */

/**
 * @name    Bitfield definitions for the PLL_DCU register
 * @{
 */
#define AT86RF2XX_PLL_DCU_MASK__PLL_DCU_START                   (0x80) /* (RW), all */

#define AT86RF2XX_PLL_DCU_START__FINISHED                       (0x00) /* Default */
#define AT86RF2XX_PLL_DCU_START__TRIGGER                        (0x80)

/** @} */

/**
 * @name    Bitfield definitions for the XAH_CTRL_0 register
 * @{
 */
#define AT86RF2XX_XAH_CTRL_0_MASK__MAX_FRAME_RETRIES            (0xF0)  /* (RW), all */
#define AT86RF2XX_XAH_CTRL_0_MASK__MAX_CSMA_RETRIES             (0x0E)  /* (RW), all */
#define AT86RF2XX_XAH_CTRL_0_MASK__SLOTTED_OPERATION            (0x01)  /* (RW), all */

#define AT86RF2XX_MAX_CSMA_RETRIES__NO_CSMA                     (0x0E)

#define AT86RF2XX_SLOTTED_OPERATION__NO                         (0x00)  /* Default */
#define AT86RF2XX_SLOTTED_OPERATION__YES                        (0x01)
/** @} */

/**
 * @name    Bitfield definitions for the CSMA_SEED_0 register
 * @{
 */
#define AT86RF2XX_CSMA_SEED_0_MASK__CSMA_SEED_0                 (0xFF) /* (RW), all */
/** @} */

/**
 * @name    Bitfield definitions for the CSMA_SEED_1 register
 * @{
 */
#define AT86RF2XX_CSMA_SEED_1_MASK__AACK_FVN_MODE               (0xC0)  /* (RW), all */
#define AT86RF2XX_CSMA_SEED_1_MASK__AACK_SET_PD                 (0x20)  /* (RW), all */
#define AT86RF2XX_CSMA_SEED_1_MASK__AACK_DIS_ACK                (0x10)  /* (RW), all */
#define AT86RF2XX_CSMA_SEED_1_MASK__AACK_I_AM_COORD             (0x08)  /* (RW), all */
#define AT86RF2XX_CSMA_SEED_1_MASK__CSMA_SEED_1                 (0x07)  /* (RW), all */

#define AT86RF2XX_AACK_FVN_MODE__VER0                           (0x00)
#define AT86RF2XX_AACK_FVN_MODE__VER01                          (0x40)  /* Default */
#define AT86RF2XX_AACK_FVN_MODE__VER012                         (0x80)
#define AT86RF2XX_AACK_FVN_MODE__ANY                            (0xC0)

#define AT86RF2XX_AACK_SET_PD__NO                               (0x00)  /* Default */
#define AT86RF2XX_AACK_SET_PD__YES                              (0x20)

#define AT86RF2XX_AACK_DIS_ACK__NO                              (0x00)  /* Default */
#define AT86RF2XX_AACK_DIS_ACK__YES                             (0x10)

#define AT86RF2XX_AACK_I_AM_COORD__NO                           (0x00)  /* Default */
#define AT86RF2XX_AACK_I_AM_COORD__YES                          (0x08)
/** @} */

/**
 * @name    Bitfield definitions for the CSMA_BE register
 * @{
 */
#define AT86RF2XX_CSMA_BE_MASK__MAX_BE                          (0xF0)  /* (RW), all */
#define AT86RF2XX_CSMA_BE_MASK__MIN_BE                          (0x0F)  /* (RW), all */
/** @} */

/**
 * @name    Bitfield definitions for the TST_CTRL_DIGI register
 * @{
 */
#define AT86RF2XX_TST_CTRL_DIGI_MASK__TST_CTRL_DIG              (0x0F) /* AT86RF232, AT86RF233, AT86RFA1, AT86RFR2 */

#define AT86RF2XX_TST_CTRL_DIG__NONE                            (0x00) /* Default */
#define AT86RF2XX_TST_CTRL_DIG__CONTINUOUS                      (0x0F)
/** @} */

/**
 * @name    Bitfield definitions for the TST_AGC register
 * @{
 */
#define AT86RF2XX_TST_AGC_MASK__AGC_HOLD_SEL                    (0x20)  /* AT86RF233 */
#define AT86RF2XX_TST_AGC_MASK__AGC_RST                         (0x10)  /* AT86RF233 */
#define AT86RF2XX_TST_AGC_MASK__AGC_OFF                         (0x08)  /* AT86RF233 */
#define AT86RF2XX_TST_AGC_MASK__AGC_HOLD                        (0x04)  /* AT86RF233 */
#define AT86RF2XX_TST_AGC_MASK__GC                              (0x03)  /* AT86RF233 */

#define AT86RF2XX_AGC_HOLD_SEL__MANUAL                          (0x20)
#define AT86RF2XX_AGC_HOLD_SEL__NORMAL                          (0x00)  /* Default */

#define AT86RF2XX_AGC_RST__YES                                  (0x10)
#define AT86RF2XX_AGC_RST__NO                                   (0x00)  /* Default */

#define AT86RF2XX_AGC_OFF__YES                                  (0x08)
#define AT86RF2XX_AGC_OFF__NO                                   (0x00)  /* Default */

#define AT86RF2XX_AGC_HOLD__FROZEN                              (0x04)
#define AT86RF2XX_AGC_HOLD__RUNNING                             (0x00)  /* Default */

#define AT86RF2XX_GC__MAX_GAIN                                  (0x00)  /* Default */
#define AT86RF2XX_GC__MED_GAIN                                  (0x01)
#define AT86RF2XX_GC__MIN_GAIN                                  (0x02)

/** @} */

/**
 * @name    Bitfield definitions for the TST_SDM register
 * @{
 */
#define AT86RF2XX_TST_SDM_MASK__MOD_SEL                         (0x80)  /* AT86RF233 */
#define AT86RF2XX_TST_SDM_MASK__MOD                             (0x40)  /* AT86RF233 */
#define AT86RF2XX_TST_SDM_MASK__TX_RX                           (0x20)  /* AT86RF233 */
#define AT86RF2XX_TST_SDM_MASK__TX_RX_SEL                       (0x10)  /* AT86RF233 */

#define AT86RF2XX_MOD_SEL__NORMAL                               (0x00)  /* Default */
#define AT86RF2XX_MOD_SEL__MANUAL                               (0x80)

#define AT86RF2XX_MOD__CONT_0_CHIPS                             (0x00)  /* Default */
#define AT86RF2XX_MOD__CONT_1_CHIPS                             (0x40)

#define AT86RF2XX_TX_RX__RX                                     (0x00)  /* Default */
#define AT86RF2XX_TX_RX__TX                                     (0x20)

#define AT86RF2XX_TX_RX_SEL__NORMAL                             (0x00)  /* Default */
#define AT86RF2XX_TX_RX_SEL__MANUAL                             (0x10)

/** @} */

/**
 * @name    Bitfield definitions for the PHY_TX_TIME register
 * @{
 */
#define AT86RF2XX_PHY_TX_TIME_MASK__IRC_TX_TIME                 (0x0F) /* (R), AT86RF233 */
/** @} */

/**
 * @name    Bitfield definitions for the PHY_PMU_VALUE register
 * @{
 */
#define AT86RF2XX_PHY_PMU_VALUE_MASK__PHY_PMU_VALUE             (0xFF) /* (R), AT86RF233 */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AT86RF2XX_COMMON_REGISTERS_H */
/** @} */
