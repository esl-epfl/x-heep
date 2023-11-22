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

#include "x-heep.h"
#include "iffifo_regs.h"

#include "mmio.h"
#include "handler.h"
#include "csr.h"
#include "hart.h"

#include "rv_plic.h"

#include "dma.h"
#include "dma_regs.h"
#include "fast_intr_ctrl.h"


/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

unsigned int IFFIFO_START_ADDRESS = EXT_PERIPHERAL_START_ADDRESS + 0x2000;

int32_t to_fifo  [6]   __attribute__ ((aligned (4)))  = { 1, 2, 3, 4, 5, 6 };
int32_t from_fifo[4]   __attribute__ ((aligned (4)))  = { 0, 0, 0, 0 };

int8_t dma_intr_flag = 0;
void dma_intr_handler_trans_done()
{
  dma_intr_flag = 1;
}

void protected_wait_for_dma_interrupt(void)
{
  while(!dma_is_ready()) {
    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if (!dma_is_ready()) {
        wait_for_interrupt();
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
  }
}

iffifo_intr_flag = 0;
static void handler_irq_iffifo( uint32_t int_id )
{
  mmio_region_t iffifo_base_addr = mmio_region_from_addr((uintptr_t)IFFIFO_START_ADDRESS);
  mmio_region_write32(iffifo_base_addr, IFFIFO_INTERRUPTS_REG_OFFSET, 0b0);
  iffifo_intr_flag = 1;
  PRINTF(" ** REACH intr. fired.\n");
}

static dma_target_t tgt_src;
static dma_target_t tgt_dst;
static dma_trans_t trans;

int compare_print_fifo_array(void) {
  int errors = 0;
  PRINTF("from_fifo = {");
  for (int i = 0; i < 4; i+=1) {
    PRINTF("%d",from_fifo[i]);
    if(i != 4-1) {PRINTF(", ");};
    if (to_fifo[i]+1 != from_fifo[i]) {++errors;}
  }
  PRINTF("}\n");
  return errors;
}

void print_status_register(void)
{
  mmio_region_t iffifo_base_addr = mmio_region_from_addr((uintptr_t)IFFIFO_START_ADDRESS);
  int32_t status = mmio_region_read32(iffifo_base_addr, IFFIFO_STATUS_REG_OFFSET);
  PRINTF("STATUS = ");
  PRINTF(status & (1 << IFFIFO_STATUS_EMPTY_BIT)     ? "E" : "-"); // FIFO empty
  PRINTF(status & (1 << IFFIFO_STATUS_AVAILABLE_BIT) ? "A" : "-"); // Data available in FIFO
  PRINTF(status & (1 << IFFIFO_STATUS_REACHED_BIT)   ? "R" : "-"); // Watermark reached
  PRINTF(status & (1 << IFFIFO_STATUS_FULL_BIT)      ? "F" : "-"); // FIFO full
  PRINTF("\n");
}

int is_iffifo_full(void)
{
  mmio_region_t iffifo_base_addr = mmio_region_from_addr((uintptr_t)IFFIFO_START_ADDRESS);
  int32_t status = mmio_region_read32(iffifo_base_addr, IFFIFO_STATUS_REG_OFFSET);
  return status & (1 << IFFIFO_STATUS_FULL_BIT);
}

int main(int argc, char *argv[]) {

    mmio_region_t iffifo_base_addr = mmio_region_from_addr((uintptr_t)IFFIFO_START_ADDRESS);
    
    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 11;
    CSR_SET_BITS(CSR_REG_MIE, mask);
    
    if(plic_Init()) {return EXIT_FAILURE;};
    if(plic_irq_set_priority(EXT_INTR_1, 1)) {return EXIT_FAILURE;};
    if(plic_irq_set_enabled(EXT_INTR_1, kPlicToggleEnabled)) {return EXIT_FAILURE;};
    
    plic_assign_external_irq_handler(EXT_INTR_1, &handler_irq_iffifo);
    
    mmio_region_write32(iffifo_base_addr, IFFIFO_WATERMARK_REG_OFFSET, 2);
    mmio_region_write32(iffifo_base_addr, IFFIFO_INTERRUPTS_REG_OFFSET, 0b1);
    
    dma_config_flags_t ret;

     // -- DMA CONFIGURATION --

    dma_init(NULL);
    tgt_src.ptr        = to_fifo;
    tgt_src.inc_du     = 1;
    tgt_src.trig       = DMA_TRIG_MEMORY;
    tgt_src.type       = DMA_DATA_TYPE_WORD;
    tgt_src.size_du    = 6;

    tgt_dst.ptr        = IFFIFO_START_ADDRESS + IFFIFO_FIFO_IN_REG_OFFSET;
    tgt_dst.inc_du     = 0;
    tgt_dst.trig       = DMA_TRIG_SLOT_EXT_TX;
    tgt_dst.type       = DMA_DATA_TYPE_WORD;
    
    trans.src        = &tgt_src;
    trans.dst        = &tgt_dst;
    trans.end        = DMA_TRANS_END_INTR;

    ret = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    if (ret != 0) {return EXIT_FAILURE;}
    ret = dma_load_transaction(&trans);
    if (ret != 0) {return EXIT_FAILURE;}
    
    if (compare_print_fifo_array() != 4) {return EXIT_FAILURE;}
    
    print_status_register();
    
    PRINTF("Launch MM -> Stream DMA\n");
    // Launch a 6-word TX DMA transaction to a 4-word FIFO. The FIFO will be full.
    dma_launch( &trans );
    
    // To terminate the DMA transaction, 2 words must be manually popped from the FIFO.
    while(!is_iffifo_full());
    int32_t read0 = mmio_region_read32(iffifo_base_addr, IFFIFO_FIFO_OUT_REG_OFFSET);
    while(!is_iffifo_full());
    int32_t read1 = mmio_region_read32(iffifo_base_addr, IFFIFO_FIFO_OUT_REG_OFFSET);
    
    print_status_register();
    
    PRINTF("Manual readings: {%d, %d}\n", read0, read1);

    protected_wait_for_dma_interrupt();
    
    dma_init(NULL);
    tgt_src.ptr        = IFFIFO_START_ADDRESS + IFFIFO_FIFO_OUT_REG_OFFSET;
    tgt_src.inc_du     = 0;
    tgt_src.trig       = DMA_TRIG_SLOT_EXT_RX;
    tgt_src.type       = DMA_DATA_TYPE_WORD;
    tgt_src.size_du    = 4;

    tgt_dst.ptr        = from_fifo;
    tgt_dst.inc_du     = 1;
    tgt_dst.trig       = DMA_TRIG_MEMORY;
    tgt_dst.type       = DMA_DATA_TYPE_WORD;

    trans.src        = &tgt_src;
    trans.dst        = &tgt_dst;
    trans.end        = DMA_TRANS_END_INTR;
    
    ret = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    if (ret != 0) {return EXIT_FAILURE;}
    ret = dma_load_transaction(&trans);
    if (ret != 0) {return EXIT_FAILURE;}
    PRINTF("Launch Stream -> MM DMA\n");
    dma_launch( &trans );
    
    protected_wait_for_dma_interrupt();
    
    print_status_register();

    if (compare_print_fifo_array() == 0) {return EXIT_FAILURE;};
    
    if (!iffifo_intr_flag) {return EXIT_FAILURE;};
    
    return EXIT_SUCCESS;
    
}

