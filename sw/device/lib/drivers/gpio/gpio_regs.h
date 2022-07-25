// Generated register defines for gpio

// Copyright information found in source file:
// Copyright lowRISC contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _GPIO_REG_DEFS_
#define _GPIO_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define GPIO_PARAM_REG_WIDTH 32

// Common Interrupt Offsets

// Interrupt State Register
#define GPIO_INTR_STATE_REG_OFFSET 0x0

// Interrupt Enable Register
#define GPIO_INTR_ENABLE_REG_OFFSET 0x4

// Interrupt Test Register
#define GPIO_INTR_TEST_REG_OFFSET 0x8

// GPIO Input data read value
#define GPIO_DATA_IN_REG_OFFSET 0xc

// GPIO direct output data write value
#define GPIO_DIRECT_OUT_REG_OFFSET 0x10

// GPIO write data lower with mask.
#define GPIO_MASKED_OUT_LOWER_REG_OFFSET 0x14
#define GPIO_MASKED_OUT_LOWER_DATA_MASK 0xffff
#define GPIO_MASKED_OUT_LOWER_DATA_OFFSET 0
#define GPIO_MASKED_OUT_LOWER_DATA_FIELD \
  ((bitfield_field32_t) { .mask = GPIO_MASKED_OUT_LOWER_DATA_MASK, .index = GPIO_MASKED_OUT_LOWER_DATA_OFFSET })
#define GPIO_MASKED_OUT_LOWER_MASK_MASK 0xffff
#define GPIO_MASKED_OUT_LOWER_MASK_OFFSET 16
#define GPIO_MASKED_OUT_LOWER_MASK_FIELD \
  ((bitfield_field32_t) { .mask = GPIO_MASKED_OUT_LOWER_MASK_MASK, .index = GPIO_MASKED_OUT_LOWER_MASK_OFFSET })

// GPIO write data upper with mask.
#define GPIO_MASKED_OUT_UPPER_REG_OFFSET 0x18
#define GPIO_MASKED_OUT_UPPER_DATA_MASK 0xffff
#define GPIO_MASKED_OUT_UPPER_DATA_OFFSET 0
#define GPIO_MASKED_OUT_UPPER_DATA_FIELD \
  ((bitfield_field32_t) { .mask = GPIO_MASKED_OUT_UPPER_DATA_MASK, .index = GPIO_MASKED_OUT_UPPER_DATA_OFFSET })
#define GPIO_MASKED_OUT_UPPER_MASK_MASK 0xffff
#define GPIO_MASKED_OUT_UPPER_MASK_OFFSET 16
#define GPIO_MASKED_OUT_UPPER_MASK_FIELD \
  ((bitfield_field32_t) { .mask = GPIO_MASKED_OUT_UPPER_MASK_MASK, .index = GPIO_MASKED_OUT_UPPER_MASK_OFFSET })

// GPIO Output Enable.
#define GPIO_DIRECT_OE_REG_OFFSET 0x1c

// GPIO write Output Enable lower with mask.
#define GPIO_MASKED_OE_LOWER_REG_OFFSET 0x20
#define GPIO_MASKED_OE_LOWER_DATA_MASK 0xffff
#define GPIO_MASKED_OE_LOWER_DATA_OFFSET 0
#define GPIO_MASKED_OE_LOWER_DATA_FIELD \
  ((bitfield_field32_t) { .mask = GPIO_MASKED_OE_LOWER_DATA_MASK, .index = GPIO_MASKED_OE_LOWER_DATA_OFFSET })
#define GPIO_MASKED_OE_LOWER_MASK_MASK 0xffff
#define GPIO_MASKED_OE_LOWER_MASK_OFFSET 16
#define GPIO_MASKED_OE_LOWER_MASK_FIELD \
  ((bitfield_field32_t) { .mask = GPIO_MASKED_OE_LOWER_MASK_MASK, .index = GPIO_MASKED_OE_LOWER_MASK_OFFSET })

// GPIO write Output Enable upper with mask.
#define GPIO_MASKED_OE_UPPER_REG_OFFSET 0x24
#define GPIO_MASKED_OE_UPPER_DATA_MASK 0xffff
#define GPIO_MASKED_OE_UPPER_DATA_OFFSET 0
#define GPIO_MASKED_OE_UPPER_DATA_FIELD \
  ((bitfield_field32_t) { .mask = GPIO_MASKED_OE_UPPER_DATA_MASK, .index = GPIO_MASKED_OE_UPPER_DATA_OFFSET })
#define GPIO_MASKED_OE_UPPER_MASK_MASK 0xffff
#define GPIO_MASKED_OE_UPPER_MASK_OFFSET 16
#define GPIO_MASKED_OE_UPPER_MASK_FIELD \
  ((bitfield_field32_t) { .mask = GPIO_MASKED_OE_UPPER_MASK_MASK, .index = GPIO_MASKED_OE_UPPER_MASK_OFFSET })

// GPIO interrupt enable for GPIO, rising edge.
#define GPIO_INTR_CTRL_EN_RISING_REG_OFFSET 0x28

// GPIO interrupt enable for GPIO, falling edge.
#define GPIO_INTR_CTRL_EN_FALLING_REG_OFFSET 0x2c

// GPIO interrupt enable for GPIO, level high.
#define GPIO_INTR_CTRL_EN_LVLHIGH_REG_OFFSET 0x30

// GPIO interrupt enable for GPIO, level low.
#define GPIO_INTR_CTRL_EN_LVLLOW_REG_OFFSET 0x34

// filter enable for GPIO input bits.
#define GPIO_CTRL_EN_INPUT_FILTER_REG_OFFSET 0x38

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _GPIO_REG_DEFS_
// End generated register defines for gpio
