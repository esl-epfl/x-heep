// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _DRIVERS_PAD_CONTROL_H_
#define _DRIVERS_PAD_CONTROL_H_

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pad_control {
  /**
   * The base address for the pad_control hardware registers.
   */
  mmio_region_t base_addr;
} pad_control_t;

/**
 * Write to mux_addr register of the pad_control
 * @param pad_control Pointer to pad_control_t represting the target PAD CONTROL PERIPHERAL.
 * @param mux_addr_offset the multiplexer offset address.
 * @param select the value to write
 */
void pad_control_set_mux(const pad_control_t *pad_control, ptrdiff_t mux_addr_offset, uint8_t select);

/**
 * Read the mux_addr register of the pad_control
 * @param pad_control Pointer to pad_control_t represting the target PAD CONTROL PERIPHERAL.
 * @param mux_addr_offset the multiplexer offset address.
 */
uint8_t pad_control_get_mux(const pad_control_t *pad_control, ptrdiff_t mux_addr_offset);

/**
 * Write to attribute register of the pad_control
 * @param pad_control Pointer to pad_control_t represting the target PAD CONTROL PERIPHERAL.
 * @param attribute_addr_offset the attribute offset address.
 * @param value the value to write
 */
void pad_control_set_attribute(const pad_control_t *pad_control, ptrdiff_t attribute_addr_offset, uint8_t value);

/**
 * Read the mux_addr register of the pad_control
 * @param pad_control Pointer to pad_control_t represting the target PAD CONTROL PERIPHERAL.
 * @param attribute_addr_offset the attribute offset address.
 */
uint8_t pad_control_get_attribute(const pad_control_t *pad_control, ptrdiff_t attribute_addr_offset);


#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_PAD_CONTROL_H_
