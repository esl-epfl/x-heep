/*
 *  Copyright EPFL and PoliTO contributors.
 *  Licensed under the Apache License, Version 2.0, see LICENSE for details.
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Author: Luigi Giuffrida <luigi.giuffrida@polito.it>
 *  
 *  Info: Example application of the DMA peripheral for signed data types
 *
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "rv_plic.h"

uint32_t incr[] = {1, 2, 4};

uint32_t data_type[] = {DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_WORD};

#define wait_dma                              \
    while (!dma_is_ready())                   \
    {                                         \
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8); \
        if (dma_is_ready() == 0)              \
        {                                     \
            wait_for_interrupt();             \
        }                                     \
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);   \
    }

#define launch(src_type, dst_type, dma, DMA_SRC_TYPE, DMA_DST_TYPE) \
    src_type src[5] = {-1, -2, -3, -4, -5};                         \
    dst_type dst[5] = {0};                                          \
    dma->SRC_PTR = src;                                             \
    dma->DST_PTR = dst;                                             \
    dma->SRC_DATA_TYPE = DMA_SRC_TYPE;                              \
    dma->DST_DATA_TYPE = DMA_DST_TYPE;                              \
    dma->SRC_PTR_INC_D1 = sizeof(src[0]);                           \
    dma->DST_PTR_INC_D1 = sizeof(dst[0]);                           \
    dma->SIGN_EXT = 1;                                              \
    dma->INTERRUPT_EN = 0x1;                                        \
    dma->MODE = DMA_TRANS_MODE_SINGLE;                              \
    dma->SIZE_D2 = 0;                                               \
    dma->SIZE_D1 = 5 * sizeof(dst[0]);                              \
    wait_dma;

#define check_results                         \
    for (int i = 0; i < 5; i++)               \
    {                                         \
        if (src[i] != dst[i])                 \
        {                                     \
            printf("Error at index %d\n", i); \
            printf("Expected: %x\n", src[i]); \
            printf("Got: %x\n", dst[i]);      \
            return EXIT_FAILURE;              \
        }                                     \
    }

#define launch_with_HAL(src_TYPE, dst_TYPE, dma, DMA_SRC_TYPE, DMA_DST_TYPE) \
    src_TYPE src[5] = {-1, -2, -3, -4, -5};                                  \
    dst_TYPE dst[5] = {0};                                                   \
    dma_target_t src_target = {0};                                           \
    src_target.ptr = src;                                                    \
    src_target.inc_du = 1;                                                   \
    src_target.size_du = 5;                                                  \
    src_target.type = DMA_SRC_TYPE;                                          \
    dma_target_t dst_target = {0};                                           \
    dst_target.ptr = dst;                                                    \
    dst_target.inc_du = 1;                                                   \
    dst_target.size_du = 5;                                                  \
    dst_target.type = DMA_DST_TYPE;                                          \
    dma_trans_t trans = {0};                                                 \
    trans.src = &src_target;                                                 \
    trans.dst = &dst_target;                                                 \
    trans.src_type = DMA_SRC_TYPE;                                           \
    trans.dst_type = DMA_DST_TYPE;                                           \
    trans.sign_ext = 1;                                                      \
    trans.mode = DMA_TRANS_MODE_SINGLE;                                      \
    if (dma_validate_transaction(&trans, DMA_DO_NOT_ENABLE_REALIGN, 1) != 0) \
    {                                                                        \
        printf("DMA FAIL");                                                  \
        return EXIT_FAILURE;                                                 \
    }                                                                        \
    dma_load_transaction(&trans);                                            \
    dma_launch(&trans);                                                      \
    wait_dma;

int main(int argc, char *argv[])
{

    dma *peri = dma_peri;

    // Initialize the DMA
    dma_init(peri);

    do
    {
        // Launch the DMA
        launch(int8_t, int8_t, peri, DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_BYTE);

        // Check the results
        check_results;
    } while (0);

    printf("Test int8_t -> int8_t succeded\n");

    do
    {
        // Launch the DMA
        launch(int8_t, int16_t, peri, DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_HALF_WORD);

        // Check the results
        check_results;
    } while (0);

    printf("Test int8_t -> int16_t succeded\n");

    do
    {
        // Launch the DMA
        launch(int8_t, int32_t, peri, DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_WORD);

        // Check the results
        check_results;
    } while (0);

    printf("Test int8_t -> int32_t succeded\n");

    do
    {
        // Launch the DMA
        launch(int16_t, int16_t, peri, DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_HALF_WORD);

        // Check the results
        check_results;
    } while (0);

    printf("Test int16_t -> int16_t succeded\n");

    do
    {
        // Launch the DMA
        launch(int16_t, int32_t, peri, DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_WORD);

        // Check the results
        check_results;
    } while (0);

    printf("Test int16_t -> int32_t succeded\n");

    do
    {
        // Launch the DMA
        launch(int32_t, int32_t, peri, DMA_DATA_TYPE_WORD, DMA_DATA_TYPE_WORD);

        // Check the results
        check_results;
    } while (0);

    printf("Test int32_t -> int32_t succeded\n");

    dma_init(peri);

    do
    {
        // Launch the DMA
        launch_with_HAL(int8_t, int8_t, peri, DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_BYTE);

        // Check the results
        check_results;
    } while (0);
 
    printf("Test int8_t -> int8_t succeded with HAL\n");

    do
    {
        // Launch the DMA
        launch_with_HAL(int8_t, int16_t, peri, DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_HALF_WORD);

        // Check the results
        check_results;
    } while (0);

    printf("Test int8_t -> int16_t succeded with HAL\n");

    do
    {
        // Launch the DMA
        launch_with_HAL(int8_t, int32_t, peri, DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_WORD);

        // Check the results
        check_results;
    } while (0);

    printf("Test int8_t -> int32_t succeded with HAL\n");

    do
    {
        // Launch the DMA
        launch_with_HAL(int16_t, int16_t, peri, DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_HALF_WORD);

        // Check the results
        check_results;
    } while (0);

    printf("Test int16_t -> int16_t succeded with HAL\n");

    do
    {
        // Launch the DMA
        launch_with_HAL(int16_t, int32_t, peri, DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_WORD);

        // Check the results
        check_results;
    } while (0);

    printf("Test int16_t -> int32_t succeded with HAL\n");

    do
    {
        // Launch the DMA
        launch_with_HAL(int32_t, int32_t, peri, DMA_DATA_TYPE_WORD, DMA_DATA_TYPE_WORD);

        // Check the results
        check_results;
    } while (0);

    printf("Test int32_t -> int32_t succeded with HAL\n");

    printf("All tests succeded\n");

    return EXIT_SUCCESS;
}
