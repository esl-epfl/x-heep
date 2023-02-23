// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>

#include "csr.h"

#include "core_v_mini_mcu.h"
#include "dma.h"

// TODO
// - Add offset at the begining and the end and check

// Choose which scenarios to test
#define TEST_WORD
//#define TEST_HALF_WORD
//#define TEST_BYTE

#define HALF_WORD_INPUT_OFFSET 0
#define HALF_WORD_OUTPUT_OFFSET 1 // Applied at begining and end of the output vector, which should not be overwriten.
#define BYTE_INPUT_OFFSET 1
#define BYTE_OUTPUT_OFFSET 3 // Applied at begining and end of the output vector, which should not be overwriten.

#define TEST_DATA_SIZE 16

// Source and destination addresses have to be aligned on a 4 bytes address
//uint16_t* test_data_2B = test_data_4B;
//uint8_t* test_data_1B = test_data_4B;

//uint16_t copied_data_2B[TEST_DATA_SIZE] __attribute__ ((aligned (2))) = { 0 };
//uint8_t copied_data_1B[TEST_DATA_SIZE] = { 0 };


void dma_intr_handler()
{
    printf("This is not a weak implementation.\n");
}

int main(int argc, char *argv[])
{
    
    static uint32_t test_data_4B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = {
      0x76543210, 0xfedcba98, 0x579a6f90, 0x657d5bee, 0x758ee41f, 0x01234567, 0xfedbca98, 0x89abcdef, 0x679852fe, 0xff8252bb, 0x763b4521, 0x6875adaa, 0x09ac65bb, 0x666ba334, 0x55446677, 0x65ffba98};
    static uint32_t copied_data_4B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = { 0 };
    
    printf("DMA test app: 3\n\r");
    printf("no environments\n\r");

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level fast dma interrupt
    const uint32_t mask = 1 << 19;
    CSR_SET_BITS(CSR_REG_MIE, mask);
    
    // The DMA is initialized (i.e. the base address is computed  )
    printf("About to init.\n\r");
    dma_init();
    printf("Done with init.\n\r");
    
    dma_config_flags_t ret;
    
    static dma_target_t tgt1;
    static dma_target_t tgt2;
    static dma_trans_t trans;
    // Create a target pointing at the buffer to be copied. Whole WORDs, no skippings, in memory, no environment.  
    ret = dma_create_target( &tgt1, test_data_4B, 1, TEST_DATA_SIZE,  DMA_DATA_TYPE_WORD, DMA_SMPH_MEMORY, NULL, DMA_SAFETY_SANITY_CHECKS | DMA_SAFETY_INTEGRITY_CHECKS);
    ret = dma_create_target( &tgt2, copied_data_4B, 1, TEST_DATA_SIZE,  DMA_DATA_TYPE_WORD, DMA_SMPH_MEMORY, NULL, DMA_SAFETY_SANITY_CHECKS | DMA_SAFETY_INTEGRITY_CHECKS);
    ret = dma_create_transaction( &trans, &tgt1, &tgt2, DMA_ALLOW_REALIGN, DMA_SAFETY_SANITY_CHECKS | DMA_SAFETY_INTEGRITY_CHECKS );
    ret = dma_load_transaction(&trans);

    ret = dma_launch(&trans);

    printf(">> Finished transaction. \n\r");

    int32_t errors;

    errors=0;
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
