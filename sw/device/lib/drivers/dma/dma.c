// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0


#include <stddef.h>
#include <stdint.h>

#include "dma.h"
#include "dma_regs.h"  // Generated.

void dma_set_read_ptr(const dma_t *dma, uint32_t read_ptr) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_PTR_IN_REG_OFFSET), read_ptr);
}

void dma_set_write_ptr(const dma_t *dma, uint32_t write_ptr) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_PTR_OUT_REG_OFFSET), write_ptr);
}

void dma_set_cnt_start(const dma_t *dma, uint32_t copy_size) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_DMA_START_REG_OFFSET), copy_size);
}

int32_t dma_get_done(const dma_t *dma) {
  return mmio_region_read32(dma->base_addr, (ptrdiff_t)(DMA_DONE_REG_OFFSET));
}
