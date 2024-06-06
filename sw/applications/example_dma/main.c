// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>

#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "rv_plic.h"
#include "dma_test_macros.h"

// #define TEST_SINGULAR_MODE
#define TEST_ADDRESS_MODE
// #define TEST_PENDING_TRANSACTION
// #define TEST_WINDOW
// #define TEST_ADDRESS_MODE_EXTERNAL_DEVICE

#if TEST_DATA_LARGE < 2 * TEST_DATA_SIZE
#errors("TEST_DATA_LARGE must be at least 2*TEST_DATA_SIZE")
#endif

int8_t cycles = 0;

void dma_intr_handler_trans_done(void)
{
    cycles++;
}

#ifdef TEST_WINDOW

int32_t window_intr_flag;

void dma_intr_handler_window_done(void)
{
    window_intr_flag++;
}

uint8_t dma_window_ratio_warning_threshold()
{
    return 0;
}

#endif // TEST_WINDOW

int test_singular_mode();
int test_pending_transaction();
int test_window();
int test_address_mode();
int test_address_mode_external_device();

int main(int argc, char *argv[])
{
    
    int local_errors = 0;

#ifdef TEST_SINGULAR_MODE

    PRINTF("\n\n\r===================================\n\n\r");
    PRINTF("    TESTING SINGLE MODE   ");
    PRINTF("\n\n\r===================================\n\n\r");

    local_errors = test_singular_mode();

    if (local_errors == 0)
    {
        PRINTF("DMA single mode success.\n\r");
    }
    else
    {
        PRINTF("DMA single mode failed.\n\r");
        return EXIT_FAILURE;
    }

#endif // TEST_SINGULAR_MODE

#ifdef TEST_ADDRESS_MODE

    // PRINTF("\n\n\r===================================\n\n\r");
    // PRINTF("    TESTING ADDRESS MODE   ");
    // PRINTF("\n\n\r===================================\n\n\r");

    local_errors = test_address_mode();

    if (local_errors == 0)
    {
        PRINTF("DMA address mode success.\n\r");
    }
    else
    {
        PRINTF("DMA address mode failed.\n\r");
        return EXIT_FAILURE;
    }

#endif // TEST_ADDRESS_MODE

#if defined(TARGET_SIM) || defined(TARGET_SYSTEMC)

#ifdef TEST_ADDRESS_MODE_EXTERNAL_DEVICE

#pragma message("this application should not be ran in a system integrating x-heep as in the external \
    slave can be plugged something else than a slow memory as in our testbench")

    uint32_t *ext_test_addr_4B_PTR = EXT_SLAVE_START_ADDRESS;
    uint32_t *ext_copied_data_4B;

    ext_copied_data_4B = &ext_test_addr_4B_PTR[TEST_DATA_SIZE + 1];

    tgt_addr.ptr = ext_test_addr_4B_PTR;
    trans.src_addr = &tgt_addr;

    PRINTF("\n\n\r=====================================\n\n\r");
    PRINTF("    TESTING ADDRESS MODE IN EXTERNAL MEMORY  ");
    PRINTF("\n\n\r=====================================\n\n\r");

    // Prepare the data
    for (int i = 0; i < TEST_DATA_SIZE; i++)
    {
        ext_test_addr_4B_PTR[i] = &ext_copied_data_4B[i * 2];
    }

    trans.mode = DMA_TRANS_MODE_ADDRESS;

    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");
    res = dma_launch(&trans);
    PRINTF("laun: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");

    while (!dma_is_ready())
    {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (dma_is_ready() == 0)
        {
            wait_for_interrupt();
            // from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    PRINTF(">> Finished transaction. \n\r");

    for (uint32_t i = 0; i < trans.size_b >> 2; i++)
    {
        if (ext_copied_data_4B[i * 2] != test_data_4B[i])
        {
            PRINTF("ERROR [%d]: %04x != %04x\n\r", i, ext_copied_data_4B[i], test_data_4B[i]);
            errors++;
        }
    }

    if (errors == 0)
    {
        PRINTF("DMA address mode in external memory success.\n\r");
    }
    else
    {
        PRINTF("DMA address mode in external memory failure: %d errors out of %d bytes checked\n\r", errors, trans.size_b);
        return EXIT_FAILURE;
    }

    trans.mode = DMA_TRANS_MODE_SINGLE;

#endif // TEST_ADDRESS_MODE_EXTERNAL_DEVICE

#else
#pragma message("TEST_ADDRESS_MODE_EXTERNAL_DEVICE is not executed on target different than TARGET_SIM")
#endif

#ifdef TEST_PENDING_TRANSACTION
    PRINTF("\n\n\r===================================\n\n\r");
    PRINTF("    TESTING MULTIPLE TRANSACTIONS   ");
    PRINTF("\n\n\r===================================\n\n\r");

    for (uint32_t i = 0; i < TEST_DATA_LARGE; i++)
    {
        test_data_large[i] = i;
    }

    tgt_src.ptr = test_data_large;
    tgt_src.size_du = TEST_DATA_LARGE;
    tgt_dst.size_du = TEST_DATA_LARGE;

    // trans.end = DMA_TRANS_END_INTR_WAIT; // This option makes no sense, because the launch is blocking the program until the trans finishes.
    trans.end = DMA_TRANS_END_INTR;
    // trans.end = DMA_TRANS_END_POLLING;

    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");
    cycles = 0;
    uint8_t consecutive_trans = 0;

    for (uint8_t i = 0; i < TRANSACTIONS_N; i++)
    {
        res = dma_load_transaction(&trans);
        res |= dma_launch(&trans);
        if (res == DMA_CONFIG_OK)
            consecutive_trans++;
    }

    if (trans.end == DMA_TRANS_END_POLLING)
    {
        while (cycles < consecutive_trans)
        {
            while (!dma_is_ready())
                ;
            cycles++;
        }
    }
    else
    {
        while (cycles < consecutive_trans)
        {
            wait_for_interrupt();
        }
    }
    PRINTF(">> Finished %d transactions. That is %s.\n\r", consecutive_trans, consecutive_trans > 1 ? "bad" : "good");

    for (int i = 0; i < TEST_DATA_LARGE; i++)
    {
        if (tgt_src.ptr[i] != tgt_dst.ptr[i])
        {
            PRINTF("ERROR COPY [%d]: %08x != %08x : %04x != %04x\n\r", i, &tgt_src.ptr[i], &tgt_dst.ptr[i], tgt_src.ptr[i], tgt_dst.ptr[i]);
            errors++;
        }
    }

    if (errors == 0)
    {
        PRINTF("DMA multiple transactions success.\n\r");
    }
    else
    {
        PRINTF("DMA multiple transactions failure: %d errors out of %d words checked\n\r", errors, TEST_DATA_SIZE);
        return EXIT_FAILURE;
    }

#endif // TEST_PENDING_TRANSACTION

#ifdef TEST_WINDOW

    PRINTF("\n\n\r===================================\n\n\r");
    PRINTF("    TESTING WINDOW INTERRUPT   ");
    PRINTF("\n\n\r===================================\n\n\r");

    plic_Init();
    plic_irq_set_priority(DMA_WINDOW_INTR, 1);
    plic_irq_set_enabled(DMA_WINDOW_INTR, kPlicToggleEnabled);

    window_intr_flag = 0;

    for (uint32_t i = 0; i < TEST_DATA_LARGE; i++)
    {
        test_data_large[i] = i;
        copied_data_4B[i] = 0;
    }

    tgt_src.ptr = test_data_large;
    tgt_src.size_du = TEST_DATA_LARGE;

    tgt_src.type = DMA_DATA_TYPE_WORD;
    tgt_dst.type = DMA_DATA_TYPE_WORD;

    trans.win_du = TEST_WINDOW_SIZE_DU;
    trans.end = DMA_TRANS_END_INTR;

    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");

    dma_launch(&trans);

    if (trans.end == DMA_TRANS_END_POLLING)
    { // There will be no interrupts whatsoever!
        while (!dma_is_ready())
            ;
        PRINTF("?\n\r");
    }
    else
    {
        while (!dma_is_ready())
        {
            wait_for_interrupt();
            PRINTF("i\n\r");
        }
    }

    PRINTF("\nWe had %d window interrupts.\n\r", window_intr_flag);

    for (uint32_t i = 0; i < TEST_DATA_LARGE; i++)
    {
        if (copied_data_4B[i] != test_data_large[i])
        {
            PRINTF("[%d] %04x\tvs.\t%04x\n\r", i, copied_data_4B[i], test_data_large[i]);
            errors++;
        }
    }

    if (errors == 0)
    {
        PRINTF("DMA window success\n\r");
    }
    else
    {
        PRINTF("DMA window failure: %d errors out of %d words checked\n\r", errors, TEST_DATA_SIZE);
        return EXIT_FAILURE;
    }

#endif // TEST_WINDOW

    return EXIT_SUCCESS;
}

int test_singular_mode()
{
    int global_errors = 0;

    TEST_MODE(DMA_TRANS_MODE_SINGLE)

    return global_errors;
}

int test_address_mode()
{
    int global_errors = 0;

    TEST_MODE(DMA_TRANS_MODE_ADDRESS)

    return global_errors;
}

int test_pending_transaction()
{
}

int test_window()
{
}

int test_address_mode_external_device()
{
}
