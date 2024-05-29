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

#define DMA_REGISTER_SIZE_BYTES sizeof(int)
#define DMA_SELECTION_OFFSET_START 0

static inline void write_register(uint32_t p_val,
                                  uint32_t p_offset,
                                  uint32_t p_mask,
                                  uint8_t p_sel,
                                  dma *peri)
{
    /*
     * The index is computed to avoid needing to access the structure
     * as a structure.
     */
    uint8_t index = p_offset / DMA_REGISTER_SIZE_BYTES;
    /*
     * An intermediate variable "value" is used to prevent writing twice into
     * the register.
     */
    uint32_t value = ((uint32_t *)peri)[index];
    value &= ~(p_mask << p_sel);
    value |= (p_val & p_mask) << p_sel;
    ((uint32_t *)peri)[index] = value;
}

uint32_t incr[] = {1, 2, 4};

uint32_t data_type[] = {DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_WORD};

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
    while (!dma_is_ready())                                         \
    {                                                               \
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);                       \
        if (dma_is_ready() == 0)                                    \
        {                                                           \
            wait_for_interrupt();                                   \
        }                                                           \
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);                         \
    }

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

    printf("All tests succeded\n");

    return EXIT_SUCCESS;
}
