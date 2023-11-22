// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>


#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "rv_plic.h"
#include "iffifo_regs.h"

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

#if TARGET_PYNQ_Z2
#error This example requires the IFFIFO peripheral, instantiated in the test bench only.
#endif

unsigned int IFFIFO_START_ADDRESS = EXT_PERIPHERAL_START_ADDRESS + 0x2000;

int32_t src     [4]   __attribute__ ((aligned (4)))  = { 2, 7, 3, 12 };
int32_t dst     [4]   __attribute__ ((aligned (4)))  = { 0, 0, 0, 0 };
int32_t dst_bcst[4]   __attribute__ ((aligned (4)))  = { 0, 0, 0, 0 };

static dma_target_t tgt_src;
static dma_target_t tgt_dst;
static dma_target_t tgt_dst_bcst;
static dma_trans_t trans;

void print_fifo_array(void) {

  PRINTF("src = {");
  for (int i = 0; i < 4; i+=1) {
    PRINTF("%d",src[i]);
    if(i != 4-1) {PRINTF(", ");};
  }
  PRINTF("}\n");
  
  PRINTF("dst = {");
  for (int i = 0; i < 4; i+=1) {
    PRINTF("%d",dst[i]);
    if(i != 4-1) {PRINTF(", ");};
  }
  PRINTF("}\n");
  
  PRINTF("dst_bcst = {");
  for (int i = 0; i < 4; i+=1) {
    PRINTF("%d",dst_bcst[i]);
    if(i != 4-1) {PRINTF(", ");};
  }
  PRINTF("}\n");

}

int main(int argc, char *argv[])
{

    dma_init(NULL);
    tgt_src.ptr         = src;
    tgt_src.inc_du      = 1;
    tgt_src.trig        = DMA_TRIG_MEMORY;
    tgt_src.type        = DMA_DATA_TYPE_WORD;
    tgt_src.size_du     = 4;

    tgt_dst.ptr         = dst;
    tgt_dst.inc_du      = 1;
    tgt_dst.trig        = DMA_TRIG_MEMORY;
    tgt_dst.type        = DMA_DATA_TYPE_WORD;
    
    tgt_dst_bcst.ptr    = IFFIFO_START_ADDRESS + IFFIFO_FIFO_IN_REG_OFFSET;
    tgt_dst_bcst.inc_du = 0;
    tgt_dst_bcst.trig   = DMA_TRIG_SLOT_EXT_TX;
    tgt_dst_bcst.type   = DMA_DATA_TYPE_WORD;
    
    trans.src      = &tgt_src;
    trans.dst      = &tgt_dst;
    trans.dst_bcst = &tgt_dst_bcst;
    trans.mode     = DMA_TRANS_MODE_BROADCAST;
    trans.end      = DMA_TRANS_END_INTR;

    if (dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY )) {return EXIT_FAILURE;}
    if (dma_load_transaction(&trans)) {return EXIT_FAILURE;}
    
    print_fifo_array();
    
    PRINTF("Launch DMA SRC > DST >> IFFIFO transaction\n");
    if (dma_launch( &trans )) {return EXIT_FAILURE;}
    
    print_fifo_array();
    
    dma_init(NULL);
    tgt_src.ptr        = IFFIFO_START_ADDRESS + IFFIFO_FIFO_OUT_REG_OFFSET;
    tgt_src.inc_du     = 0;
    tgt_src.trig       = DMA_TRIG_SLOT_EXT_RX;
    tgt_src.type       = DMA_DATA_TYPE_WORD;
    tgt_src.size_du    = 4;

    tgt_dst.ptr        = dst_bcst;
    tgt_dst.inc_du     = 1;
    tgt_dst.trig       = DMA_TRIG_MEMORY;
    tgt_dst.type       = DMA_DATA_TYPE_WORD;

    trans.src        = &tgt_src;
    trans.dst        = &tgt_dst;
    trans.end        = DMA_TRANS_END_INTR;
    
    if (dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY )) {return EXIT_FAILURE;}
    if (dma_load_transaction(&trans)) {return EXIT_FAILURE;}
    
    PRINTF("Launch IFFIFO > DST_BCST transaction\n");
    if (dma_launch( &trans )) {return EXIT_FAILURE;}
    
    print_fifo_array();

    return EXIT_SUCCESS;
}
