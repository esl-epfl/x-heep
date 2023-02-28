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
#include "memcopy_periph.h"

#define COPY_SIZE 10

int8_t external_intr_flag;

// Interrupt controller variables
dif_plic_params_t rv_plic_params;
dif_plic_t rv_plic;
dif_plic_result_t plic_res;
dif_plic_irq_id_t intr_num;

void handler_irq_external(void) {
    // Claim/clear interrupt
    plic_res = dif_plic_irq_claim(&rv_plic, 0, &intr_num);
    if (plic_res == kDifPlicOk && intr_num == EXT_INTR_0) {
        external_intr_flag = 1;
    }
}

int main(int argc, char *argv[])
{
    printf("--- MEMCOPY EXAMPLE - external peripheral ---\n");

    printf("Init the PLIC...");
    rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)RV_PLIC_START_ADDRESS);
    plic_res = dif_plic_init(rv_plic_params, &rv_plic);

    if (plic_res == kDifPlicOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Set MEMCOPY interrupt priority to 1...");
    // Set memcopy priority to 1 (target threshold is by default 0) to trigger an interrupt to the target (the processor)
    plic_res = dif_plic_irq_set_priority(&rv_plic, EXT_INTR_0, 1);
    if (plic_res == kDifPlicOk) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

    printf("Enable MEMCOPY interrupt...");
    plic_res = dif_plic_irq_set_enabled(&rv_plic, EXT_INTR_0, 0, kDifPlicToggleEnabled);
    if (plic_res == kDifPlicOk) {
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
    external_intr_flag = 0;

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

    // memcopy peripheral structure to access the registers
    memcopy_periph_t memcopy_periph;
    memcopy_periph.base_addr = mmio_region_from_addr((uintptr_t)EXT_PERIPHERAL_START_ADDRESS);

    memcopy_periph_set_read_ptr(&memcopy_periph, (uint32_t) original_data);
    memcopy_periph_set_write_ptr(&memcopy_periph, (uint32_t) copied_data);
    printf("Memcopy launched...");
    memcopy_periph_set_cnt_start(&memcopy_periph, (uint32_t) COPY_SIZE);
    // Wait copy is done
    while(external_intr_flag==0) {
        wait_for_interrupt();
    }
    printf("finished\n");

    printf("Complete interrupt...");
    plic_res = dif_plic_irq_complete(&rv_plic, 0, &intr_num);
    if (plic_res == kDifPlicOk && intr_num == EXT_INTR_0) {
        printf("success\n");
    } else {
        printf("fail\n;");
    }

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
    }

    return EXIT_SUCCESS;
}
