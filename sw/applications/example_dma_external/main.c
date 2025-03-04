// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>


#include "dma.h"
#include "core_v_mini_mcu.h"
#include "csr.h"
#include "x-heep.h"

#define TEST_DATA_SIZE      16
#define TEST_DATA_LARGE     1024

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif


int32_t errors = 0;

void dma_intr_handler_trans_done(uint8_t channel)
{
    PRINTF("D");
}

int main(int argc, char *argv[])
{

    static uint32_t test_data[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = {
      0x76543210, 0xfedcba98, 0x579a6f90, 0x657d5bee, 0x758ee41f, 0x01234567, 0xfedbca98, 0x89abcdef, 0x679852fe, 0xff8252bb, 0x763b4521, 0x6875adaa, 0x09ac65bb, 0x666ba334, 0x55446677, 0x65ffba98};
    static uint32_t copied_data[TEST_DATA_LARGE] __attribute__ ((aligned (4))) = { 0 };

    // The DMA is initialized (i.e. Any current transaction is cleaned.)

    volatile static dma *peri =  EXT_PERIPHERAL_START_ADDRESS;

    dma_init(peri);

    dma_config_flags_t res;

    static dma_target_t tgt_src = {
                                .ptr        = test_data,
                                .inc_d1_du     = 1,
                                .trig       = DMA_TRIG_MEMORY,
                                .type       = DMA_DATA_TYPE_WORD,
                                };
    static dma_target_t tgt_dst = {
                                .ptr        = copied_data,
                                .inc_d1_du     = 1,
                                .trig       = DMA_TRIG_MEMORY,
                                };
    static dma_trans_t trans = {
                                .src        = &tgt_src,
                                .dst        = &tgt_dst,
                                .size_d1_du    = TEST_DATA_SIZE,
                                .mode       = DMA_TRANS_MODE_SINGLE,
                                .win_du      = 0,
                                .end        = DMA_TRANS_END_INTR,
                                };
    // Create a target pointing at the buffer to be copied. Whole WORDs, no skippings, in memory, no environment.

    PRINTF("\n\n\r===================================\n\n\r");
    PRINTF(" TESTING DMA ON EXTERNAL PERIPHERAL   ");
    PRINTF("\n\n\r===================================\n\n\r");

    res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_launch(&trans);
    PRINTF("laun: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");

    while( ! dma_is_ready(0) ){
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
            //from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }
    PRINTF(">> Finished transaction. \n\r");

    for(uint32_t i = 0; i < trans.size_d1_du; i++ ) {
        if ( ((uint8_t*)copied_data)[i] != ((uint8_t*)test_data)[i] ) {
            PRINTF("ERROR [%d]: %04x != %04x\n\r", i, ((uint8_t*)copied_data)[i], ((uint8_t*)test_data)[i]);
            errors++;
        }
    }

    if (errors == 0) {
        PRINTF("External DMA success\n\r");
        return EXIT_SUCCESS;
    } else {
        PRINTF("External DMA failure: %d errors out of %d elements checked\n\r", errors, trans.size_d1_du );
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
