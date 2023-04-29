// Generated register defines for fast_intr_ctrl

// Copyright information found in source file:
// Copyright lowRISC contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _FAST_INTR_CTRL_REG_DEFS_
#define _FAST_INTR_CTRL_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define FAST_INTR_CTRL_PARAM_REG_WIDTH 32

// Pending fast interrupt
#define FAST_INTR_CTRL_FAST_INTR_PENDING_REG_OFFSET 0x0
#define FAST_INTR_CTRL_FAST_INTR_PENDING_FAST_INTR_PENDING_MASK 0x7fff
#define FAST_INTR_CTRL_FAST_INTR_PENDING_FAST_INTR_PENDING_OFFSET 0
#define FAST_INTR_CTRL_FAST_INTR_PENDING_FAST_INTR_PENDING_FIELD \
  ((bitfield_field32_t) { .mask = FAST_INTR_CTRL_FAST_INTR_PENDING_FAST_INTR_PENDING_MASK, .index = FAST_INTR_CTRL_FAST_INTR_PENDING_FAST_INTR_PENDING_OFFSET })

// Clear fast interrupt
#define FAST_INTR_CTRL_FAST_INTR_CLEAR_REG_OFFSET 0x4
#define FAST_INTR_CTRL_FAST_INTR_CLEAR_FAST_INTR_CLEAR_MASK 0x7fff
#define FAST_INTR_CTRL_FAST_INTR_CLEAR_FAST_INTR_CLEAR_OFFSET 0
#define FAST_INTR_CTRL_FAST_INTR_CLEAR_FAST_INTR_CLEAR_FIELD \
  ((bitfield_field32_t) { .mask = FAST_INTR_CTRL_FAST_INTR_CLEAR_FAST_INTR_CLEAR_MASK, .index = FAST_INTR_CTRL_FAST_INTR_CLEAR_FAST_INTR_CLEAR_OFFSET })

// Enable fast interrupt
#define FAST_INTR_CTRL_FAST_INTR_ENABLE_REG_OFFSET 0x8
#define FAST_INTR_CTRL_FAST_INTR_ENABLE_FAST_INTR_ENABLE_MASK 0x7fff
#define FAST_INTR_CTRL_FAST_INTR_ENABLE_FAST_INTR_ENABLE_OFFSET 0
#define FAST_INTR_CTRL_FAST_INTR_ENABLE_FAST_INTR_ENABLE_FIELD \
  ((bitfield_field32_t) { .mask = FAST_INTR_CTRL_FAST_INTR_ENABLE_FAST_INTR_ENABLE_MASK, .index = FAST_INTR_CTRL_FAST_INTR_ENABLE_FAST_INTR_ENABLE_OFFSET })

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _FAST_INTR_CTRL_REG_DEFS_
// End generated register defines for fast_intr_ctrl