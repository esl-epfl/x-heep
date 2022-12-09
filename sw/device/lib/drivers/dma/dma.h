// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _DRIVERS_DMA_H_
#define _DRIVERS_DMA_H_

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
typedef struct dma {
  /**
   * The base address for the soc_ctrl hardware registers.
   */
  mmio_region_t base_addr;
} dma_t;

/**
 * Write to read_ptr register of the MEMCOPY PERIPHERAL.
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @param read_ptr Any valid memory address.
 */
void dma_set_read_ptr(const dma_t *dma, uint32_t read_ptr);

/**
 * Write to write_ptr register of the MEMCOPY PERIPHERAL.
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @param write_ptr Any valid memory address.
 */
void dma_set_write_ptr(const dma_t *dma, uint32_t write_ptr);

/**
 * Write to num_byte register of the MEMCOPY PERIPHERAL.
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @param copy_size Number of data to be copied from read_ptr to write_ptr.
 */
void dma_set_cnt(const dma_t *dma, uint32_t copy_size);

/**
 * Write to configration register of the MEMCOPY PERIPHERAL.
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @param config Configurations of the iDMA
 */
void dma_set_config(const dma_t *dma, uint32_t config);

/**
 * Read from the status register of the MEMCOPY PERIPHERAL.
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @return status value (1: bust, 0: Not bust)
 */
void dma_get_status(const dma_t *dma);

/**
 * Read from the next_id register of the MEMCOPY PERIPHERAL.
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @return next ID, used to launcher transfer
 */
int32_t dma_get_nextid(const dma_t *dma);

/**
 * Read from done register of the MEMCOPY PERIPHERAL.
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @return done value (0: data are being copied - 1: copy done/peripheral idle)
 */
int32_t dma_get_done(const dma_t *dma);

#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_DMA_H_
