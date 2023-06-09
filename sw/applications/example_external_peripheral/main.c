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
#include "rv_plic_structs.h"
#include "dma.h"

#define COPY_SIZE 10

#ifndef RV_PLIC_IS_INCLUDED
  #error ( "This app does NOT work as the RV_PLIC peripheral is not included" )
#endif

#ifdef TARGET_PYNQ_Z2
  #error ( "This app does NOT work on the FPGA as it relies on the simulator testbench" )
#endif

// Interrupt controller variables
plic_result_t plic_res;
plic_irq_id_t intr_num;


int main(int argc, char *argv[])
{

    printf("Init the PLIC...");
    plic_res = plic_Init();

    if (plic_res != kPlicOk) {
        return -1;
    }

    // Set memcopy priority to 1 (target threshold is by default 0) to trigger an interrupt to the target (the processor)
    plic_res = plic_irq_set_priority(EXT_INTR_0, 1);
    if (plic_res == kPlicOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Enable MEMCOPY interrupt...");
    plic_res = plic_irq_set_enabled(EXT_INTR_0, kPlicToggleEnabled);
    if (plic_res == kPlicOk) {
        printf("Success\n");
    } else {
        printf("Fail\n;");
    }

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 11;//IRQ_EXT_ENABLE_OFFSET;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    // Use the stack
    int32_t original_data[COPY_SIZE];
    int32_t copied_data[COPY_SIZE];
    // Or use the slow sram ip example for data
    // int32_t original_data = EXT_SLAVE_START_ADDRESS;
    // int32_t copied_data = EXT_SLAVE_START_ADDRESS+COPY_SIZE;

    volatile uint32_t *src_ptr = original_data;
    volatile uint32_t *dest_ptr = copied_data;

    // Put some data to initialize the memory addresses
    for(int i=0; i<COPY_SIZE; i++) {
        *src_ptr++ = i;
    }

    // dma peripheral structure to access the registers
    dma_t memcopy_periph;
    memcopy_periph.base_addr = mmio_region_from_addr((uintptr_t)EXT_PERIPHERAL_START_ADDRESS);


    dma_set_read_ptr(&memcopy_periph, (uint32_t) original_data);
    dma_set_write_ptr(&memcopy_periph, (uint32_t) copied_data);
    dma_set_read_ptr_inc(&memcopy_periph, (uint32_t) 4);
    dma_set_write_ptr_inc(&memcopy_periph, (uint32_t) 4);
    dma_set_spi_mode(&memcopy_periph, (uint32_t) 0);
    dma_set_data_type(&memcopy_periph, (uint32_t) 0);

    printf("Memcopy launched...\r\n");
    dma_set_cnt_start(&memcopy_periph, (uint32_t) COPY_SIZE*sizeof(*original_data));
    // Wait copy is done
    while(plic_intr_flag==0) {
        wait_for_interrupt();
    }
    printf("Memcopy finished...\r\n");

    // Reinitialized the read pointer to the original address
    src_ptr = original_data;

    // Check for errors
    int32_t errors=0;
    for(int i=0; i<COPY_SIZE; i++) {
        if ((*src_ptr) != (*dest_ptr)) {
            printf("WARNING: %d != %d\n", *src_ptr, *dest_ptr);
            errors++;
        }
        src_ptr++;
        dest_ptr++;
    }

    if (errors == 0) {
        printf("MEMCOPY SUCCESS\n");
    } else {
        printf("MEMCOPY FAILURE: %d errors out of %d words copied\n", errors, COPY_SIZE);
        return -1;
    }

    return EXIT_SUCCESS;
}
