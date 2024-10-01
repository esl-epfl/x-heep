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

// TEST DEFINES AND CONFIGURATION

#define TEST_SINGLE_MODE
#define TEST_ADDRESS_MODE
#define TEST_PENDING_TRANSACTION
#define TEST_WINDOW
#define TEST_ADDRESS_MODE_EXTERNAL_DEVICE

#define TEST_DATA_SIZE 16
#define TEST_DATA_LARGE 256
#define TRANSACTIONS_N 3         // Only possible to perform one transaction at a time, others should be blocked
#define TEST_WINDOW_SIZE_DU 256 // if put at <=71 the isr is too slow to react to the interrupt

#if TEST_DATA_LARGE < 2 * TEST_DATA_SIZE
#errors("TEST_DATA_LARGE must be at least 2*TEST_DATA_SIZE")
#endif

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA 1
#define PRINTF_IN_SIM 0

#if TARGET_SIM && PRINTF_IN_SIM
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define PRINTF(...)
#endif

// UTILITIES

#define type2name(dma_type)                                                                   \
    dma_type == DMA_DATA_TYPE_BYTE ? "8-bit" : dma_type == DMA_DATA_TYPE_HALF_WORD ? "16-bit" \
                                           : dma_type == DMA_DATA_TYPE_WORD        ? "32-bit" \
                                                                                   : "TYPE NOT VALID"

dma_data_type_t C_type_2_dma_type(int C_type)
{
    switch (C_type)
    {
    case 1:
        return DMA_DATA_TYPE_BYTE;
    case 2:
        return DMA_DATA_TYPE_HALF_WORD;
    case 4:
        return DMA_DATA_TYPE_WORD;
    default:
        return DMA_DATA_TYPE_WORD;
    }
}

#define WAIT_DMA                              \
    while (!dma_is_ready(0))                   \
    {                                         \
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8); \
        if (dma_is_ready(0) == 0)              \
        {                                     \
            wait_for_interrupt();             \
        }                                     \
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);   \
    }

#define RUN_DMA                                                                               \
    trans.flags = 0x0;                                                                        \
    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY); \
    PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");                \
    res = dma_load_transaction(&trans);                                                       \
    PRINTF("load: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");                \
    res = dma_launch(&trans);                                                                 \
    PRINTF("laun: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");

// TEST MACROS

#define PRINT_TEST(signed, data_size, dma_src_type, dma_dst_type) \
    PRINTF("TEST:\n\r");                                          \
    PRINTF("Data size: %d\n\r", data_size);                       \
    PRINTF("Signed: %d\n\r", signed);                             \
    PRINTF("Source type size: %s\n\r", type2name(dma_src_type));  \
    PRINTF("Destination type size: %s\n\r", type2name(dma_dst_type));

#define DEFINE_DATA(data_size, C_src_type, C_dst_type, signed) \
    C_src_type src[data_size] __attribute__((aligned(4)));     \
    C_dst_type dst[data_size] __attribute__((aligned(4)));     \
    if (data_size <= TEST_DATA_SIZE)                           \
        for (int i = 0; i < data_size; i++)                    \
            if (signed && (i % 2) == 0)                        \
                src[i] = (C_src_type)(-test_data_4B[i]);       \
            else                                               \
                src[i] = (C_src_type)test_data_4B[i];

#define CHECK_RESULTS(data_size)                                                            \
    for (int i = 0; i < data_size; i++)                                                     \
    {                                                                                       \
        if (src[i] != dst[i])                                                               \
        {                                                                                   \
            PRINTF("[%d] Expected: %x Got : %x\n\r", i, src[i], dst[i]);                      \
            errors++;                                                                       \
        }                                                                                   \
    }                                                                                       \
    if (errors != 0)                                                                        \
    {                                                                                       \
        PRINTF("DMA failure: %d errors out of %d elements checked\n\r", errors, trans.size_d1_du); \
        return EXIT_FAILURE;                                                                \
    }

#define INIT_TEST(signed, data_size, dma_src_type, dma_dst_type) \
    tgt_src.ptr = (uint8_t *)src;                                \
    tgt_src.inc_d1_du = 1;                                          \
    tgt_src.inc_d2_du = 0;                                       \
    tgt_src.trig = DMA_TRIG_MEMORY;                              \
    tgt_src.type = dma_src_type;                                 \
    tgt_src.env = NULL;                                          \
    tgt_dst.ptr = (uint8_t *)dst;                                \
    tgt_dst.inc_d1_du = 1;                                          \
    tgt_dst.inc_d2_du = 0;                                       \
    tgt_dst.trig = DMA_TRIG_MEMORY;                              \
    tgt_dst.type = dma_dst_type;                                 \
    tgt_dst.env = NULL;                                          \
    trans.src = &tgt_src;                                        \
    trans.dst = &tgt_dst;                                        \
    trans.src_addr = &tgt_addr;                                  \
    trans.src_type = dma_src_type;                               \
    trans.dst_type = dma_dst_type;                               \
    trans.size_d1_du = data_size;                                \
    trans.mode = DMA_TRANS_MODE_SINGLE;                          \
    trans.win_du = 0;                                            \
    trans.sign_ext = signed;                                     \
    trans.end = DMA_TRANS_END_INTR;                              \
    trans.dim = DMA_DIM_CONF_1D;                                 \

#define TEST(C_src_type, C_dst_type, test_size, sign_extend)                                                         \
    PRINT_TEST(sign_extend, test_size, C_type_2_dma_type(sizeof(C_src_type)), C_type_2_dma_type(sizeof(C_dst_type))) \
    DEFINE_DATA(test_size, C_src_type, C_dst_type, sign_extend)                                                      \
    INIT_TEST(sign_extend, test_size, C_type_2_dma_type(sizeof(C_src_type)), C_type_2_dma_type(sizeof(C_dst_type)))  \
    RUN_DMA                                                                                                          \
    WAIT_DMA                                                                                                         \
    CHECK_RESULTS(test_size)                                                                                         \
    PRINTF("\n\r")

#define TEST_SINGLE                                  \
    {                                                \
        TEST(uint8_t, uint8_t, TEST_DATA_SIZE, 0);   \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(uint8_t, uint16_t, TEST_DATA_SIZE, 0);  \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(uint8_t, uint32_t, TEST_DATA_SIZE, 0);  \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(int8_t, int8_t, TEST_DATA_SIZE, 1);     \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(int8_t, int16_t, TEST_DATA_SIZE, 1);    \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(int8_t, int32_t, TEST_DATA_SIZE, 1);    \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(uint16_t, uint16_t, TEST_DATA_SIZE, 0); \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(uint16_t, uint32_t, TEST_DATA_SIZE, 0); \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(int16_t, int16_t, TEST_DATA_SIZE, 1);   \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(int16_t, int32_t, TEST_DATA_SIZE, 1);   \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(uint32_t, uint32_t, TEST_DATA_SIZE, 0); \
        errors += errors;                            \
    }                                                \
    {                                                \
        TEST(int32_t, int32_t, TEST_DATA_SIZE, 1);   \
        errors += errors;                            \
    }

// GLOBAL VARIABLES

int32_t errors = 0;
int8_t cycles = 0;

// INTERRUPT HANDLERS
void dma_intr_handler_trans_done(uint8_t channel)
{
    cycles++;
}

#ifdef TEST_WINDOW

int32_t window_intr_flag;

void dma_intr_handler_window_done(uint8_t channel) {
    window_intr_flag ++;
}

uint8_t dma_window_ratio_warning_threshold()
{
    return 0;
}

#endif // TEST_WINDOW

dma_trans_t trans;

int main(int argc, char *argv[])
{


    static uint32_t test_data_4B[TEST_DATA_SIZE] __attribute__((aligned(4))) = {
        0x76543210, 0xfedcba98, 0x579a6f90, 0x657d5bee, 0x758ee41f, 0x01234567, 0xfedbca98, 0x89abcdef, 0x679852fe, 0xff8252bb, 0x763b4521, 0x6875adaa, 0x09ac65bb, 0x666ba334, 0x55446677, 0x65ffba98};
    static uint32_t copied_data_4B[TEST_DATA_LARGE] __attribute__((aligned(4))) = {0};
    static uint32_t test_data_large[TEST_DATA_LARGE] __attribute__((aligned(4))) = {0};

    // this array will contain the even address of copied_data_4B
    uint32_t *test_addr_4B_PTR = &test_data_large[0];

    // The DMA is initialized (i.e. Any current transaction is cleaned.)
    dma_init(NULL);
    dma_config_flags_t res;
    dma_target_t tgt_src;
    dma_target_t tgt_dst;
    dma_target_t tgt_addr = {
        .ptr = (uint8_t *)test_addr_4B_PTR,
        .inc_d1_du = 1,
        .trig = DMA_TRIG_MEMORY,
    };

#ifdef TEST_SINGLE_MODE

    PRINTF("\n\n\r===================================\n\n\r");
    PRINTF("    TESTING SINGLE MODE   ");
    PRINTF("\n\n\r===================================\n\n\r");

    TEST_SINGLE

#endif // TEST_SINGLE_MODE

    // Initialize the DMA for the next tests
    tgt_src.ptr = (uint8_t *)test_data_4B;
    tgt_src.inc_d1_du = 1;
    tgt_src.trig = DMA_TRIG_MEMORY;
    tgt_src.type = DMA_DATA_TYPE_WORD;

    tgt_dst.ptr = (uint8_t *)copied_data_4B;
    tgt_dst.inc_d1_du = 1;
    tgt_dst.trig = DMA_TRIG_MEMORY;
    tgt_dst.type = DMA_DATA_TYPE_WORD;

    trans.src = &tgt_src;
    trans.dst = &tgt_dst;
    trans.src_addr = &tgt_addr;
    trans.size_d1_du = TEST_DATA_SIZE;
    trans.src_type = DMA_DATA_TYPE_WORD;
    trans.dst_type = DMA_DATA_TYPE_WORD;
    trans.mode = DMA_TRANS_MODE_SINGLE;
    trans.win_du = 0;
    trans.sign_ext = 0;
    trans.end = DMA_TRANS_END_INTR;

#ifdef TEST_ADDRESS_MODE

    PRINTF("\n\n\r===================================\n\n\r");
    PRINTF("    TESTING ADDRESS MODE   ");
    PRINTF("\n\n\r===================================\n\n\r");

    // Prepare the data
    for (int i = 0; i < TEST_DATA_SIZE; i++)
    {
        test_addr_4B_PTR[i] = (uint32_t)&copied_data_4B[i * 2];
    }

    trans.mode = DMA_TRANS_MODE_ADDRESS;

    RUN_DMA

    WAIT_DMA

    PRINTF(">> Finished transaction. \n\r");

    for (uint32_t i = 0; i < trans.size_d1_du; i++)
    {
        if (copied_data_4B[i * 2] != test_data_4B[i])
        {
            PRINTF("ERROR [%d]: %04x != %04x\n\r", i, copied_data_4B[i * 2], test_data_4B[i]);
            errors++;
        }
    }

    if (errors == 0)
    {
        PRINTF("DMA address mode success.\n\r");
    }
    else
    {
        PRINTF("DMA address mode failure: %d errors out of %d elements checked\n\r", errors, trans.size_d1_du);
        return EXIT_FAILURE;
    }


#endif // TEST_ADDRESS_MODE

#if defined(TARGET_SIM) || defined(TARGET_SYSTEMC)

#ifdef TEST_ADDRESS_MODE_EXTERNAL_DEVICE

#pragma message("this application should not be ran in a system integrating x-heep as in the external \
    slave can be plugged something else than a slow memory as in our testbench")

    uint32_t *ext_test_addr_4B_PTR = (uint32_t *)EXT_SLAVE_START_ADDRESS;
    uint32_t *ext_copied_data_4B;

    ext_copied_data_4B = &ext_test_addr_4B_PTR[TEST_DATA_SIZE + 1];

    tgt_addr.ptr = (uint8_t *) ext_test_addr_4B_PTR;
    trans.src_addr = &tgt_addr;

    PRINTF("\n\n\r=====================================\n\n\r");
    PRINTF("    TESTING ADDRESS MODE IN EXTERNAL MEMORY  ");
    PRINTF("\n\n\r=====================================\n\n\r");

    // Prepare the data
    for (int i = 0; i < TEST_DATA_SIZE; i++)
    {
        ext_test_addr_4B_PTR[i] = (uint32_t) &ext_copied_data_4B[i * 2];
    }

    trans.mode = DMA_TRANS_MODE_ADDRESS;

    RUN_DMA

    WAIT_DMA

    PRINTF(">> Finished transaction. \n\r");

    for (uint32_t i = 0; i < trans.size_d1_du; i++)
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
        PRINTF("DMA address mode in external memory failure: %d errors out of %d elements checked\n\r", errors, trans.size_d1_du);
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

    tgt_src.ptr = (uint8_t *)test_data_large;
    trans.size_d1_du = TEST_DATA_LARGE;
    trans.mode = DMA_TRANS_MODE_SINGLE;

    trans.end = DMA_TRANS_END_INTR;

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
            while (!dma_is_ready(0));
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

    tgt_src.ptr = (uint8_t *)test_data_large;
    trans.size_d1_du = TEST_DATA_LARGE;

    tgt_src.type = DMA_DATA_TYPE_WORD;
    tgt_dst.type = DMA_DATA_TYPE_WORD;

    trans.win_du = TEST_WINDOW_SIZE_DU;
    trans.end = DMA_TRANS_END_INTR;

    RUN_DMA

    if (trans.end == DMA_TRANS_END_POLLING)
    { // There will be no interrupts whatsoever!
        while (!dma_is_ready(0));
        PRINTF("?\n\r");
    }
    else
    {
        while (!dma_is_ready(0))
        {
            wait_for_interrupt();
            PRINTF("i\n\r");//@ToDo: is this a debugging?
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