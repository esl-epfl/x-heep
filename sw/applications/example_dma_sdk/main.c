// Copyright 2024 EPFL and Politecnico di Torino.
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: dma_sdk.c
// Author: Juan Sapriza
// Date: 13/06/2024
// Description: Example application to test the DMA SDK. Will copy
//              a constant value in a buffer and then copy the content
//              of the buffer into another. Will check that both transactions
//              are performed correctly.

#include <stdint.h>
#include <stdlib.h>
#include "dma_sdk.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#define SOURCE_BUFFER_SIZE_32b  5
#define CONST_VALUE             123

int main(){
    static uint32_t source[SOURCE_BUFFER_SIZE_32b];
    static uint32_t destin[SOURCE_BUFFER_SIZE_32b];

    static uint32_t value = CONST_VALUE;

    dma_fill( &source, &value, SOURCE_BUFFER_SIZE_32b );
    dma_copy_32b( &destin, &source, SOURCE_BUFFER_SIZE_32b );

    uint32_t i;
    uint32_t errors = 0;
    for( i = 0; i < SOURCE_BUFFER_SIZE_32b; i++){
        errors += destin[i] != CONST_VALUE;
    }
    PRINTF("Errors:%d\n\r",errors );

    return errors ? EXIT_FAILURE : EXIT_SUCCESS;
}