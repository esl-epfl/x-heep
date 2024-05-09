/*
    Copyright EPFL contributors.
    Licensed under the Apache License, Version 2.0, see LICENSE for details.
    SPDX-License-Identifier: Apache-2.0

    Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
    
    Info: Example application of im2col algorithm with configurable format, verification and performance analysis.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "x-heep.h"
#include "im2col_lib.h"

#define NCHW_FORMAT 0
#define NHWC_FORMAT 1

int main()
{
    PRINTF("\nStarting test...\n\n");
    
    int errors;
    unsigned int cycles;
    
    #if TIMING
        CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
        CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif 
    
    im2col_nchw_int32(); // Execute the im2col algorithm with NCHW format

    #if TIMING
        CSR_READ(CSR_REG_MCYCLE, &cycles);
    #endif
    
    errors = verify(NCHW_FORMAT);

    PRINTF("im2col NCHW test executed\n");
    
    PRINTF_TIM("Total number of cycles: [%d]\n\n", cycles);

    if (errors != 0)
    {
        PRINTF("TEST FAILED: %d errors\n", errors);
        return 1;
    } 
    else
    {
        PRINTF("TEST PASSED!\n");
    }

    #if TIMING
        CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
        CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    im2col_nhwc_int32(); // Execute the im2col algorithm with NHWC format

    #if TIMING
        CSR_READ(CSR_REG_MCYCLE, &cycles);
    #endif

    errors = verify(NHWC_FORMAT);

    PRINTF("im2col NHWC test executed\n");
    PRINTF_TIM("Total number of cycles: [%d]\n\n", cycles);

    if (errors != 0)
    {
        PRINTF("TEST FAILED: %d errors\n", errors);
        return 1;
    } 
    else
    {
        PRINTF("TEST PASSED!\n");
    }

    return 0;
}
