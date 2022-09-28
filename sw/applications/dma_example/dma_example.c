// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>

#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "dma.h"

#define COPY_SIZE 16

// Source and destination data have to be aligned on a four bytes address
int32_t original_data[COPY_SIZE] __attribute__ ((aligned (4))) = {
  0x76543210,0xfedcba98,0x76543210,0xfedcba98,0x76543210,0xfedcba98,0x76543210,0xfedcba98,0x76543210,0xfedcba98,0x76543210,0xfedcba98,0x76543210,0xfedcba98,0x76543210,0x0000ba98
};
int32_t copied_data_32b[COPY_SIZE] __attribute__ ((aligned (4))) = { 0 };
int16_t copied_data_16b[COPY_SIZE*2] __attribute__ ((aligned (4))) = { 0 };
int8_t copied_data_8b[COPY_SIZE*4] __attribute__ ((aligned (4))) = { 0 };

int8_t dma_intr_flag;

// Interrupt controller variables
dif_plic_params_t rv_plic_params;
dif_plic_t rv_plic;
dif_plic_result_t plic_res;
dif_plic_irq_id_t intr_num;

void handler_irq_external(void) {
    // Claim/clear interrupt
    plic_res = dif_plic_irq_claim(&rv_plic, 0, &intr_num);
    if (plic_res == kDifPlicOk && intr_num == DMA_INTR_DONE) {
        dma_intr_flag = 1;
    }
}

int main(int argc, char *argv[])
{
    printf("--- DMA EXAMPLE ---\n");

    printf("Init the PLIC\n");
    rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)PLIC_START_ADDRESS);
    plic_res = dif_plic_init(rv_plic_params, &rv_plic);
    if (plic_res != kDifPlicOk) {
        printf("PLIC initialization failed\n;");
        return EXIT_FAILURE;
    }

    printf("Set DMA interrupt priority to 1\n");
    // Set dma priority to 1 (target threshold is by default 0) to trigger an interrupt to the target (the processor)
    plic_res = dif_plic_irq_set_priority(&rv_plic, DMA_INTR_DONE, 1);
    if (plic_res != kDifPlicOk) {
        printf("Set priority failed\n;");
        return EXIT_FAILURE;
    }

    printf("Enable DMA interrupt\n");
    plic_res = dif_plic_irq_set_enabled(&rv_plic, DMA_INTR_DONE, 0, kDifPlicToggleEnabled);
    if (plic_res != kDifPlicOk) {
        printf("Enable DMA interrupt failed\n;");
        return EXIT_FAILURE;
    }

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 11;//IRQ_EXT_ENABLE_OFFSET;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    // Put a specific value in the last byte(s) to check they are not overwritten for the 16b and 8b transfers
    // They will be overwritten with zeros if the transaction fails
    copied_data_16b[COPY_SIZE*2-1] = 0xDEAD;
    copied_data_8b[COPY_SIZE*4-1] = 0xDE;


    // dma peripheral structure to access the registers
    dma_t dma;
    dma.base_addr = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);

    dma_set_read_ptr(&dma, (uint32_t) original_data);
    dma_set_write_ptr(&dma, (uint32_t) copied_data_32b);
    printf("DMA 32b transactions launched\n");
    // Give number of bytes to transfer
    dma_set_cnt_start(&dma, (uint32_t) 4*COPY_SIZE);
    // Wait copy is done
    dma_intr_flag = 0;
    while(dma_intr_flag==0) {
        wait_for_interrupt();
    }

    // Complete the interrupt
    plic_res = dif_plic_irq_complete(&rv_plic, 0, &intr_num);
    if (plic_res != kDifPlicOk || intr_num != DMA_INTR_DONE) {
        printf("DMA interrupt complete failed\n;");
    }

    dma_set_read_ptr(&dma, (uint32_t) original_data);
    dma_set_write_ptr(&dma, (uint32_t) copied_data_16b);
    printf("DMA 16b transactions launched\n");
    // Give number of bytes to transfer
    // Last two bytes are not copy to check the DMA works properly
    dma_set_cnt_start(&dma, (uint32_t) 4*COPY_SIZE-2);
    // Wait copy is done
    dma_intr_flag = 0;
    while(dma_intr_flag==0) {
        wait_for_interrupt();
    }

    // Complete the interrupt
    plic_res = dif_plic_irq_complete(&rv_plic, 0, &intr_num);
    if (plic_res != kDifPlicOk || intr_num != DMA_INTR_DONE) {
        printf("DMA interrupt complete failed\n;");
    }

    dma_set_read_ptr(&dma, (uint32_t) original_data);
    dma_set_write_ptr(&dma, (uint32_t) copied_data_8b);
    printf("DMA 8b transactions launched\n");
    // Give number of bytes to transfer
    // Last byte are not copy to check the DMA works properly
    dma_set_cnt_start(&dma, (uint32_t) 4*COPY_SIZE-1);
    // Wait copy is done
    dma_intr_flag = 0;
    while(dma_intr_flag==0) {
        wait_for_interrupt();
    }

    // Complete the interrupt
    plic_res = dif_plic_irq_complete(&rv_plic, 0, &intr_num);
    if (plic_res != kDifPlicOk || intr_num != DMA_INTR_DONE) {
        printf("DMA interrupt complete failed\n;");
    }

    // Check for errors in 32b copy
    int32_t errors=0;
    for(int i=0; i<COPY_SIZE; i++) {
        if (copied_data_32b[i] != original_data[i]) {
            printf("WARNING: %08x != %08x\n", copied_data_32b[i], original_data[i]);
            errors++;
        }
    }

    if (errors == 0) {
        printf("DMA 32B TRANSFER SUCCESS\n");
    } else {
        printf("DMA 32B TRANSFER FAILURE: %d errors out of %d bytes checked\n", errors, 4*COPY_SIZE);
    }

    // Check for errors in 16b copy
    volatile int16_t *src_ptr_16b = original_data;
    errors=0;
    for(int i=0; i<2*COPY_SIZE-1; i++) {
        if (copied_data_16b[i] != *src_ptr_16b) {
            printf("WARNING: %04x != %04x\n", copied_data_16b[i], *src_ptr_16b);
            errors++;
        }
        src_ptr_16b++;
    }
    // Check last two bytes
    if ((copied_data_16b[2*COPY_SIZE-1] & 0x0000FFFF) != 0xDEAD) {
        printf("WARNING: %08x != %08x\n", copied_data_16b[2*COPY_SIZE-1], 0xDEAD);
        errors++;
    }

    if (errors == 0) {
        printf("DMA 16B TRANSFER SUCCESS\n");
    } else {
        printf("DMA 16B TRANSFER FAILURE: %d errors out of %d bytes checked\n", errors, 4*COPY_SIZE);
    }

    // Check for errors in 8b copy
    volatile int8_t *src_ptr_8b = original_data;
    errors=0;
    for(int i=0; i<4*COPY_SIZE-1; i++) {
        if (copied_data_8b[i] != *src_ptr_8b) {
            printf("WARNING: %02x != %02x\n", copied_data_8b[i], *src_ptr_8b);
            errors++;
        }
        src_ptr_8b++;
    }
    // Check last byte
    if ((copied_data_8b[4*COPY_SIZE-1] & 0x000000FF) != 0xDE) {
        printf("WARNING: %08x != %08x\n", copied_data_8b[4*COPY_SIZE-1], 0xDE);
        errors++;
    }    

    if (errors == 0) {
        printf("DMA 8B TRANSFER SUCCESS\n");
    } else {
        printf("DMA 8B TRANSFER FAILURE: %d errors out of %d bytes checked\n", errors, 4*COPY_SIZE);
    }

    return EXIT_SUCCESS;
}
