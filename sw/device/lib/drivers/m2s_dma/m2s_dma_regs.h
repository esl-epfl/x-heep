// Generated register defines for m2s_dma

// Copyright information found in source file:
// Copyright EPFL contributors.

// Licensing information found in source file:
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _M2S_DMA_REG_DEFS_
#define _M2S_DMA_REG_DEFS_

#ifdef __cplusplus
extern "C" {
#endif
// Register width
#define M2S_DMA_PARAM_REG_WIDTH 32

// Control register
#define M2S_DMA_CONTROL_REG_OFFSET 0x0
#define M2S_DMA_CONTROL_SDC_EN_BIT 0

// Transaction done interrupt flag register, enables the transaction done
// interrupt
#define M2S_DMA_TRANSACITON_IFR_REG_OFFSET 0x4
#define M2S_DMA_TRANSACITON_IFR_EN_MASK 0xff
#define M2S_DMA_TRANSACITON_IFR_EN_OFFSET 0
#define M2S_DMA_TRANSACITON_IFR_EN_FIELD \
  ((bitfield_field32_t) { .mask = M2S_DMA_TRANSACITON_IFR_EN_MASK, .index = M2S_DMA_TRANSACITON_IFR_EN_OFFSET })

// Window done interrupt flag register, enables the window done interrupt
#define M2S_DMA_WINDOW_IFR_REG_OFFSET 0x8
#define M2S_DMA_WINDOW_IFR_EN_MASK 0xff
#define M2S_DMA_WINDOW_IFR_EN_OFFSET 0
#define M2S_DMA_WINDOW_IFR_EN_FIELD \
  ((bitfield_field32_t) { .mask = M2S_DMA_WINDOW_IFR_EN_MASK, .index = M2S_DMA_WINDOW_IFR_EN_OFFSET })

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // _M2S_DMA_REG_DEFS_
// End generated register defines for m2s_dma