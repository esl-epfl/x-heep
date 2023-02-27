// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#ifndef _DRIVERS_DMA_H_
#define _DRIVERS_DMA_H_

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"


/**
 * Wait Mode Defines
 * 
 */
#define DMA_RX_WAIT_MODE_DISABLED 0
#define DMA_RX_WAIT_SPI           1
#define DMA_RX_WAIT_SPI_FLASH     2

#define DMA_TX_WAIT_MODE_DISABLED 0
#define DMA_RX_WAIT_SPI           1
#define DMA_RX_WAIT_SPI_FLASH     2


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialization parameters for DMA PERIPHERAL.
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
bool dma_get_done(const dma_t *dma);

/**
 * Read the halfway flag from done register of the DMA
 * @param dma Pointer to dma_t represting the target MEMCOPY PERIPHERAL.
 * @return halfway value (0: dma is processing first have - 1: first have is done)
 */
bool dma_get_halfway(const dma_t *dma);

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
 * Sets the DMA data transfer modes when used with peripherals
 * @param dma Pointer to dma_t represting the target DMA.
 * @param peripheral_mask mask to listen to correct peripheral
 */
void dma_set_rx_wait_mode(const dma_t *dma, uint32_t peripheral_mask);

/**
 * Sets the DMA data transfer modes when used with peripherals
 * @param dma Pointer to dma_t represting the target DMA.
 * @param peripheral_mask mask to listen to correct peripheral
 */
void dma_set_tx_wait_mode(const dma_t *dma, uint32_t peripheral_mask);

/**
 * Sets the DMA data type.
 * @param dma Pointer to dma_t represting the target DMA.
 * @param data_type Data type to transfer: 32-bit word(0), 16-bit half word (1), 8-bit byte(2,3).
 */
void dma_set_data_type(const dma_t *dma, uint32_t data_type);

/**
 * Enables/disables the cirucular mode of the DMA.
 * @param dma Pointer to dma_t represting the target DMA.
 * @param enable bool.
 */
void dma_enable_circular_mode(const dma_t *dma, bool enable);

#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_DMA_H_
