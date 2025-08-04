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

// Size of the transaction
#define DLC_TRANS_SIZE_REG_OFFSET 0x0
#define DLC_TRANS_SIZE_SIZE_MASK 0xffff
#define DLC_TRANS_SIZE_SIZE_OFFSET 0
#define DLC_TRANS_SIZE_SIZE_FIELD \
  ((bitfield_field32_t) { .mask = DLC_TRANS_SIZE_SIZE_MASK, .index = DLC_TRANS_SIZE_SIZE_OFFSET })

// Current level of the dLC (can be used as initial condition)
#define DLC_CURR_LVL_REG_OFFSET 0x4
#define DLC_CURR_LVL_LVL_MASK 0xffff
#define DLC_CURR_LVL_LVL_OFFSET 0
#define DLC_CURR_LVL_LVL_FIELD \
  ((bitfield_field32_t) { .mask = DLC_CURR_LVL_LVL_MASK, .index = DLC_CURR_LVL_LVL_OFFSET })

// Whether or not to have 1-level of hysteresis
#define DLC_HYSTERESIS_EN_REG_OFFSET 0x8
#define DLC_HYSTERESIS_EN_HYSTERESIS_BIT 0

// Log2 of the level width
#define DLC_DLVL_LOG_LEVEL_WIDTH_REG_OFFSET 0xc
#define DLC_DLVL_LOG_LEVEL_WIDTH_LOG_WL_MASK 0xf
#define DLC_DLVL_LOG_LEVEL_WIDTH_LOG_WL_OFFSET 0
#define DLC_DLVL_LOG_LEVEL_WIDTH_LOG_WL_FIELD \
  ((bitfield_field32_t) { .mask = DLC_DLVL_LOG_LEVEL_WIDTH_LOG_WL_MASK, .index = DLC_DLVL_LOG_LEVEL_WIDTH_LOG_WL_OFFSET })

// Number of LSB to discard
#define DLC_DISCARD_BITS_REG_OFFSET 0x10
#define DLC_DISCARD_BITS_DISCARD_MASK 0xf
#define DLC_DISCARD_BITS_DISCARD_OFFSET 0
#define DLC_DISCARD_BITS_DISCARD_FIELD \
  ((bitfield_field32_t) { .mask = DLC_DISCARD_BITS_DISCARD_MASK, .index = DLC_DISCARD_BITS_DISCARD_OFFSET })

// Delta level N bits
#define DLC_DLVL_N_BITS_REG_OFFSET 0x14
#define DLC_DLVL_N_BITS_N_BITS_MASK 0xf
#define DLC_DLVL_N_BITS_N_BITS_OFFSET 0
#define DLC_DLVL_N_BITS_N_BITS_FIELD \
  ((bitfield_field32_t) { .mask = DLC_DLVL_N_BITS_N_BITS_MASK, .index = DLC_DLVL_N_BITS_N_BITS_OFFSET })

// Delta level mask
#define DLC_DLVL_MASK_REG_OFFSET 0x18
#define DLC_DLVL_MASK_MASK_MASK 0xffff
#define DLC_DLVL_MASK_MASK_OFFSET 0
#define DLC_DLVL_MASK_MASK_FIELD \
  ((bitfield_field32_t) { .mask = DLC_DLVL_MASK_MASK_MASK, .index = DLC_DLVL_MASK_MASK_OFFSET })

// Delta level format
#define DLC_DLVL_FORMAT_REG_OFFSET 0x1c
#define DLC_DLVL_FORMAT_TWOSCOMP_N_SGNMOD_BIT 0

// Delta time mask
#define DLC_DT_MASK_REG_OFFSET 0x20
#define DLC_DT_MASK_MASK_MASK 0xffff
#define DLC_DT_MASK_MASK_OFFSET 0
#define DLC_DT_MASK_MASK_FIELD \
  ((bitfield_field32_t) { .mask = DLC_DT_MASK_MASK_MASK, .index = DLC_DT_MASK_MASK_OFFSET })

// Bypass input data to output without filter
#define DLC_BYPASS_REG_OFFSET 0x24
#define DLC_BYPASS_BP_BIT 0

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _DLC_REG_DEFS_
// End generated register defines for dlc