// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>


#include "dma.h"
#include "core_v_mini_mcu.h"
#include "csr.h"

#define TEST_DATA_SIZE 16

void dma_intr_handler()
{
    printf("This is not a weak implementation.\n\r");
}

int main(int argc, char *argv[])
{
    
    static uint32_t test_data_4B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = {
      0x76543210, 0xfedcba98, 0x579a6f90, 0x657d5bee, 0x758ee41f, 0x01234567, 0xfedbca98, 0x89abcdef, 0x679852fe, 0xff8252bb, 0x763b4521, 0x6875adaa, 0x09ac65bb, 0x666ba334, 0x55446677, 0x65ffba98};
    static uint32_t copied_data_4B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = { 0 };
    
    printf("DMA test app: 4\n\r");
    
    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8 ); 
    
    // The DMA is initialized (i.e. the base address is computed  )
    printf("About to init.\n\r");
    dma_init();
    printf("Init finished.\n\r");
    
    dma_config_flags_t res;
    
    static dma_target_t tgt1 = {
                                .ptr = test_data_4B,
                                .inc_du = 1,
                                .size_du = TEST_DATA_SIZE,
                                .smph = DMA_SMPH_MEMORY,
                                .type = DMA_DATA_TYPE_WORD,
                                };
    static dma_target_t tgt2 = {
                                .ptr = copied_data_4B,
                                .inc_du = 1,
                                .size_du = TEST_DATA_SIZE,
                                .smph = DMA_SMPH_MEMORY,
                                .type = DMA_DATA_TYPE_WORD,
                                };
    static dma_trans_t trans = {
                                .src = &tgt1,
                                .dst = &tgt2,
                                .end = DMA_END_EVENT_INTR,
                                };
    // Create a target pointing at the buffer to be copied. Whole WORDs, no skippings, in memory, no environment.  
    
    res = dma_create_transaction( &trans, DMA_ALLOW_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    printf("tran: %u \n\r", res);
    
    res = dma_load_transaction(&trans);
    printf("load: %u \n\r", res);
    res = dma_launch(&trans);
    printf("laun: %u \n\r", res);
    printf(">> Finished transaction launch. \n\r");
    
    while( ! dma_is_done() ){}
    printf(">> Finished transaction. \n\r");
    
    int32_t errors = 0;
    for(int i=0; i<TEST_DATA_SIZE; i++) {
        if (copied_data_4B[i] != test_data_4B[i]) {
            printf("ERROR COPY [%d]: %08x != %08x : %04x != %04x\n\r", i, &copied_data_4B[i], &test_data_4B[i], copied_data_4B[i], test_data_4B[i]);
            errors++;
        }
    }

    if (errors == 0) {
        printf("DMA word transfer success\nFinished! :) \n\r");
    } else {
        printf("DMA word transfer failure: %d errors out of %d words checked\n\r", errors, TEST_DATA_SIZE);
    }
    return EXIT_SUCCESS;
}
