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
#include <stdlib.h>
#include <stdio.h>
#include "dma_sdk.h"
#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   1

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#define SOURCE_BUFFER_SIZE_32b  5
#define SOURCE_BUFFER_SIZE_16b  5
#define SOURCE_BUFFER_SIZE_8b   5
#define CONST_VALUE_32B         123
#define CONST_VALUE_16B         123
#define CONST_VALUE_8B          123

static uint32_t source_32b[SOURCE_BUFFER_SIZE_32b];
static uint32_t destin_32b[SOURCE_BUFFER_SIZE_32b];

static uint16_t destin_16b[SOURCE_BUFFER_SIZE_16b];
static uint16_t source_16b[SOURCE_BUFFER_SIZE_16b];

static uint8_t destin_8b[SOURCE_BUFFER_SIZE_8b];
static uint8_t source_8b[SOURCE_BUFFER_SIZE_8b];

static uint32_t value_32b = CONST_VALUE_32B;
static uint16_t value_16b = CONST_VALUE_16B;
static uint8_t value_8b = CONST_VALUE_8B;

uint32_t i;
uint32_t errors = 0;

#define FS_INITIAL 0x01
int main(){

    CSR_SET_BITS(CSR_REG_MSTATUS, (FS_INITIAL << 13));
    dma_sdk_init();

    dma *the_dma = dma_peri(0);


    DMA_FILL(source_32b, &value_32b, SOURCE_BUFFER_SIZE_32b, uint32_t, uint32_t, 0, the_dma);
    DMA_WAIT(0);
    DMA_COPY(destin_32b, source_32b, SOURCE_BUFFER_SIZE_32b, uint32_t, uint32_t, 0, the_dma);
    DMA_WAIT(0);
    // dma_fill_32b( &source_32b, &value_32b, SOURCE_BUFFER_SIZE_32b, 0);
    // dma_copy_32b( &destin_32b, &source_32b, SOURCE_BUFFER_SIZE_32b, 0);

    for( i = 0; i < SOURCE_BUFFER_SIZE_32b; i++){
        errors += destin_32b[i] != CONST_VALUE_32B;
    }

    DMA_FILL(source_16b, &value_16b, SOURCE_BUFFER_SIZE_16b, uint16_t, uint16_t, 0, the_dma);
    DMA_WAIT(0);
    DMA_COPY(destin_16b, source_16b, SOURCE_BUFFER_SIZE_16b, uint16_t, uint16_t, 0, the_dma);
    DMA_WAIT(0);

    for( i = 0; i < SOURCE_BUFFER_SIZE_16b; i++){
        errors += destin_16b[i] != CONST_VALUE_16B;
    }
    // printf("Errors:%d\n\r",errors );

    DMA_FILL(source_8b, &value_8b, SOURCE_BUFFER_SIZE_8b, uint8_t, uint8_t, 0, the_dma);
    DMA_WAIT(0);
    DMA_COPY(destin_8b, source_8b, SOURCE_BUFFER_SIZE_8b, uint8_t, uint8_t, 0, the_dma);
    DMA_WAIT(0);

    // // dma_fill_8b( &source_8b, &value_8b, SOURCE_BUFFER_SIZE_8b, 0);
    // // dma_copy_8b( &destin_8b, &source_8b, SOURCE_BUFFER_SIZE_8b, 0);

    for( i = 0; i < SOURCE_BUFFER_SIZE_8b; i++){
        errors += destin_8b[i] != CONST_VALUE_8B;
    }

    PRINTF("Errors:%d\n\r",errors );

    return errors ? EXIT_FAILURE : EXIT_SUCCESS;
}