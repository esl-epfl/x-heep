// Generated register defines for dlc

// Copyright information found in source file:
// Copyright EPFL and Politecnico di Torino contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _DLC_REG_DEFS_
#define _DLC_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define DLC_PARAM_REG_WIDTH 32

// Log2 of the level width
#define DLC_DLVL_LOG_LEVEL_WIDTH_REG_OFFSET 0x0
#define DLC_DLVL_LOG_LEVEL_WIDTH_LOG_WL_MASK 0xf
#define DLC_DLVL_LOG_LEVEL_WIDTH_LOG_WL_OFFSET 0
#define DLC_DLVL_LOG_LEVEL_WIDTH_LOG_WL_FIELD \
  ((bitfield_field32_t) { .mask = DLC_DLVL_LOG_LEVEL_WIDTH_LOG_WL_MASK, .index = DLC_DLVL_LOG_LEVEL_WIDTH_LOG_WL_OFFSET })

// Delta level N bits
#define DLC_DLVL_N_BITS_REG_OFFSET 0x4
#define DLC_DLVL_N_BITS_N_BITS_MASK 0xf
#define DLC_DLVL_N_BITS_N_BITS_OFFSET 0
#define DLC_DLVL_N_BITS_N_BITS_FIELD \
  ((bitfield_field32_t) { .mask = DLC_DLVL_N_BITS_N_BITS_MASK, .index = DLC_DLVL_N_BITS_N_BITS_OFFSET })

// Delta level mask
#define DLC_DLVL_MASK_REG_OFFSET 0x8
#define DLC_DLVL_MASK_MASK_MASK 0xffff
#define DLC_DLVL_MASK_MASK_OFFSET 0
#define DLC_DLVL_MASK_MASK_FIELD \
  ((bitfield_field32_t) { .mask = DLC_DLVL_MASK_MASK_MASK, .index = DLC_DLVL_MASK_MASK_OFFSET })

// Delta level format
#define DLC_DLVL_FORMAT_REG_OFFSET 0xc
#define DLC_DLVL_FORMAT_TWOSCOMP_N_SGNMOD_BIT 0

// Delta time mask
#define DLC_DT_MASK_REG_OFFSET 0x10
#define DLC_DT_MASK_MASK_MASK 0xffff
#define DLC_DT_MASK_MASK_OFFSET 0
#define DLC_DT_MASK_MASK_FIELD \
  ((bitfield_field32_t) { .mask = DLC_DT_MASK_MASK_MASK, .index = DLC_DT_MASK_MASK_OFFSET })

// dLC response push setting
#define DLC_READNOTWRITE_REG_OFFSET 0x14
#define DLC_READNOTWRITE_RNW_BIT 0

// Bypass input data to output without filter
#define DLC_BYPASS_REG_OFFSET 0x18
#define DLC_BYPASS_BP_BIT 0

// Interrupt enable to be used in bypass mode
#define DLC_INTERRUPT_EN_REG_OFFSET 0x1c
#define DLC_INTERRUPT_EN_EN_BIT 0

// Interrupt bit set whena crossing is detected
#define DLC_XING_INTR_REG_OFFSET 0x20
#define DLC_XING_INTR_XING_BIT 0

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _DLC_REG_DEFS_
// End generated register defines for dlc