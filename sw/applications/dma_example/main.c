// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>


#include "dma.h"
#include "core_v_mini_mcu.h"

#include "csr.h"
#include "hart.h"

#include "gpio.h"
#include "mmio.h"


#define TEST_SINGULAR_MODE
#define TEST_PENDING_TRANSACTION
#define TEST_WINDOW 

#define TEST_DATA_SIZE  16
#define TEST_DATA_LARGE 4096
#define TRANSACTIONS_N  5 // Only possible to perform 2 consecutive transactions
#define TEST_WINDOW_SIZE_W 1024//72 // if put at <=71 the isr is too slow to react to the interrupt 
                            // and one will be lost (with size = 1024.)
                            // meaning with the given implementation the isr takes about 78 dma cycles.


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

#ifdef TEST_WINDOW
int32_t window_intr_flag;
gpio_t gpio;

void handler_irq_dma(void) {
    gpio_write(&gpio, 8, true);
    window_intr_flag ++;
    plic_irq_complete();
    PRINTF("w");
    gpio_write(&gpio, 8, false);
}

uint8_t dma_window_ratio_warning_threshold()
{
    return 0;
}

#endif // TEST_WINDOW

void dma_intr_handler()
{
    gpio_write(&gpio, 9, true);
    cycles++;
    PRINTF("D");
    gpio_write(&gpio, 9, false);
}

int main(int argc, char *argv[])
{
    
    static uint32_t test_data_4B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = {
      0x76543210, 0xfedcba98, 0x579a6f90, 0x657d5bee, 0x758ee41f, 0x01234567, 0xfedbca98, 0x89abcdef, 0x679852fe, 0xff8252bb, 0x763b4521, 0x6875adaa, 0x09ac65bb, 0x666ba334, 0x55446677, 0x65ffba98};
    static uint32_t copied_data_4B[TEST_DATA_LARGE] __attribute__ ((aligned (4))) = { 0 };
    static uint32_t test_data_large[TEST_DATA_LARGE] __attribute__ ((aligned (4))) = { 0 };

    enable_all_fast_interrupts(true); // not needed is default - done on reset


    PRINTF("DMA test app\n\r");
    
    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8 ); 
    
    // The DMA is initialized (i.e. the base address is computed  )
    dma_init();
    
    dma_config_flags_t res;
    
    static dma_target_t tgt_src = {
                                .ptr        = test_data_4B,
                                .inc_du     = 1,
                                .size_du    = TEST_DATA_SIZE,
                                .trig       = DMA_TRIG_MEMORY,
                                .type       = DMA_DATA_TYPE_WORD,
                                };
    static dma_target_t tgt_dst = {
                                .ptr        = copied_data_4B,
                                .inc_du     = 1,
                                .size_du    = TEST_DATA_SIZE,
                                .trig       = DMA_TRIG_MEMORY,
                                .type       = DMA_DATA_TYPE_WORD,
                                };
    static dma_trans_t trans = {
                                .src        = &tgt_src,
                                .dst        = &tgt_dst,
                                .mode       = DMA_TRANS_MODE_SINGLE,
                                .win_b      = 0,
                                .end        = DMA_TRANS_END_INTR,
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
    
    
    for(uint32_t i = 0; i < TEST_DATA_SIZE; i++ ) {
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


    tgt_src.ptr     = test_data_large;
    tgt_src.size_du = TEST_DATA_LARGE;

    // trans.end = DMA_TRANS_END_INTR_WAIT; // This option makes no sense, because the launch is blocking the program until the trans finishes. 
    trans.end = DMA_TRANS_END_INTR;
    // trans.end = DMA_TRANS_END_POLLING; 


    res = dma_create_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("tran: %u \n\r", res);
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \n\r", res);

    cycles = 0;
    uint8_t consecutive_trans = 0;

    for(  uint8_t i = 0; i < TRANSACTIONS_N; i++ ){
        res = dma_launch(&trans);
        if( res == DMA_CONFIG_OK ) consecutive_trans++;
    }
    
    if( trans.end == DMA_TRANS_END_POLLING ){
        while( cycles < consecutive_trans ){
            while( ! dma_is_ready() );
            cycles++;
        }
    } else {
        while( cycles < consecutive_trans ){
            wait_for_interrupt();
        }
    }
    PRINTF(">> Finished %d transactions. \n\r", consecutive_trans);
    
    
    for(int i=0; i<TEST_DATA_LARGE; i++) {
        if (tgt_src.ptr[i] != tgt_dst.ptr[i]) {
            PRINTF("ERROR COPY [%d]: %08x != %08x : %04x != %04x\n\r", i, &tgt_src.ptr[i], &tgt_dst.ptr[i], tgt_src.ptr[i], tgt_dst.ptr[i]);
            errors++;
        }
    }

    if (errors == 0) {
        PRINTF("DMA word transfer success\nFinished! :) \n\r");
        PRINTF("DMA successfully processed %d consecutive transactions\n", consecutive_trans );
    } else {
        PRINTF("DMA word transfer failure: %d errors out of %d words checked\n\r", errors, TEST_DATA_SIZE);
        PRINTF("DMA couldn't manage consecutive transactions. \n");
    }

#endif // TEST_PENDING_TRANSACTION


#ifdef TEST_WINDOW

    PRINTF("\n\n===================================\n\n");
    PRINTF("    TESTING WINDOW INTERRUPT   ");
    PRINTF("\n\n===================================\n\n");

    
    gpio_params_t gpio_params;
    gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_START_ADDRESS);
    gpio_init(gpio_params, &gpio);
    gpio_output_set_enabled(&gpio, 8, true);
    gpio_write(&gpio, 8, false); 
    gpio_output_set_enabled(&gpio, 9, true);
    gpio_write(&gpio, 9, false);
    gpio_output_set_enabled(&gpio, 11, true);
    gpio_write(&gpio, 11, false); 

    window_intr_flag = 0;

    for (uint32_t i = 0; i < TEST_DATA_LARGE; i++) {
        test_data_large [i] = i;
        copied_data_4B  [i] = 0;
    }

    tgt_src.ptr     = test_data_large;
    tgt_src.size_du = TEST_DATA_LARGE;

    tgt_src.type    = DMA_DATA_TYPE_WORD;
    tgt_dst.type    = DMA_DATA_TYPE_WORD;

    trans.win_b     = TEST_WINDOW_SIZE_W;
    trans.end       = DMA_TRANS_END_INTR;
    
    res = dma_create_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("tran: %u \n\r", res);
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \n\r", res);

    dma_launch(&trans);

    if( trans.end == DMA_TRANS_END_POLLING ){ //There will be no interrupts whatsoever!
        while( ! dma_is_ready() );
        printf("?");
    } else {
        while( !dma_is_ready() ){
            wait_for_interrupt();
            printf("i");
        }
    }  

    gpio_write(&gpio, 11, true);
    cycles--;
    gpio_write(&gpio, 11, false);

    PRINTF("\nWe had %d window interrupts.\n", window_intr_flag);

    for(uint32_t i = 0; i < TEST_DATA_LARGE; i++ ) {
        if (copied_data_4B[i] != test_data_large[i]) {
            PRINTF("[%d] %04x\tvs.\t%04x\n\r", i, copied_data_4B[i], test_data_large[i]);
            errors++;
        }
    }

    if (errors == 0) {
        PRINTF("DMA word transfer success\nFinished! :) \n\r");
    } else {
        PRINTF("DMA word transfer failure: %d errors out of %d words checked\n\r", errors, TEST_DATA_SIZE);
    }

/*
    uint32_t status;
    do {
        status = mmio_region_read32(dma.base_addr, DMA_STATUS_REG_OFFSET);
        // wait for done - ISR done should be disabled.
    } while((status & (1 << DMA_STATUS_READY_BIT)) == 0);

    if (status & (1 << DMA_STATUS_WINDOW_DONE_BIT) == 0) {
        PRINTF("[E] DMA window done flag not raised\r\n");
        errors += 1;
    }
    if (dma_get_halfway(&dma)) { 
        // should be clean on read so rereading should be 0
        PRINTF("[E] DMA window done flag not reset on status read\r\n");
        errors += 1;
    }

    if (dma_intr_flag == 1) {
        PRINTF("[E] DMA window test failed DONE interrupt was triggered\n");
        errors += 1;
    }

    uint32_t window_count = mmio_region_read32(dma.base_addr, DMA_WINDOW_COUNT_REG_OFFSET);

    if (window_intr_flag != TEST_WINDOW_DATA_SIZE / TEST_WINDOW_SIZE) {
        PRINTF("[E] DMA window test failed ISR wasn't trigger the right number %d != %d\r\n", window_intr_flag, TEST_WINDOW_DATA_SIZE / TEST_WINDOW_SIZE);
        errors += 1;
    }
    
    if (window_count != TEST_WINDOW_DATA_SIZE / TEST_WINDOW_SIZE) {
        PRINTF("[E] DMA window test failed Window count register is wrong %d != %d\r\n", window_count, TEST_WINDOW_DATA_SIZE / TEST_WINDOW_SIZE);
        errors += 1;
    }
    if (!errors) {
        PRINTF("DMA window count is okay (#%d * %d)\r\n", TEST_WINDOW_DATA_SIZE / TEST_WINDOW_SIZE, TEST_WINDOW_SIZE);
    }
    else {
        PRINTF("F-DMA window test with %d errors\r\n", error);
    }
    */
#endif // TEST_WINDOW

    return EXIT_SUCCESS;
}
