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

// GPIO Mode
#define GPIO_GPIO_MODE_0_OFFSET 0x8
#define GPIO_GPIO_MODE_1_OFFSET 0xc

// GPIO Enable
#define GPIO_GPIO_EN_OFFSET 0x80

// GPIO Input data read value
#define GPIO_DATA_IN_REG_OFFSET 0x100

// GPIO direct output data write value
#define GPIO_DIRECT_OUT_REG_OFFSET 0x180

// GPIO Set
#define GPIO_GPIO_SET_OFFSET 0x200

// GPIO Clear
#define GPIO_GPIO_CLEAR_OFFSET 0x280

// GPIO Toggle
#define GPIO_GPIO_TOGGLE_OFFSET 0x300

// GPIO interrupt enable for GPIO, rising edge.
#define GPIO_INTR_CTRL_EN_RISING_REG_OFFSET 0x380

// GPIO interrupt enable for GPIO, falling edge.
#define GPIO_INTR_CTRL_EN_FALLING_REG_OFFSET 0x400

// GPIO interrupt enable for GPIO, level high.
#define GPIO_INTR_CTRL_EN_LVLHIGH_REG_OFFSET 0x480

// GPIO interrupt enable for GPIO, level low.
#define GPIO_INTR_CTRL_EN_LVLLOW_REG_OFFSET 0x500


// GPIO interrupt status for GPIO
#define GPIO_INTRPT_STATUS_OFFSET 0x580

// GPIO interrupt status for GPIO, falling edge.
#define GPIO_INTRPT_RISE_STATUS_OFFSET 0x600

// GPIO interrupt status for GPIO, level high.
#define GPIO_INTRPT_FALL_STATUS_OFFSET 0x680

// GPIO interrupt status for GPIO, level low.
#define GPIO_INTRPT_LVL_HIGH_STATUS_OFFSET 0x700

// GPIO interrupt status for GPIO, level low.
#define GPIO_INTRPT_LVL_LOW_STATUS_OFFSET 0x780


#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _GPIO_REG_DEFS_
// End generated register defines for gpio
