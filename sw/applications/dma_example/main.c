// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>


#include "dma.h"
#include "core_v_mini_mcu.h"
#include "csr.h"
#include "hart.h"



#define TEST_SINGULAR_MODE
//#define TEST_PENDING_TRANSACTION
//#define TEST_WINDOW 


#define TEST_DATA_SIZE 16
#define TEST_DATA_LARGE 4096


#define DEBUG

// Use PRINTF instead of PRINTF to remove print by default
#ifdef DEBUG
  #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
  #define PRINTF(...)
#endif // DEBUG

#define PRINTF2(fmt, ...)    printf(fmt, ## __VA_ARGS__)


int32_t errors = 0;
int8_t cycles = 0;

void dma_intr_handler()
{
    cycles++;
    PRINTF("#");
}


#ifdef TEST_WINDOW
int32_t external_intr_flag;

// Interrupt controller variables
dif_plic_params_t rv_plic_params;
dif_plic_t rv_plic;
dif_plic_result_t plic_res;
dif_plic_irq_id_t intr_num;

void handler_irq_external(void) {
    // Claim/clear interrupt
    plic_res = dif_plic_irq_claim(&rv_plic, 0, &intr_num);
    if (plic_res == kDifPlicOk && intr_num == DMA_WINDOW_INTR) {
        external_intr_flag += 1;
    }
    dif_plic_irq_complete(&rv_plic, 0, &intr_num); // complete in any case
}
#endif // TEST_WINDOW


int main(int argc, char *argv[])
{
    
    static uint32_t test_data_4B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = {
      0x76543210, 0xfedcba98, 0x579a6f90, 0x657d5bee, 0x758ee41f, 0x01234567, 0xfedbca98, 0x89abcdef, 0x679852fe, 0xff8252bb, 0x763b4521, 0x6875adaa, 0x09ac65bb, 0x666ba334, 0x55446677, 0x65ffba98};
    static uint32_t copied_data_4B[TEST_DATA_LARGE] __attribute__ ((aligned (4))) = { 0 };
    static uint32_t test_data_large[TEST_DATA_LARGE] __attribute__ ((aligned (4))) = { 0 };
    
#ifdef TEST_CIRCULAR_MODE
    static uint32_t test_data_circular[TEST_DATA_CIRCULAR] __attribute__ ((aligned (4))) = { 1 };
#endif //TEST_CIRCULAR_MODE

    PRINTF("DMA test app: 4\n\r");
    
    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8 ); 
    
    // The DMA is initialized (i.e. the base address is computed  )
    PRINTF("About to init.\n\r");
    dma_init();
    PRINTF("Init finished.\n\r");
    
    dma_config_flags_t res;
    
    static dma_target_t tgt_src = {
                                .ptr = test_data_4B,
                                .inc_du = 1,
                                .size_du = TEST_DATA_SIZE,
                                .trig = DMA_TRIG_MEMORY,
                                .type = DMA_DATA_TYPE_WORD,
                                };
    static dma_target_t tgt_dst = {
                                .ptr = copied_data_4B,
                                .inc_du = 1,
                                .size_du = TEST_DATA_SIZE,
                                .trig = DMA_TRIG_MEMORY,
                                .type = DMA_DATA_TYPE_WORD,
                                };
    static dma_trans_t trans = {
                                .src = &tgt_src,
                                .dst = &tgt_dst,
                                .mode = DMA_TRANS_MODE_SINGLE,
                                .win_b = 0,
                                .end = DMA_TRANS_END_INTR,
                                };
    // Create a target pointing at the buffer to be copied. Whole WORDs, no skippings, in memory, no environment.  

#ifdef TEST_SINGULAR_MODE

    PRINTF("\n\n===================================\n\n");
    PRINTF("    TESTING SINGULAR MODE   ");
    PRINTF("\n\n===================================\n\n");

    res = dma_create_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("tran: %u \n\r", res);
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \n\r", res);
    res = dma_launch(&trans);
    PRINTF("laun: %u \n\r", res);
    
    while( ! dma_is_ready() ){
        wait_for_interrupt();
    }
    PRINTF(">> Finished transaction. \n\r");
    
    
    for(int i=0; i<TEST_DATA_SIZE; i++) {
        if (copied_data_4B[i] != test_data_4B[i]) {
            PRINTF("ERROR COPY [%d]: %08x != %08x : %04x != %04x\n\r", i, &copied_data_4B[i], &test_data_4B[i], copied_data_4B[i], test_data_4B[i]);
            errors++;
        }
    }

    if (errors == 0) {
        PRINTF("DMA word transfer success\nFinished! :) \n\r");
    } else {
        PRINTF("DMA word transfer failure: %d errors out of %d words checked\n\r", errors, TEST_DATA_SIZE);
    }

#endif // TEST_SINGULAR_MODE


#ifdef TEST_PENDING_TRANSACTION
        PRINTF("\n\n===================================\n\n");
        PRINTF("    TESTING MULTIPLE TRANSACTIONS   ");
        PRINTF("\n\n===================================\n\n");
        
        for (uint32_t i = 0; i < TEST_DATA_LARGE; i++) {
            test_data_large[i] = i;
        }


        cycles = 0;

        tgt_src.ptr     = test_data_large;
        tgt_src.size_du = TEST_DATA_LARGE;


        res = dma_create_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
        PRINTF("tran: %u \n\r", res);
        res = dma_load_transaction(&trans);
        PRINTF("load: %u \n\r", res);
        res = dma_launch(&trans);
        //PRINTF("laun #1: %u \n\r", res);
        res = dma_launch(&trans);
        //PRINTF("laun #2: %u \n\r", res);
        
        while( cycles < 2 ){
            wait_for_interrupt();
        }
        PRINTF(">> Finished %d transactions. \n\r", cycles);
        
        
        for(int i=0; i<TEST_DATA_LARGE; i++) {
            if (tgt_src.ptr[i] != tgt_dst.ptr[i]) {
                PRINTF("ERROR COPY [%d]: %08x != %08x : %04x != %04x\n\r", i, &tgt_src.ptr[i], &tgt_dst.ptr[i], tgt_src.ptr[i], tgt_dst.ptr[i]);
                errors++;
            }
        }

        if (errors == 0) {
            PRINTF("DMA word transfer success\nFinished! :) \n\r");
            PRINTF("DMA couldn't manage two consecutive transactions. \n");
        } else {
            PRINTF("DMA word transfer failure: %d errors out of %d words checked\n\r", errors, TEST_DATA_SIZE);
            PRINTF("DMA successfully processed two consecutive transactions\n");
        }

#endif // TEST_PENDING_TRANSACTION


#ifdef TEST_WINDOW

        rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)RV_PLIC_START_ADDRESS);
        dif_plic_init(rv_plic_params, &rv_plic);
        dif_plic_irq_set_priority(&rv_plic, DMA_WINDOW_INTR, 1);
        dif_plic_irq_set_enabled(&rv_plic, DMA_WINDOW_INTR, 0, kDifPlicToggleEnabled);
        external_intr_flag = 0;

        dma_enable_intr(&dma, false, true);
    
        // -- DMA CONFIG -- //
        dma_set_read_ptr(&dma, (uint32_t) test_window_data1);
        dma_set_write_ptr(&dma, (uint32_t) test_window_data2);
        dma_set_ptr_inc(&dma, 1, 1);
        dma_set_slot(&dma, 0, 0);
        dma_set_data_type(&dma, (uint32_t) DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_8BIT_WORD);
        // Give number of bytes to transfer
        dma_intr_flag = 0;
        mmio_region_write32(dma.base_addr,  DMA_WINDOW_SIZE_REG_OFFSET, TEST_WINDOW_SIZE);
        dma_set_cnt_start(&dma, TEST_WINDOW_DATA_SIZE);

        uint8_t error = 0;

        uint32_t status;
        do {
            status = mmio_region_read32(dma.base_addr, DMA_STATUS_REG_OFFSET);
            // wait for done - ISR done should be disabled.
        } while((status & (1 << DMA_STATUS_READY_BIT)) == 0);

        if (status & (1 << DMA_STATUS_WINDOW_DONE_BIT) == 0) {
            PRINTF("[E] DMA window done flag not raised\r\n");
            error += 1;
        }
        if (dma_get_halfway(&dma)) { 
            // should be clean on read so rereading should be 0
            PRINTF("[E] DMA window done flag not reset on status read\r\n");
            error += 1;
        }

        if (dma_intr_flag == 1) {
            PRINTF("[E] DMA window test failed DONE interrupt was triggered\n");
            error += 1;
        }

        uint32_t window_count = mmio_region_read32(dma.base_addr, DMA_WINDOW_COUNT_REG_OFFSET);

        if (external_intr_flag != TEST_WINDOW_DATA_SIZE / TEST_WINDOW_SIZE) {
            PRINTF("[E] DMA window test failed ISR wasn't trigger the right number %d != %d\r\n", external_intr_flag, TEST_WINDOW_DATA_SIZE / TEST_WINDOW_SIZE);
            error += 1;
        }
        
        if (window_count != TEST_WINDOW_DATA_SIZE / TEST_WINDOW_SIZE) {
            PRINTF("[E] DMA window test failed Window count register is wrong %d != %d\r\n", window_count, TEST_WINDOW_DATA_SIZE / TEST_WINDOW_SIZE);
            error += 1;
        }
        if (!error) {
            PRINTF("DMA window count is okay (#%d * %d)\r\n", TEST_WINDOW_DATA_SIZE / TEST_WINDOW_SIZE, TEST_WINDOW_SIZE);
        }
        else {
            PRINTF("F-DMA window test with %d errors\r\n", error);
        }
    
#endif // TEST_WINDOW

    return EXIT_SUCCESS;
}
