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
#include "csr.h"

// Define the format of the im2col to test:
// 0: NCHW
// 1: NHWC

#define FORMAT 1

int main()
{
    PRINTF("\nStarting test...\n\n");
    
    uint16_t errors;
    unsigned int cycles;
    
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

    CSR_WRITE(CSR_REG_MCYCLE, 0);
    
    #if FORMAT == 0
        im2col_nchw_int32();
    #elif FORMAT == 1
        im2col_nhwc_int32();
    #endif

    CSR_READ(CSR_REG_MCYCLE, &cycles);
    
    errors = verify();
    
    PRINTF("im2col test executed in %d cycles\n", cycles);

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
