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

#include "dma.h"
#include "dma_regs.h"
#include "fast_intr_ctrl.h"


#define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)


int32_t to_fifo  [6]   __attribute__ ((aligned (4)))  = { 1, 2, 3, 4, 5, 6 };
int32_t from_fifo[4]   __attribute__ ((aligned (4)))  = { 0, 0, 0, 0 };

int8_t dma_intr_flag = 0;
void dma_intr_handler_trans_done()
{
  dma_intr_flag = 1;
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

int main(int argc, char *argv[]) {


    unsigned int IFFIFO_START_ADDRESS = EXT_PERIPHERAL_START_ADDRESS + 0x2000;
    mmio_region_t iffifo_base_addr = mmio_region_from_addr((uintptr_t)IFFIFO_START_ADDRESS);
    
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
    
    PRINTF("Launch MM -> Stream DMA\n");
    dma_launch( &trans );
    
    int32_t read0 = mmio_region_read32(iffifo_base_addr, IFFIFO_FIFO_OUT_REG_OFFSET);
    int32_t read1 = mmio_region_read32(iffifo_base_addr, IFFIFO_FIFO_OUT_REG_OFFSET);
    
    PRINTF("Manual readings: {%d, %d}\n", read0, read1);
    
    if (dma_intr_flag == 0) { wait_for_interrupt(); }
    dma_intr_flag = 0;
    
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
    
    wait_for_interrupt(); 

    if (compare_print_fifo_array() == 0) {return EXIT_FAILURE;};
    
    return EXIT_SUCCESS;
    
}

