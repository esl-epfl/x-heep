// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0


#include <stddef.h>
#include <stdint.h>

#include "memcopy_periph.h"
#include "memcopy_periph_regs.h"  // Generated.

void memcopy_periph_set_read_ptr(const memcopy_periph_t *memcopy_periph, uint32_t read_ptr) {
  mmio_region_write32(memcopy_periph->base_addr, (ptrdiff_t)(MEMCOPY_PERIPH_PTR_READ_REG_OFFSET), read_ptr);
}

void memcopy_periph_set_write_ptr(const memcopy_periph_t *memcopy_periph, uint32_t write_ptr) {
  mmio_region_write32(memcopy_periph->base_addr, (ptrdiff_t)(MEMCOPY_PERIPH_PTR_WRITE_REG_OFFSET), write_ptr);
}

void memcopy_periph_set_cnt_start(const memcopy_periph_t *memcopy_periph, uint32_t copy_size) {
  mmio_region_write32(memcopy_periph->base_addr, (ptrdiff_t)(MEMCOPY_PERIPH_CNT_START_REG_OFFSET), copy_size);
}

int32_t memcopy_periph_get_done(const memcopy_periph_t *memcopy_periph) {
  return mmio_region_read32(memcopy_periph->base_addr, (ptrdiff_t)(MEMCOPY_PERIPH_DONE_REG_OFFSET));
}
