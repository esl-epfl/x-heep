// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0


#include <stddef.h>
#include <stdint.h>

#include "dma.h"
#include "dma_regs.h"  // Generated.

void dma_set_read_ptr(const dma_t *dma, uint32_t read_ptr) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_SRC_PTR_REG_OFFSET), read_ptr);
}

void dma_set_write_ptr(const dma_t *dma, uint32_t write_ptr) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_DST_PTR_REG_OFFSET), write_ptr);
}

void dma_set_cnt_start(const dma_t *dma, uint32_t copy_size) {
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_SIZE_REG_OFFSET), copy_size);
}

bool dma_get_done(const dma_t *dma) {
  return mmio_region_get_bit32(dma->base_addr, (ptrdiff_t)(DMA_STATUS_REG_OFFSET), DMA_STATUS_READY_BIT);
}

bool dma_get_halfway(const dma_t *dma) {
    return mmio_region_get_bit32(dma->base_addr, (ptrdiff_t)(DMA_STATUS_REG_OFFSET), DMA_STATUS_WINDOW_DONE_BIT);
}

void dma_set_ptr_inc(const dma_t *dma, uint8_t read_ptr_inc, uint8_t write_ptr_inc){
  mmio_region_write32(dma->base_addr, (ptrdiff_t)(DMA_PTR_INC_REG_OFFSET), (read_ptr_inc << DMA_PTR_INC_SRC_PTR_INC_OFFSET) + (write_ptr_inc << DMA_PTR_INC_DST_PTR_INC_OFFSET));
}

void dma_set_slot(const dma_t *dma, uint16_t rx_slot_mask, uint16_t tx_slot_mask) {
  printf("Wrote @ %08x: %04x << %d + %04x = %08x\n",DMA_SLOT_REG_OFFSET, tx_slot_mask, DMA_SLOT_TX_TRIGGER_SLOT_OFFSET,rx_slot_mask,  (tx_slot_mask << DMA_SLOT_TX_TRIGGER_SLOT_OFFSET) + rx_slot_mask );
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

void dma_enable_circular_mode(const dma_t *dma, bool enable) {
  mmio_region_write32(dma->base_addr, DMA_MODE_REG_OFFSET, enable << DMA_MODE_CIRCULAR_MODE_BIT);
}

void dma_enable_intr(const dma_t *dma, bool done_intr_en, bool window_intr_en){
  mmio_region_write32(dma->base_addr, DMA_INTERRUPT_EN_REG_OFFSET, (done_intr_en << DMA_INTERRUPT_EN_TRANSACTION_DONE_BIT) + (window_intr_en << DMA_INTERRUPT_EN_WINDOW_DONE_BIT));
}
