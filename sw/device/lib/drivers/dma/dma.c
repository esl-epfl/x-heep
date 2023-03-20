// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0


#include <stddef.h>
#include <stdint.h>

#include "dma.h"
#include "dma_regs.h"  // Generated.

void dma_set_read_ptr(const dma_t *dma, uint32_t read_ptr) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(IDMA_REG32_FRONTEND_SRC_ADDR_REG_OFFSET), read_ptr);
}

void dma_set_write_ptr(const dma_t *dma, uint32_t write_ptr) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(IDMA_REG32_FRONTEND_DST_ADDR_REG_OFFSET), write_ptr);
}

void dma_set_cnt(const dma_t *dma, uint32_t copy_size) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(IDMA_REG32_FRONTEND_NUM_BYTES_REG_OFFSET), copy_size);
}

void dma_set_config(const dma_t *dma, uint32_t config) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(IDMA_REG32_FRONTEND_CONF_REG_OFFSET), config);
}

void dma_get_status(const dma_t *dma) {
  return mmio_region_read32(dma->base_addr, (ptrdiff_t)(IDMA_REG32_FRONTEND_STATUS_REG_OFFSET));
}

int32_t dma_get_nextid(const dma_t *dma) {
  return mmio_region_read32(dma->base_addr, (ptrdiff_t)(IDMA_REG32_FRONTEND_NEXT_ID_REG_OFFSET));
}

int32_t dma_get_done(const dma_t *dma) {
  return mmio_region_read32(dma->base_addr, (ptrdiff_t)(IDMA_REG32_FRONTEND_DONE_REG_OFFSET));
}
