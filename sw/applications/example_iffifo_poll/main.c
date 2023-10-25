/*
 * Copyright EPFL contributors.
 * Licensed under the Apache License, Version 2.0, see LICENSE for details.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Author: Pierre Guillod <pierre.guillod@epfl.ch>
 */

#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"

#include "iffifo_regs.h"

#include "mmio.h"

#include "csr.h"

#include "fast_intr_ctrl.h"
#include "dma.h"


#define DMA_CSR_REG_MIE_MASK 0xFFFFFFFF


#define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)

unsigned int IFFIFO_START_ADDRESS = EXT_PERIPHERAL_START_ADDRESS + 0x2000;

void iffifo_isr(void)    
{
  mmio_region_t iffifo_base_addr = mmio_region_from_addr((uintptr_t)IFFIFO_START_ADDRESS);
  // Assert and disable interrupts
  mmio_region_write32(iffifo_base_addr, IFFIFO_INTERRUPTS_REG_OFFSET, 0b000);
  PRINTF(" ** REACH interrupt fired.");
}

void print_status_register(void)
{
  mmio_region_t iffifo_base_addr = mmio_region_from_addr((uintptr_t)IFFIFO_START_ADDRESS);
  int32_t status = mmio_region_read32(iffifo_base_addr, IFFIFO_STATUS_REG_OFFSET);
  PRINTF("STATUS = ");
  PRINTF(status & 1 ? "E" : "-"); // FIFO empty
  PRINTF(status & 2 ? "A" : "-"); // Data available in FIFO
  PRINTF(status & 4 ? "R" : "-"); // Watermark reached
  PRINTF(status & 8 ? "F" : "-"); // FIFO full
  PRINTF("\n");
}

int main(int argc, char *argv[]) {

    CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK );

    mmio_region_t iffifo_base_addr = mmio_region_from_addr((uintptr_t)IFFIFO_START_ADDRESS);
    
    mmio_region_write32(iffifo_base_addr, IFFIFO_WATERMARK_REG_OFFSET, 2);
    mmio_region_write32(iffifo_base_addr, IFFIFO_INTERRUPTS_REG_OFFSET, 0b010);
    print_status_register();
    mmio_region_write32(iffifo_base_addr, IFFIFO_FIFO_IN_REG_OFFSET, 1);
    print_status_register();
    mmio_region_write32(iffifo_base_addr, IFFIFO_FIFO_IN_REG_OFFSET, 2);
    print_status_register();
    mmio_region_write32(iffifo_base_addr, IFFIFO_FIFO_IN_REG_OFFSET, 3);
    print_status_register();
    mmio_region_write32(iffifo_base_addr, IFFIFO_FIFO_IN_REG_OFFSET, 4);
    print_status_register();
    mmio_region_read32(iffifo_base_addr, IFFIFO_FIFO_OUT_REG_OFFSET);
    print_status_register();
    mmio_region_read32(iffifo_base_addr, IFFIFO_FIFO_OUT_REG_OFFSET);
    print_status_register();
    mmio_region_read32(iffifo_base_addr, IFFIFO_FIFO_OUT_REG_OFFSET);
    print_status_register();
    mmio_region_read32(iffifo_base_addr, IFFIFO_FIFO_OUT_REG_OFFSET);
    print_status_register();
    
    return EXIT_SUCCESS;
    
}

