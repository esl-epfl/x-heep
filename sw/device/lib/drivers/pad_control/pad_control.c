// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0


#include <stddef.h>
#include <stdint.h>

#include "pad_control.h"
#include "pad_control_regs.h"  // Generated.

void pad_control_set_mux(const pad_control_t *pad_control, ptrdiff_t mux_addr_offset, uint8_t select) {
  mmio_region_write32(pad_control->base_addr, mux_addr_offset, select);
}

uint8_t pad_control_get_mux(const pad_control_t *pad_control, ptrdiff_t mux_addr_offset) {
  return mmio_region_read32(pad_control->base_addr, mux_addr_offset);
}

void pad_control_set_attribute(const pad_control_t *pad_control, ptrdiff_t attribute_addr_offset, uint8_t value) {
  mmio_region_write32(pad_control->base_addr, attribute_addr_offset, value);
}

uint8_t pad_control_get_attribute(const pad_control_t *pad_control, ptrdiff_t attribute_addr_offset) {
  return mmio_region_read32(pad_control->base_addr, attribute_addr_offset);
}
