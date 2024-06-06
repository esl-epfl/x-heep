#ifndef DMA_TEST_H
#define DMA_TEST_H

#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "dma.h"

#define TEST_DATA_SIZE 16
#define TEST_DATA_LARGE 1024
#define TRANSACTIONS_N 3         // Only possible to perform transaction at a time, others should be blocked
#define TEST_WINDOW_SIZE_DU 1024 // if put at <=71 the isr is too slow to react to the interrupt

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA 1
#define PRINTF_IN_SIM 1

#if TARGET_SIM && PRINTF_IN_SIM
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define type2name(dma_type)                                                                   \
    dma_type == DMA_DATA_TYPE_BYTE ? "8-bit" : dma_type == DMA_DATA_TYPE_HALF_WORD ? "16-bit" \
                                           : dma_type == DMA_DATA_TYPE_WORD        ? "32-bit" \
                                                                                   : "TYPE NOT VALID"

dma_data_type_t C2dma_type(int C_type)
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
static uint32_t test_data_4B[TEST_DATA_SIZE] __attribute__((aligned(4))) = {
    0x76543210, 0xfedcba98, 0x579a6f90, 0x657d5bee, 0x758ee41f, 0x01234567, 0xfedbca98, 0x89abcdef, 0x679852fe, 0xff8252bb, 0x763b4521, 0x6875adaa, 0x09ac65bb, 0x666ba334, 0x55446677, 0x65ffba98};

#define PRINT_TEST(signed, data_size, dma_src_type, dma_dst_type) \
    // PRINTF("TEST:\n\r");                                          \
    // PRINTF("Data size: %d\n\r", data_size);                       \
    // PRINTF("Signed: %d\n\r", signed);                             \
    // PRINTF("Source type size: %s\n\r", type2name(dma_src_type));  \
    // PRINTF("Destination type size: %s\n\r", type2name(dma_dst_type))

#define DEFINE_DATA(data_size, C_src_type, C_dst_type, signed, dma_mode) \
    C_src_type src[data_size] __attribute__((aligned(4)));               \
    C_src_type src_large[TEST_DATA_LARGE] __attribute__((aligned(4)));   \
    C_dst_type dst[TEST_DATA_LARGE] __attribute__((aligned(4)));         \
    C_src_type *src_addr = src_large;                                    \
    if (dma_mode != DMA_TRANS_MODE_ADDRESS)                              \
    {                                                                    \
        if (data_size <= TEST_DATA_SIZE)                                 \
        {                                                                \
            for (int i = 0; i < data_size; i++)                          \
            {                                                            \
                if (signed &&i % 2 == 0)                                 \
                {                                                        \
                    src[i] = (C_src_type)(-test_data_4B[i]);             \
                }                                                        \
                else                                                     \
                {                                                        \
                    src[i] = (C_src_type)test_data_4B[i];                \
                }                                                        \
            }                                                            \
        }                                                                \
        else                                                             \
        {                                                                \
            for (int i = 0; i < data_size; i++)                          \
            {                                                            \
                src[i] = (C_src_type)test_data_4B[i];                    \
            }                                                            \
        }                                                                \
    }                                                                    \
    else                                                                 \
    {                                                                    \
        for (int i = 0; i < data_size; i++)                              \
        {                                                                \
            printf("src_addr[%d] = %p\n", i, &dst[i * 2]);               \
            src_addr[i] = &dst[i * 2];                                   \
        }                                                                \
    }

#define WAIT_DMA                              \
    while (!dma_is_ready())                   \
    {                                         \
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8); \
        if (dma_is_ready() == 0)              \
        {                                     \
            wait_for_interrupt();             \
        }                                     \
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);   \
    }

#define CHECK_RESULTS(data_size, dma_mode)                                                  \
    int errors = 0;                                                                         \
    if (dma_mode != DMA_TRANS_MODE_ADDRESS)                                                 \
    {                                                                                       \
        for (int i = 0; i < data_size; i++)                                                 \
        {                                                                                   \
            if (src[i] != dst[i])                                                           \
            {                                                                               \
                PRINTF("[%d] Expected: %x Got : %x\n", i, src[i], dst[i]);                  \
                errors++;                                                                   \
            }                                                                               \
        }                                                                                   \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        for (uint32_t i = 0; i < (data_size * sizeof(dst[0])) >> 2; i++)                    \
        {                                                                                   \
            if (dst[i * 2] != src[i])                                                       \
            {                                                                               \
                PRINTF("[%d] Expected: %x Got : %x\n", i, src[i], dst[i]);                  \
                errors++;                                                                   \
            }                                                                               \
        }                                                                                   \
    }                                                                                       \
    if (errors != 0)                                                                        \
    {                                                                                       \
        PRINTF("DMA failure: %d errors out of %d bytes checked\n\r", errors, trans.size_b); \
    }

#define INIT_TEST(signed, data_size, dma_src_type, dma_dst_type, dma_mode) \
    dma_init(NULL);                                                        \
    dma_config_flags_t res;                                                \
    dma_target_t tgt_src = {                                               \
        .ptr = src,                                                        \
        .inc_du = 1,                                                       \
        .size_du = data_size,                                              \
        .trig = DMA_TRIG_MEMORY,                                           \
        .type = dma_src_type,                                              \
    };                                                                     \
    dma_target_t tgt_dst = {                                               \
        .ptr = dst,                                                        \
        .inc_du = 1,                                                       \
        .size_du = data_size,                                              \
        .trig = DMA_TRIG_MEMORY,                                           \
        .type = dma_dst_type,                                              \
    };                                                                     \
    dma_target_t tgt_addr = {                                              \
        .ptr = src_addr,                                                   \
        .inc_du = 1,                                                       \
        .size_du = data_size,                                              \
        .trig = DMA_TRIG_MEMORY,                                           \
    };                                                                     \
    dma_trans_t trans = {                                                  \
        .src = &tgt_src,                                                   \
        .dst = &tgt_dst,                                                   \
        .src_addr = &tgt_addr,                                             \
        .src_type = dma_dst_type,                                          \
        .dst_type = dma_dst_type,                                          \
        .mode = dma_mode,                                                  \
        .win_du = 0,                                                       \
        .sign_ext = signed,                                                \
        .end = DMA_TRANS_END_INTR,                                         \
    }

#define RUN_TEST                                                                                                 \
    dma_config_flags_t ret = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY); \
    PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");                                   \
    ret = dma_load_transaction(&trans);                                                                          \
    PRINTF("load: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!");                                   \
    ret = dma_launch(&trans);                                                                                    \
    PRINTF("laun: %u \t%s\n\r", res, res == DMA_CONFIG_OK ? "Ok!" : "Error!")

#define TEST(C_src_type, C_dst_type, test_size, sign_extend, dma_mode)                                           \
    PRINT_TEST(sign_extend, test_size, C2dma_type(sizeof(C_src_type)), C2dma_type(sizeof(C_dst_type)));          \
    DEFINE_DATA(test_size, C_src_type, C_dst_type, sign_extend, dma_mode)                                        \
    INIT_TEST(sign_extend, test_size, C2dma_type(sizeof(C_src_type)), C2dma_type(sizeof(C_dst_type)), dma_mode); \
    RUN_TEST;                                                                                                    \
    WAIT_DMA                                                                                                     \
    CHECK_RESULTS(test_size, dma_mode)                                                                           \
    PRINTF("\n\r");

#define TEST_MODE(dma_mode)                                    \
    {                                                          \
        TEST(uint8_t, uint8_t, TEST_DATA_SIZE, 0, dma_mode);   \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(uint8_t, uint16_t, TEST_DATA_SIZE, 0, dma_mode);  \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(uint8_t, uint32_t, TEST_DATA_SIZE, 0, dma_mode);  \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(int8_t, int8_t, TEST_DATA_SIZE, 1, dma_mode);     \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(int8_t, int16_t, TEST_DATA_SIZE, 1, dma_mode);    \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(int8_t, int32_t, TEST_DATA_SIZE, 1, dma_mode);    \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(uint16_t, uint16_t, TEST_DATA_SIZE, 0, dma_mode); \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(uint16_t, uint32_t, TEST_DATA_SIZE, 0, dma_mode); \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(int16_t, int16_t, TEST_DATA_SIZE, 1, dma_mode);   \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(int16_t, int32_t, TEST_DATA_SIZE, 1, dma_mode);   \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(uint32_t, uint32_t, TEST_DATA_SIZE, 0, dma_mode); \
        global_errors += errors;                               \
    }                                                          \
    {                                                          \
        TEST(int32_t, int32_t, TEST_DATA_SIZE, 1, dma_mode);   \
        global_errors += errors;                               \
    }

#endif // DMA_TEST_H
