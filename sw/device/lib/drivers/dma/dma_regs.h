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
#define DMA_SRC_PTR_REG_OFFSET 0x0

// Output data pointer (word aligned)
#define DMA_DST_PTR_REG_OFFSET 0x4

// Addess data pointer (word aligned)
#define DMA_ADDR_PTR_REG_OFFSET 0x8

// Number of bytes to copy - Once a value is written, the copy starts
#define DMA_SIZE_REG_OFFSET 0xc

// Status bits are set to one if a given event occurred
#define DMA_STATUS_REG_OFFSET 0x10
#define DMA_STATUS_READY_BIT 0
#define DMA_STATUS_WINDOW_DONE_BIT 1

// Increment number of src/dst pointer every time a word is copied
#define DMA_PTR_INC_REG_OFFSET 0x14
#define DMA_PTR_INC_SRC_PTR_INC_MASK 0xff
#define DMA_PTR_INC_SRC_PTR_INC_OFFSET 0
#define DMA_PTR_INC_SRC_PTR_INC_FIELD \
  ((bitfield_field32_t) { .mask = DMA_PTR_INC_SRC_PTR_INC_MASK, .index = DMA_PTR_INC_SRC_PTR_INC_OFFSET })
#define DMA_PTR_INC_DST_PTR_INC_MASK 0xff
#define DMA_PTR_INC_DST_PTR_INC_OFFSET 8
#define DMA_PTR_INC_DST_PTR_INC_FIELD \
  ((bitfield_field32_t) { .mask = DMA_PTR_INC_DST_PTR_INC_MASK, .index = DMA_PTR_INC_DST_PTR_INC_OFFSET })

// The DMA will wait for the signal
#define DMA_SLOT_REG_OFFSET 0x18
#define DMA_SLOT_RX_TRIGGER_SLOT_MASK 0xffff
#define DMA_SLOT_RX_TRIGGER_SLOT_OFFSET 0
#define DMA_SLOT_RX_TRIGGER_SLOT_FIELD \
  ((bitfield_field32_t) { .mask = DMA_SLOT_RX_TRIGGER_SLOT_MASK, .index = DMA_SLOT_RX_TRIGGER_SLOT_OFFSET })
#define DMA_SLOT_TX_TRIGGER_SLOT_MASK 0xffff
#define DMA_SLOT_TX_TRIGGER_SLOT_OFFSET 16
#define DMA_SLOT_TX_TRIGGER_SLOT_FIELD \
  ((bitfield_field32_t) { .mask = DMA_SLOT_TX_TRIGGER_SLOT_MASK, .index = DMA_SLOT_TX_TRIGGER_SLOT_OFFSET })

// Width/type of the data to transfer
#define DMA_DATA_TYPE_REG_OFFSET 0x1c
#define DMA_DATA_TYPE_DATA_TYPE_MASK 0x3
#define DMA_DATA_TYPE_DATA_TYPE_OFFSET 0
#define DMA_DATA_TYPE_DATA_TYPE_FIELD \
  ((bitfield_field32_t) { .mask = DMA_DATA_TYPE_DATA_TYPE_MASK, .index = DMA_DATA_TYPE_DATA_TYPE_OFFSET })
#define DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_32BIT_WORD 0x0
#define DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_16BIT_WORD 0x1
#define DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_8BIT_WORD 0x2
#define DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_8BIT_WORD_2 0x3

// Set the operational mode of the DMA
#define DMA_MODE_REG_OFFSET 0x20
#define DMA_MODE_MODE_MASK 0x3
#define DMA_MODE_MODE_OFFSET 0
#define DMA_MODE_MODE_FIELD \
  ((bitfield_field32_t) { .mask = DMA_MODE_MODE_MASK, .index = DMA_MODE_MODE_OFFSET })
#define DMA_MODE_MODE_VALUE_LINEAR_MODE 0x0
#define DMA_MODE_MODE_VALUE_CIRCULAR_MODE 0x1
#define DMA_MODE_MODE_VALUE_ADDRESS_MODE 0x2

// Will trigger a every "WINDOW_SIZE" writes
#define DMA_WINDOW_SIZE_REG_OFFSET 0x24

// Number of times the end of the window was reached since the beginning.
#define DMA_WINDOW_COUNT_REG_OFFSET 0x28

// Interrupt Enable Register
#define DMA_INTERRUPT_EN_REG_OFFSET 0x2c
#define DMA_INTERRUPT_EN_TRANSACTION_DONE_BIT 0
#define DMA_INTERRUPT_EN_WINDOW_DONE_BIT 1

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _DMA_REG_DEFS_
// End generated register defines for dma