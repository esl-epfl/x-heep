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

// Increment number of source pointer every time a word is copied from source
#define DMA_SRC_PTR_INC_REG_OFFSET 0x10

// Increment number of source pointer every time a word is copied to
// destination
#define DMA_DST_PTR_INC_REG_OFFSET 0x14

// SPI mode selection: disable(0), receive from SPI (1), and send to SPI (2),
// receive from SPI FLASH (3), and send to SPI FLASH (4). It waits for TX and
// RX FIFO in modes 1 and 2, respectively.
#define DMA_SPI_MODE_REG_OFFSET 0x18
#define DMA_SPI_MODE_SPI_MODE_MASK 0x7
#define DMA_SPI_MODE_SPI_MODE_OFFSET 0
#define DMA_SPI_MODE_SPI_MODE_FIELD \
  ((bitfield_field32_t) { .mask = DMA_SPI_MODE_SPI_MODE_MASK, .index = DMA_SPI_MODE_SPI_MODE_OFFSET })

// Data type to transfer: 32-bit word(0), 16-bit half word(1), 8-bit
// byte(2,3).
#define DMA_DATA_TYPE_REG_OFFSET 0x1c
#define DMA_DATA_TYPE_DATA_TYPE_MASK 0x3
#define DMA_DATA_TYPE_DATA_TYPE_OFFSET 0
#define DMA_DATA_TYPE_DATA_TYPE_FIELD \
  ((bitfield_field32_t) { .mask = DMA_DATA_TYPE_DATA_TYPE_MASK, .index = DMA_DATA_TYPE_DATA_TYPE_OFFSET })

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _DMA_REG_DEFS_
// End generated register defines for dma