/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * ad2433.h  --  AD2433 ALSA SoC audio driver
 *
 * Copyright 2018 Realtek Microelectronics
 * Author: Bard Liao <bardliao@realtek.com>
 */

#ifndef __AD2433_H__
#define __AD2433_H__

/* --- PRINTF_BYTE_TO_BINARY macro's --- */
#define PRINTF_BINARY_PATTERN_INT8 "%c%c%c%c %c%c%c%c"
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BINARY_PATTERN_INT16 \
      PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
      PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   PRINTF_BYTE_TO_BINARY_INT8(i)

/* Define how often to check (and clear) the fault status register (in ms) */
#define AD2433_FAULT_CHECK_INTERVAL 2000

#endif /* __AD2433_H__ */
