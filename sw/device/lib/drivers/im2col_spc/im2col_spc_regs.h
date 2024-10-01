// Generated register defines for im2col_spc

// Copyright information found in source file:
// Copyright EPFL contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _IM2COL_SPC_REG_DEFS_
#define _IM2COL_SPC_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define IM2COL_SPC_PARAM_REG_WIDTH 32

// Input data pointer (word aligned)
#define IM2COL_SPC_SRC_PTR_REG_OFFSET 0x0

// Output data pointer (word aligned)
#define IM2COL_SPC_DST_PTR_REG_OFFSET 0x4

// Image width
#define IM2COL_SPC_IW_REG_OFFSET 0x8
#define IM2COL_SPC_IW_SIZE_MASK 0xffff
#define IM2COL_SPC_IW_SIZE_OFFSET 0
#define IM2COL_SPC_IW_SIZE_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_IW_SIZE_MASK, .index = IM2COL_SPC_IW_SIZE_OFFSET })

// Image heigth
#define IM2COL_SPC_IH_REG_OFFSET 0xc
#define IM2COL_SPC_IH_SIZE_MASK 0xffff
#define IM2COL_SPC_IH_SIZE_OFFSET 0
#define IM2COL_SPC_IH_SIZE_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_IH_SIZE_MASK, .index = IM2COL_SPC_IH_SIZE_OFFSET })

// Filter width
#define IM2COL_SPC_FW_REG_OFFSET 0x10
#define IM2COL_SPC_FW_SIZE_MASK 0xff
#define IM2COL_SPC_FW_SIZE_OFFSET 0
#define IM2COL_SPC_FW_SIZE_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_FW_SIZE_MASK, .index = IM2COL_SPC_FW_SIZE_OFFSET })

// Filter heigth
#define IM2COL_SPC_FH_REG_OFFSET 0x14
#define IM2COL_SPC_FH_SIZE_MASK 0xff
#define IM2COL_SPC_FH_SIZE_OFFSET 0
#define IM2COL_SPC_FH_SIZE_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_FH_SIZE_MASK, .index = IM2COL_SPC_FH_SIZE_OFFSET })

// Batch number
#define IM2COL_SPC_BATCH_REG_OFFSET 0x18
#define IM2COL_SPC_BATCH_SIZE_MASK 0xff
#define IM2COL_SPC_BATCH_SIZE_OFFSET 0
#define IM2COL_SPC_BATCH_SIZE_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_BATCH_SIZE_MASK, .index = IM2COL_SPC_BATCH_SIZE_OFFSET })

// Number of channels. When written, the im2col will start executing
#define IM2COL_SPC_NUM_CH_REG_OFFSET 0x1c
#define IM2COL_SPC_NUM_CH_NUM_MASK 0xff
#define IM2COL_SPC_NUM_CH_NUM_OFFSET 0
#define IM2COL_SPC_NUM_CH_NUM_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_NUM_CH_NUM_MASK, .index = IM2COL_SPC_NUM_CH_NUM_OFFSET })

// Number of iterations to perform
#define IM2COL_SPC_CH_COL_REG_OFFSET 0x20
#define IM2COL_SPC_CH_COL_NUM_MASK 0xffff
#define IM2COL_SPC_CH_COL_NUM_OFFSET 0
#define IM2COL_SPC_CH_COL_NUM_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_CH_COL_NUM_MASK, .index = IM2COL_SPC_CH_COL_NUM_OFFSET })

// Number of patches along W
#define IM2COL_SPC_N_PATCHES_W_REG_OFFSET 0x24
#define IM2COL_SPC_N_PATCHES_W_NUM_MASK 0xffff
#define IM2COL_SPC_N_PATCHES_W_NUM_OFFSET 0
#define IM2COL_SPC_N_PATCHES_W_NUM_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_N_PATCHES_W_NUM_MASK, .index = IM2COL_SPC_N_PATCHES_W_NUM_OFFSET })

// Number of patches along H
#define IM2COL_SPC_N_PATCHES_H_REG_OFFSET 0x28
#define IM2COL_SPC_N_PATCHES_H_NUM_MASK 0xffff
#define IM2COL_SPC_N_PATCHES_H_NUM_OFFSET 0
#define IM2COL_SPC_N_PATCHES_H_NUM_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_N_PATCHES_H_NUM_MASK, .index = IM2COL_SPC_N_PATCHES_H_NUM_OFFSET })

// Adapted right padded region
#define IM2COL_SPC_ADPT_PAD_RIGHT_REG_OFFSET 0x2c
#define IM2COL_SPC_ADPT_PAD_RIGHT_SIZE_MASK 0xff
#define IM2COL_SPC_ADPT_PAD_RIGHT_SIZE_OFFSET 0
#define IM2COL_SPC_ADPT_PAD_RIGHT_SIZE_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_ADPT_PAD_RIGHT_SIZE_MASK, .index = IM2COL_SPC_ADPT_PAD_RIGHT_SIZE_OFFSET })

// Adapted bottom padded region
#define IM2COL_SPC_ADPT_PAD_BOTTOM_REG_OFFSET 0x30
#define IM2COL_SPC_ADPT_PAD_BOTTOM_SIZE_MASK 0xff
#define IM2COL_SPC_ADPT_PAD_BOTTOM_SIZE_OFFSET 0
#define IM2COL_SPC_ADPT_PAD_BOTTOM_SIZE_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_ADPT_PAD_BOTTOM_SIZE_MASK, .index = IM2COL_SPC_ADPT_PAD_BOTTOM_SIZE_OFFSET })

// Logarithmic number of strides along D1, set to 1 for no stride
#define IM2COL_SPC_LOG_STRIDES_D1_REG_OFFSET 0x34
#define IM2COL_SPC_LOG_STRIDES_D1_SIZE_MASK 0xf
#define IM2COL_SPC_LOG_STRIDES_D1_SIZE_OFFSET 0
#define IM2COL_SPC_LOG_STRIDES_D1_SIZE_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_LOG_STRIDES_D1_SIZE_MASK, .index = IM2COL_SPC_LOG_STRIDES_D1_SIZE_OFFSET })

// Logarithmic number of strides along D2, set to 1 for no stride
#define IM2COL_SPC_LOG_STRIDES_D2_REG_OFFSET 0x38
#define IM2COL_SPC_LOG_STRIDES_D2_SIZE_MASK 0xf
#define IM2COL_SPC_LOG_STRIDES_D2_SIZE_OFFSET 0
#define IM2COL_SPC_LOG_STRIDES_D2_SIZE_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_LOG_STRIDES_D2_SIZE_MASK, .index = IM2COL_SPC_LOG_STRIDES_D2_SIZE_OFFSET })

// Status bit is set to one when the im2col SPC is ready
#define IM2COL_SPC_STATUS_REG_OFFSET 0x3c
#define IM2COL_SPC_STATUS_READY_BIT 0

// The DMA will wait for the signal
#define IM2COL_SPC_SLOT_REG_OFFSET 0x40
#define IM2COL_SPC_SLOT_RX_TRIGGER_SLOT_MASK 0xffff
#define IM2COL_SPC_SLOT_RX_TRIGGER_SLOT_OFFSET 0
#define IM2COL_SPC_SLOT_RX_TRIGGER_SLOT_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_SLOT_RX_TRIGGER_SLOT_MASK, .index = IM2COL_SPC_SLOT_RX_TRIGGER_SLOT_OFFSET })
#define IM2COL_SPC_SLOT_TX_TRIGGER_SLOT_MASK 0xffff
#define IM2COL_SPC_SLOT_TX_TRIGGER_SLOT_OFFSET 16
#define IM2COL_SPC_SLOT_TX_TRIGGER_SLOT_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_SLOT_TX_TRIGGER_SLOT_MASK, .index = IM2COL_SPC_SLOT_TX_TRIGGER_SLOT_OFFSET })

// Width/type of the data to transfer
#define IM2COL_SPC_DATA_TYPE_REG_OFFSET 0x44
#define IM2COL_SPC_DATA_TYPE_DATA_TYPE_MASK 0x3
#define IM2COL_SPC_DATA_TYPE_DATA_TYPE_OFFSET 0
#define IM2COL_SPC_DATA_TYPE_DATA_TYPE_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_DATA_TYPE_DATA_TYPE_MASK, .index = IM2COL_SPC_DATA_TYPE_DATA_TYPE_OFFSET })
#define IM2COL_SPC_DATA_TYPE_DATA_TYPE_VALUE_DMA_32BIT_WORD 0x0
#define IM2COL_SPC_DATA_TYPE_DATA_TYPE_VALUE_DMA_16BIT_WORD 0x1
#define IM2COL_SPC_DATA_TYPE_DATA_TYPE_VALUE_DMA_8BIT_WORD 0x2
#define IM2COL_SPC_DATA_TYPE_DATA_TYPE_VALUE_DMA_8BIT_WORD_2 0x3

// Set the top padding
#define IM2COL_SPC_PAD_TOP_REG_OFFSET 0x48
#define IM2COL_SPC_PAD_TOP_PAD_MASK 0x3f
#define IM2COL_SPC_PAD_TOP_PAD_OFFSET 0
#define IM2COL_SPC_PAD_TOP_PAD_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_PAD_TOP_PAD_MASK, .index = IM2COL_SPC_PAD_TOP_PAD_OFFSET })

// Set the bottom padding
#define IM2COL_SPC_PAD_BOTTOM_REG_OFFSET 0x4c
#define IM2COL_SPC_PAD_BOTTOM_PAD_MASK 0x3f
#define IM2COL_SPC_PAD_BOTTOM_PAD_OFFSET 0
#define IM2COL_SPC_PAD_BOTTOM_PAD_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_PAD_BOTTOM_PAD_MASK, .index = IM2COL_SPC_PAD_BOTTOM_PAD_OFFSET })

// Set the right padding
#define IM2COL_SPC_PAD_RIGHT_REG_OFFSET 0x50
#define IM2COL_SPC_PAD_RIGHT_PAD_MASK 0x3f
#define IM2COL_SPC_PAD_RIGHT_PAD_OFFSET 0
#define IM2COL_SPC_PAD_RIGHT_PAD_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_PAD_RIGHT_PAD_MASK, .index = IM2COL_SPC_PAD_RIGHT_PAD_OFFSET })

// Set the left padding
#define IM2COL_SPC_PAD_LEFT_REG_OFFSET 0x54
#define IM2COL_SPC_PAD_LEFT_PAD_MASK 0x3f
#define IM2COL_SPC_PAD_LEFT_PAD_OFFSET 0
#define IM2COL_SPC_PAD_LEFT_PAD_FIELD \
  ((bitfield_field32_t) { .mask = IM2COL_SPC_PAD_LEFT_PAD_MASK, .index = IM2COL_SPC_PAD_LEFT_PAD_OFFSET })

// Interrupt Enable Register
#define IM2COL_SPC_INTERRUPT_EN_REG_OFFSET 0x58
#define IM2COL_SPC_INTERRUPT_EN_EN_BIT 0

// Interrupt Flag Register for the SPC operation
#define IM2COL_SPC_SPC_IFR_REG_OFFSET 0x5c
#define IM2COL_SPC_SPC_IFR_FLAG_BIT 0

// Mask that defines which DMA channel the SPC can access
#define IM2COL_SPC_SPC_CH_MASK_REG_OFFSET 0x60

// Offset of the DMA channel the SPC can access
#define IM2COL_SPC_SPC_CH_OFFSET_REG_OFFSET 0x64

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _IM2COL_SPC_REG_DEFS_
// End generated register defines for im2col_spc