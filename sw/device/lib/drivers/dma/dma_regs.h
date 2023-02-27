// Generated register defines for dma

// Copyright information found in source file:
// Copyright EPFL contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _DMA_REG_DEFS_
#define _DMA_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define DMA_PARAM_REG_WIDTH 32

// Input data pointer (word aligned)
#define DMA_PTR_IN_REG_OFFSET 0x0

// Output data pointer (word aligned)
#define DMA_PTR_OUT_REG_OFFSET 0x4

// Number of bytes to copy - Once a value is written, the copy starts
#define DMA_DMA_START_REG_OFFSET 0x8

// Register set to 1 when copy is done
#define DMA_DONE_REG_OFFSET 0xc
#define DMA_DONE_DONE_BIT 0
#define DMA_DONE_HALFWAY_BIT 1

// Increment number of source pointer every time a word is copied from source
#define DMA_SRC_PTR_INC_REG_OFFSET 0x10

// Increment number of source pointer every time a word is copied to
// destination
#define DMA_DST_PTR_INC_REG_OFFSET 0x14

// Special wait mode selection.
#define DMA_RX_WAIT_MODE_REG_OFFSET 0x18

// Special wait mode selection.
#define DMA_TX_WAIT_MODE_REG_OFFSET 0x1c

// Data type to transfer: 32-bit word(0), 16-bit half word(1), 8-bit
// byte(2,3).
#define DMA_DATA_TYPE_REG_OFFSET 0x20
#define DMA_DATA_TYPE_DATA_TYPE_MASK 0x3
#define DMA_DATA_TYPE_DATA_TYPE_OFFSET 0
#define DMA_DATA_TYPE_DATA_TYPE_FIELD \
  ((bitfield_field32_t) { .mask = DMA_DATA_TYPE_DATA_TYPE_MASK, .index = DMA_DATA_TYPE_DATA_TYPE_OFFSET })
#define DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_32BIT_WORD 0x0
#define DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_16BIT_WORD 0x1
#define DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_8BIT_WORD 0x2
#define DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_8BIT_WORD_2 0x3

// Restarts copying as soon as end of buffer is reach.
#define DMA_CIRCULAR_MODE_REG_OFFSET 0x24
#define DMA_CIRCULAR_MODE_CIRCULAR_MODE_BIT 0

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _DMA_REG_DEFS_
// End generated register defines for dma