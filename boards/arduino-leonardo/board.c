/*
 * SPDX-FileCopyrightText: 2017 Thomas Perrot <thomas.perrot@tupi.fr>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     boards_arduino-leonardo
 * @{
 *
 * @file
 * @brief       Board specific initialization for Arduino Leonardo
 *
 * @author      Thomas Perrot <thomas.perrot@tupi.fr>
 *
 * @}
 */

#include "board.h"
#include "mtd_eeprom.h"

#if MODULE_MTD_EEPROM
static mtd_eeprom_t eeprom_mtd = MTD_EEPROM_INIT_VAL;
MTD_XFA_ADD(eeprom_mtd, 0);
#endif /* MODULE_MTD_EEPROM */

void board_init(void)
{
}
