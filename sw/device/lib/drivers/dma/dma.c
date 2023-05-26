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

void dma_set_read_ptr_inc(const dma_t *dma, uint32_t read_ptr_inc){
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_SRC_PTR_INC_REG_OFFSET), read_ptr_inc);
}

void dma_set_write_ptr_inc(const dma_t *dma, uint32_t write_ptr_inc){
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_DST_PTR_INC_REG_OFFSET), write_ptr_inc);
}

void dma_set_slot(const dma_t *dma, uint16_t rx_slot_mask, uint16_t tx_slot_mask) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_SLOT_REG_OFFSET), (tx_slot_mask << DMA_SLOT_TX_TRIGGER_SLOT_OFFSET) + rx_slot_mask);
}

void dma_set_spi_mode(const dma_t *dma, uint32_t spi_mode){

  printf("MODE: %d\n", spi_mode);

  switch (spi_mode) {
    case DMA_SPI_MODE_DISABLED: {
      dma_set_slot(dma, 0, 0);
    } break;
    case DMA_SPI_MODE_SPI_RX: {
      dma_set_slot(dma, 1, 0);
    } break;
    case DMA_SPI_MODE_SPI_TX: {
      dma_set_slot(dma, 0, 1 << 1);
    } break;
    case DMA_SPI_MODE_SPI_FLASH_RX: {
      dma_set_slot(dma, 1 << 2, 0);
    } break;
    case DMA_SPI_MODE_SPI_FLASH_TX: {
      dma_set_slot(dma, 0, 1 << 3);
    } break;
  }
}

void dma_set_data_type(const dma_t *dma, uint32_t data_type){
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_DATA_TYPE_REG_OFFSET), data_type);
}
