// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: example_dma_sdk.c
// Author: Juan Sapriza
// Date: 13/06/2024
// Description: Example application to test the DMA SDK. Will copy
//              a constant value in a buffer and then copy the content
//              of the buffer into another. Will check that both transactions
//              are performed correctly.

#include <stdint.h>
#include <stdio.h>              // For compatibility with OH Group compiler
#include <stdlib.h>
#include "dma_sdk.h"
#include "dma.h"                // For compatibility with OH Group compiler
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"                // For compatibility with OH Group compiler

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

#define SOURCE_BUFFER_SIZE_32b 5
#define SOURCE_BUFFER_SIZE_16b 5
#define SOURCE_BUFFER_SIZE_8b 5
#define CONST_VALUE_32B 123
#define CONST_NEG_VALUE_32B -123
#define CONST_VALUE_16B 123
#define CONST_NEG_VALUE_16B -123
#define CONST_VALUE_8B 123
#define CONST_NEG_VALUE_8B -123

static uint32_t source_32b[SOURCE_BUFFER_SIZE_32b];
static uint32_t destin_32b[SOURCE_BUFFER_SIZE_32b];

static uint16_t destin_16b[SOURCE_BUFFER_SIZE_16b];
static uint16_t source_16b[SOURCE_BUFFER_SIZE_16b];

static uint8_t destin_8b[SOURCE_BUFFER_SIZE_8b];
static uint8_t source_8b[SOURCE_BUFFER_SIZE_8b];

static int32_t neg_source_32b[SOURCE_BUFFER_SIZE_32b];
static int32_t neg_destin_32b[SOURCE_BUFFER_SIZE_32b];

static int16_t neg_destin_16b[SOURCE_BUFFER_SIZE_16b];
static int16_t neg_source_16b[SOURCE_BUFFER_SIZE_16b];

static int8_t neg_destin_8b[SOURCE_BUFFER_SIZE_8b];
static int8_t neg_source_8b[SOURCE_BUFFER_SIZE_8b];

static uint32_t value_32b = CONST_VALUE_32B;
static uint16_t value_16b = CONST_VALUE_16B;
static uint8_t value_8b = CONST_VALUE_8B;

static int32_t neg_value_32b = CONST_NEG_VALUE_32B;
static int16_t neg_value_16b = CONST_NEG_VALUE_16B;
static int8_t neg_value_8b = CONST_NEG_VALUE_8B;

uint32_t i;
uint32_t errors = 0;

int main()
{
    dma_sdk_init();

    dma_fill((uint32_t)source_32b, (uint32_t)&value_32b, SOURCE_BUFFER_SIZE_32b, 0, DMA_DATA_TYPE_WORD, DMA_DATA_TYPE_WORD, 0);
    dma_copy((uint32_t)destin_32b, (uint32_t)source_32b, SOURCE_BUFFER_SIZE_32b, 0, DMA_DATA_TYPE_WORD, DMA_DATA_TYPE_WORD, 0);

    for (i = 0; i < SOURCE_BUFFER_SIZE_32b; i++)
    {
        errors += destin_32b[i] != CONST_VALUE_32B;
    }

    dma_fill((uint32_t)source_16b, (uint32_t)&value_16b, SOURCE_BUFFER_SIZE_16b, 0, DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_HALF_WORD, 0);
    dma_copy((uint32_t)destin_16b, (uint32_t)source_16b, SOURCE_BUFFER_SIZE_16b, 0, DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_HALF_WORD, 0);

    for (i = 0; i < SOURCE_BUFFER_SIZE_16b; i++)
    {
        errors += destin_16b[i] != CONST_VALUE_16B;
    }

    dma_fill((uint32_t)source_8b, (uint32_t)&value_8b, SOURCE_BUFFER_SIZE_8b, 0, DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_BYTE, 0);
    dma_copy((uint32_t)destin_8b, (uint32_t)source_8b, SOURCE_BUFFER_SIZE_8b, 0, DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_BYTE, 0);

    for (i = 0; i < SOURCE_BUFFER_SIZE_8b; i++)
    {
        errors += destin_8b[i] != CONST_VALUE_8B;
    }

    dma_fill((uint32_t)neg_source_16b, (uint32_t)&neg_value_8b, SOURCE_BUFFER_SIZE_32b, 0, DMA_DATA_TYPE_BYTE, DMA_DATA_TYPE_HALF_WORD, 1);
    dma_copy((uint32_t)neg_destin_32b, (uint32_t)neg_source_16b, SOURCE_BUFFER_SIZE_32b, 0, DMA_DATA_TYPE_HALF_WORD, DMA_DATA_TYPE_WORD, 1);

    for (i = 0; i < SOURCE_BUFFER_SIZE_32b; i++)
    {
        errors += destin_32b[i] != CONST_VALUE_32B;
    }

    PRINTF("Errors:%d\n\r", errors);

    return errors ? EXIT_FAILURE : EXIT_SUCCESS;
}