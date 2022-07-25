// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _DRIVERS_MEMCOPY_PERIPH_H_
#define _DRIVERS_MEMCOPY_PERIPH_H_

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialization parameters for MEMCOPY PERIPHERAL.
 *
 */
typedef struct memcopy_periph {
  /**
   * The base address for the soc_ctrl hardware registers.
   */
  mmio_region_t base_addr;
} memcopy_periph_t;

/**
 * Write to read_ptr register of the MEMCOPY PERIPHERAL.
 * @param memcopy_periph Pointer to memcopy_periph_t represting the target MEMCOPY PERIPHERAL.
 * @param read_ptr Any valid memory address.
 */
void memcopy_periph_set_read_ptr(const memcopy_periph_t *memcopy_periph, uint32_t read_ptr);

/**
 * Write to write_ptr register of the MEMCOPY PERIPHERAL.
 * @param memcopy_periph Pointer to memcopy_periph_t represting the target MEMCOPY PERIPHERAL.
 * @param write_ptr Any valid memory address.
 */
void memcopy_periph_set_write_ptr(const memcopy_periph_t *memcopy_periph, uint32_t write_ptr);

/**
 * Write to cnt_start register of the MEMCOPY PERIPHERAL.
 * @param memcopy_periph Pointer to memcopy_periph_t represting the target MEMCOPY PERIPHERAL.
 * @param copy_size Number of data to be copied from read_ptr to write_ptr.
 */
void memcopy_periph_set_cnt_start(const memcopy_periph_t *memcopy_periph, uint32_t copy_size);

/**
 * Read from done register of the MEMCOPY PERIPHERAL.
 * @param memcopy_periph Pointer to memcopy_periph_t represting the target MEMCOPY PERIPHERAL.
 * @return done value (0: data are being copied - 1: copy done/peripheral idle)
 */
int32_t memcopy_periph_get_done(const memcopy_periph_t *memcopy_periph);

#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_MEMCOPY_PERIPH_H_
