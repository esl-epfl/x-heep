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
 * Write to read_ptr register of the DMA
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @param read_ptr Any valid memory address.
 */
void dma_set_read_ptr(const dma_t *dma, uint32_t read_ptr);

/**
 * Write to write_ptr register of the DMA
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @param write_ptr Any valid memory address.
 */
void dma_set_write_ptr(const dma_t *dma, uint32_t write_ptr);

/**
 * Write to cnt_start register of the DMA
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @param copy_size Number of bytes to be copied from read_ptr to write_ptr.
 */
void dma_set_cnt_start(const dma_t *dma, uint32_t copy_size);

/**
 * Read from done register of the DMA
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @return done value (0: data are being copied - 1: copy done/peripheral idle)
 */
int32_t dma_get_done(const dma_t *dma);

/**
 * Write to src_ptr_inc register of the DMA.
 * @param dma Pointer to dma_t represting the target DMA.
 * @param read_ptr_inc Increment of source pointer (Default: 4).
 */
void dma_set_read_ptr_inc(const dma_t *dma, uint32_t read_ptr_inc);

/**
 * Write to dst_ptr_inc register of the DMA.
 * @param dma Pointer to dma_t represting the target DMA.
 * @param write_ptr_inc Increment of destination pointer (Default: 4).
 */
void dma_set_write_ptr_inc(const dma_t *dma, uint32_t write_ptr_inc);

/**
 * Sets the DMA data transfer modes when used with the SPI.
 * @param dma Pointer to dma_t represting the target DMA.
 * @param spi_mode 0: mem to mem - 1: spi_rx to mem (Default: 0) - 2: mem to spi_tx.
 */
void dma_set_spi_mode(const dma_t *dma, uint32_t spi_mode);

/**
 * Sets the DMA data type.
 * @param dma Pointer to dma_t represting the target DMA.
 * @param data_type Data type to transfer: 32-bit word(0), 16-bit half word (1), 8-bit byte(2,3).
 */
void dma_set_data_type(const dma_t *dma, uint32_t data_type);

#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_DMA_H_
