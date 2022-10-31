// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>

#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "dma.h"
#include "fast_intr_ctrl.h"
#include "fast_intr_ctrl_regs.h"

// TODO
// - Add offset at the begining and the end and check

// Choose which scenarios to test
#define TEST_WORD
#define TEST_HALF_WORD
#define TEST_BYTE

#define HALF_WORD_INPUT_OFFSET 0
#define HALF_WORD_OUTPUT_OFFSET 1 // Applied at begining and end of the output vector, which should not be overwriten.
#define BYTE_INPUT_OFFSET 1
#define BYTE_OUTPUT_OFFSET 3 // Applied at begining and end of the output vector, which should not be overwriten.

#define TEST_DATA_SIZE 16

// Source and destination addresses have to be aligned on a 4 bytes address
uint32_t test_data_4B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = {
  0x76543210, 0xfedcba98, 0x579a6f90, 0x657d5bee, 0x758ee41f, 0x01234567, 0xfedbca98, 0x89abcdef, 0x679852fe, 0xff8252bb, 0x763b4521, 0x6875adaa, 0x09ac65bb, 0x666ba334, 0x55446677, 0x65ffba98};
uint16_t* test_data_2B = test_data_4B;
uint8_t* test_data_1B = test_data_4B;

uint32_t copied_data_4B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = { 0 };
uint16_t copied_data_2B[TEST_DATA_SIZE] __attribute__ ((aligned (2))) = { 0 };
uint8_t copied_data_1B[TEST_DATA_SIZE] = { 0 };

int8_t dma_intr_flag;

void handler_irq_fast_dma(void)
{
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kDma_fic_e);

    dma_intr_flag = 1;
}

int main(int argc, char *argv[])
{
    printf("--- DMA EXAMPLE ---\n");

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level fast dma interrupt
    const uint32_t mask = 1 << 19;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    // dma peripheral structure to access the registers
    dma_t dma;
    dma.base_addr = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);

    #ifdef TEST_WORD
        // -- DMA CONFIG -- //
        dma_set_read_ptr(&dma, (uint32_t) test_data_4B);
        dma_set_write_ptr(&dma, (uint32_t) copied_data_4B);
        dma_set_read_ptr_inc(&dma, (uint32_t) 4);
        dma_set_write_ptr_inc(&dma, (uint32_t) 4);
        dma_set_spi_mode(&dma, (uint32_t) 0);
        dma_set_data_type(&dma, (uint32_t) 0);
        printf("DMA word transaction launched\n");
        // Give number of bytes to transfer
        dma_set_cnt_start(&dma, (uint32_t) TEST_DATA_SIZE*sizeof(*copied_data_4B));
        // Wait copy is done
        dma_intr_flag = 0;
        while(dma_intr_flag==0) {
            wait_for_interrupt();
        }
    #endif // TEST_WORD

    #ifdef TEST_HALF_WORD
        // -- DMA CONFIG -- //
        dma_set_read_ptr(&dma, (uint32_t) (test_data_2B + HALF_WORD_INPUT_OFFSET));
        dma_set_write_ptr(&dma, (uint32_t) (copied_data_2B + HALF_WORD_OUTPUT_OFFSET));
        dma_set_read_ptr_inc(&dma, (uint32_t) 2);
        dma_set_write_ptr_inc(&dma, (uint32_t) 2);
        dma_set_spi_mode(&dma, (uint32_t) 0);
        dma_set_data_type(&dma, (uint32_t) 1);
        printf("DMA half-word transaction launched\n");
        // Give number of bytes to transfer
        // Last 2 bytes are not copy to check the DMA works properly
        dma_set_cnt_start(&dma, (uint32_t) ((TEST_DATA_SIZE - 2*HALF_WORD_OUTPUT_OFFSET)*sizeof(*copied_data_2B)));
        // Wait copy is done
        dma_intr_flag = 0;
        while(dma_intr_flag==0) {
            wait_for_interrupt();
        }
    #endif // TEST_HALF_WORD

    #ifdef TEST_BYTE
        // -- DMA CONFIG -- //
        dma_set_read_ptr(&dma, (uint32_t) test_data_1B + BYTE_INPUT_OFFSET);
        dma_set_write_ptr(&dma, (uint32_t) (copied_data_1B + BYTE_OUTPUT_OFFSET));
        dma_set_read_ptr_inc(&dma, (uint32_t) 1);
        dma_set_write_ptr_inc(&dma, (uint32_t) 1);
        dma_set_spi_mode(&dma, (uint32_t) 0);
        dma_set_data_type(&dma, (uint32_t) 2);
        printf("DMA byte transaction launched\n");
        // Give number of bytes to transfer
        // Last byte are not copy to check the DMA works properly
        dma_set_cnt_start(&dma, (uint32_t) ((TEST_DATA_SIZE - 2*BYTE_OUTPUT_OFFSET)*sizeof(*copied_data_1B)));
        // Wait copy is done
        dma_intr_flag = 0;
        while(dma_intr_flag==0) {
            wait_for_interrupt();
        }
    #endif // TEST_BYTE

    int32_t errors;

    #ifdef TEST_WORD
        errors=0;
        for(int i=0; i<TEST_DATA_SIZE; i++) {
            if (copied_data_4B[i] != test_data_4B[i]) {
                printf("ERROR COPY [%d]: %08x != %08x : %04x != %04x\n", i, &copied_data_4B[i], &test_data_4B[i], copied_data_4B[i], test_data_4B[i]);
                errors++;
            }
        }

        if (errors == 0) {
            printf("DMA word transfer success\n");
        } else {
            printf("DMA word transfer failure: %d errors out of %d words checked\n", errors, TEST_DATA_SIZE);
        }
    #endif // TEST_WORD

    #ifdef TEST_HALF_WORD

        errors = 0;
        for (int i=0; i<(TEST_DATA_SIZE - 2*HALF_WORD_OUTPUT_OFFSET); i++) {
            if(test_data_2B[i + HALF_WORD_INPUT_OFFSET] != copied_data_2B[i + HALF_WORD_OUTPUT_OFFSET]) {
                printf("ERROR COPY [%d]: @%08x-@%08x : %04x != %04x\n" , i , &test_data_2B[i + HALF_WORD_INPUT_OFFSET] , &copied_data_2B[i + HALF_WORD_OUTPUT_OFFSET], test_data_2B[i + HALF_WORD_INPUT_OFFSET], copied_data_2B[i + HALF_WORD_OUTPUT_OFFSET]);
                errors++;
            }
        }
        // Check that the begining and end values of output vector are not overwriten
        for (int i=0; i<HALF_WORD_OUTPUT_OFFSET; i++) {
            if(copied_data_2B[i] != 0) {
                printf("Data Overwritten @%08x : %04x != 0\n" , &copied_data_2B[i], copied_data_2B[i]);
                errors++;
            }
            if(copied_data_2B[TEST_DATA_SIZE - 1 - i] != 0) {
                printf("Data Overwritten @%08x : %04x != 0\n" , &copied_data_2B[TEST_DATA_SIZE - 1 - i], copied_data_2B[TEST_DATA_SIZE - 1 - i]);
                errors++;
            }
        }

        if (errors == 0) {
            printf("DMA half-word transfer success\n");
        } else {
            printf("DMA half-word transfer failure: %d errors out of %d half-words checked\n", errors, TEST_DATA_SIZE);
        }
    #endif // TEST_HALF_WORD

    #ifdef TEST_BYTE
        errors = 0;
        for (int i=0; i<(TEST_DATA_SIZE - 2*BYTE_OUTPUT_OFFSET); i++) {
            if(test_data_1B[i + BYTE_INPUT_OFFSET] != copied_data_1B[i + BYTE_OUTPUT_OFFSET]) {
                printf("ERROR COPY [%d]: @%08x-@%08x : %02x != %02x\n" , i , &test_data_1B[i + BYTE_INPUT_OFFSET] , &copied_data_1B[i + BYTE_OUTPUT_OFFSET], test_data_1B[i + BYTE_INPUT_OFFSET], copied_data_1B[i + BYTE_OUTPUT_OFFSET]);
                errors++;
            }
        }
        // Check that the begining and end values of output vector are not overwriten
        for (int i=0; i<BYTE_OUTPUT_OFFSET; i++) {
            if(copied_data_1B[i] != 0) {
                printf("Data Overwritten @%08x : %04x != 0\n" , &copied_data_1B[i], copied_data_1B[i]);
                errors++;
            }
            if(copied_data_1B[TEST_DATA_SIZE - 1 - i] != 0) {
                printf("Data Overwritten @%08x : %04x != 0\n" , &copied_data_1B[TEST_DATA_SIZE - 1 - i], copied_data_1B[TEST_DATA_SIZE - 1 - i]);
                errors++;
            }
        }

        if (errors == 0) {
            printf("DMA byte transfer success\n");
        } else {
            printf("DMA byte transfer failure: %d errors out of %d bytes checked\n", errors, TEST_DATA_SIZE);
        }
    #endif // TEST_BYTE

    return EXIT_SUCCESS;
}
