/*
    Testbench for verification and performance analysis of im2col algorithm.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "x-heep.h"
#include <math.h>
#include "im2col_lib.h"
#include "csr.h"

// Define the format of the im2col to test:
// 0: NCHW
// 1: NHWC
#define FORMAT 0

int main()
{
    printf("\nStarting test...\n\n");
    uint16_t errors;
    unsigned int instr, cycles;
    
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

    CSR_WRITE(CSR_REG_MCYCLE, 0);
    
    #if FORMAT==0
    im2col_nchw_int32();
    #elif FORMAT==1
    im2col_nhwc_int32();
    #endif

    CSR_READ(CSR_REG_MCYCLE, &cycles);
    
    errors = verify();

    #if DEBUG==1 || DEBUG==2
    printf("\n\r%d cycles", cycles);
    fflush(stdout);
    #endif

    if (errors != 0)
    {
        #if DEBUG==0
        printf("\n\rFAIL\n");
        fflush(stdout);
        #endif
        #if DEBUG==1 || DEBUG==2
        printf("\n\rFAIL: %d errors", errors);
        fflush(stdout);
        #endif
        return 1;
    } 
    else
    {
        printf("\n\rPASS\n");
        fflush(stdout);
    }
    return 0;
}
