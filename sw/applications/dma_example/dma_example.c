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

// Choose which scenarios to test
#define TEST_4_BYTES_ALIGNED
#define TEST_2_BYTES_ALIGNED
#define TEST_BYTE_ALIGNED

#define TEST_DATA_SIZE 16

// Source and destination addresses have to be aligned on a 4 bytes address
uint32_t test_data[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = {
  0x76543210, 0xfedcba98, 0x579a6f90, 0x657d5bee, 0x758ee41f, 0x01234567, 0xfedbca98, 0x89abcdef, 0x679852fe, 0xff8252bb, 0x763b4521, 0x6875adaa, 0x09ac65bb, 0x666ba334, 0x55446677, 0x65ffba98};
uint32_t copied_data_4B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = { 0 };
uint32_t copied_data_2B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = { 0 };
uint32_t copied_data_1B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = { 0 };

int8_t dma_intr_flag;

void handler_irq_fast_dma(void)
{
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kDma_e);

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

    #ifdef TEST_4_BYTES_ALIGNED
        dma_set_read_ptr(&dma, (uint32_t) test_data);
        dma_set_write_ptr(&dma, (uint32_t) copied_data_4B);
        printf("DMA 32b aligned transaction launched\n");
        // Give number of bytes to transfer
        dma_set_cnt_start(&dma, (uint32_t) 4*TEST_DATA_SIZE);
        // Wait copy is done
        dma_intr_flag = 0;
        while(dma_intr_flag==0) {
            wait_for_interrupt();
        }
    #endif // TEST_4_BYTES_ALIGNED

    #ifdef TEST_2_BYTES_ALIGNED
        dma_set_read_ptr(&dma, (uint32_t) test_data);
        dma_set_write_ptr(&dma, (uint32_t) copied_data_2B);
        printf("DMA 16b aligned transaction launched\n");
        // Give number of bytes to transfer
        // Last 2 bytes are not copy to check the DMA works properly
        dma_set_cnt_start(&dma, (uint32_t) 4*TEST_DATA_SIZE-2);
        // Wait copy is done
        dma_intr_flag = 0;
        while(dma_intr_flag==0) {
            wait_for_interrupt();
        }
    #endif // TEST_2_BYTES_ALIGNED

    #ifdef TEST_BYTE_ALIGNED
        dma_set_read_ptr(&dma, (uint32_t) test_data);
        dma_set_write_ptr(&dma, (uint32_t) copied_data_1B);
        printf("DMA 8b aligned transaction launched\n");
        // Give number of bytes to transfer
        // Last byte are not copy to check the DMA works properly
        dma_set_cnt_start(&dma, (uint32_t) 4*TEST_DATA_SIZE-1);
        // Wait copy is done
        dma_intr_flag = 0;
        while(dma_intr_flag==0) {
            wait_for_interrupt();
        }
    #endif // TEST_BYTE_ALIGNED

    int32_t errors;

    #ifdef TEST_4_BYTES_ALIGNED
        errors=0;
        for(int i=0; i<TEST_DATA_SIZE; i++) {
            if (copied_data_4B[i] != test_data[i]) {
                printf("ERROR COPY [%d]: %08x != %08x\n", i, copied_data_4B[i], test_data[i]);
                errors++;
            }
        }

        if (errors == 0) {
            printf("DMA 32b aligned transfer success\n");
        } else {
            printf("DMA 32b aligned transfer failure: %d errors out of %d bytes checked\n", errors, 4*TEST_DATA_SIZE);
        }
    #endif // TEST_4_BYTES_ALIGNED

    #ifdef TEST_2_BYTES_ALIGNED
        uint16_t *test_data_16b = (uint16_t *)test_data;
        uint16_t *copy_data_16b = (uint16_t *)copied_data_2B;

        errors = 0;
        for (int i=0; i<2*TEST_DATA_SIZE-1; i++) {
            if(test_data_16b[i] != copy_data_16b[i]) {
                printf("@%08x-@%08x : %04x != %04x\n" , &test_data_16b[i] , &copy_data_16b[i], test_data_16b[i], copy_data_16b[i]);
                errors++;
            }
        }
        // Check last byte have not been overwritten
        if(copy_data_16b[2*TEST_DATA_SIZE-1] != 0) {
            printf("Data Overwritten @%08x : %04x != 0\n" , &copy_data_16b[2*TEST_DATA_SIZE-1], copy_data_16b[2*TEST_DATA_SIZE-1]);
            errors++;
        }

        if (errors == 0) {
            printf("DMA 16b aligned transfer success\n");
        } else {
            printf("DMA 16b aligned transfer failure: %d errors out of %d bytes checked\n", errors, 4*TEST_DATA_SIZE);
        }
    #endif // TEST_2_BYTES_ALIGNED

    #ifdef TEST_BYTE_ALIGNED
        uint8_t *test_data_8b = (uint8_t *)test_data;
        uint8_t *copy_data_8b = (uint8_t *)copied_data_1B;

        errors = 0;
        for (int i=0; i<4*TEST_DATA_SIZE-1; i++) {
            if(test_data_8b[i] != copy_data_8b[i]) {
                printf("@%08x-@%08x : %02x != %02x\n" , &test_data_8b[i] , &copy_data_8b[i], test_data_8b[i], copy_data_8b[i]);
                errors++;
            }
        }
        // Check last byte have not been overwritten
        if(copy_data_8b[4*TEST_DATA_SIZE-1] != 0) {
            printf("Data Overwritten @%08x : %02x != 0\n" , &copy_data_8b[4*TEST_DATA_SIZE-1], copy_data_8b[4*TEST_DATA_SIZE-1]);
            errors++;
        }

        if (errors == 0) {
            printf("DMA 8b aligned transfer success\n");
        } else {
            printf("DMA 8b aligned transfer failure: %d errors out of %d bytes checked\n", errors, 4*TEST_DATA_SIZE);
        }
    #endif // TEST_BYTE_ALIGNED

    return EXIT_SUCCESS;
}
