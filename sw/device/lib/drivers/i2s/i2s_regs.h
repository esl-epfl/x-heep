// Generated register defines for i2s

// Copyright information found in source file:
// Copyright EPFL contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _I2S_REG_DEFS_
#define _I2S_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Defines the maximal number of bytes that can be
#define I2S_PARAM_BYTE_PER_SAMPLE_WIDTH 2

// Bits available for the clock divider
#define I2S_PARAM_CLK_DIV_SIZE 16

// Register width
#define I2S_PARAM_REG_WIDTH 32

// Control register
#define I2S_CLKDIVIDX_REG_OFFSET 0x0
#define I2S_CLKDIVIDX_COUNT_MASK 0xffff
#define I2S_CLKDIVIDX_COUNT_OFFSET 0
#define I2S_CLKDIVIDX_COUNT_FIELD \
  ((bitfield_field32_t) { .mask = I2S_CLKDIVIDX_COUNT_MASK, .index = I2S_CLKDIVIDX_COUNT_OFFSET })

// config register
#define I2S_CFG_REG_OFFSET 0x4
#define I2S_CFG_EN_MASK 0x3
#define I2S_CFG_EN_OFFSET 0
#define I2S_CFG_EN_FIELD \
  ((bitfield_field32_t) { .mask = I2S_CFG_EN_MASK, .index = I2S_CFG_EN_OFFSET })
#define I2S_CFG_EN_VALUE_DISABLED 0x0
#define I2S_CFG_EN_VALUE_ONLY_LEFT 0x1
#define I2S_CFG_EN_VALUE_ONLY_RIGHT 0x2
#define I2S_CFG_EN_VALUE_BOTH_CHANNELS 0x3
#define I2S_CFG_LSB_FIRST_BIT 2
#define I2S_CFG_INTR_EN_BIT 3
#define I2S_CFG_DATA_WIDTH_MASK 0x3
#define I2S_CFG_DATA_WIDTH_OFFSET 4
#define I2S_CFG_DATA_WIDTH_FIELD \
  ((bitfield_field32_t) { .mask = I2S_CFG_DATA_WIDTH_MASK, .index = I2S_CFG_DATA_WIDTH_OFFSET })
#define I2S_CFG_DATA_WIDTH_VALUE_8_BITS 0x0
#define I2S_CFG_DATA_WIDTH_VALUE_16_BITS 0x1
#define I2S_CFG_DATA_WIDTH_VALUE_24_BITS 0x2
#define I2S_CFG_DATA_WIDTH_VALUE_32_BITS 0x3
#define I2S_CFG_GEN_CLK_WS_BIT 6

// Watermark to reach for an interrupt
#define I2S_WATERMARK_REG_OFFSET 0x8

// Status flags of the I2s peripheral
#define I2S_STATUS_REG_OFFSET 0xc
#define I2S_STATUS_RX_DATA_READY_BIT 0
#define I2S_STATUS_RX_OVERFLOW_BIT 1

// Memory area: I2s Receive data
#define I2S_RXDATA_REG_OFFSET 0x10
#define I2S_RXDATA_SIZE_WORDS 1
#define I2S_RXDATA_SIZE_BYTES 4
#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _I2S_REG_DEFS_
// End generated register defines for i2s