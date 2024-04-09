/*
    Testbench for verification and performance analysis of im2col algorithm.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "x-heep.h"
#include <math.h>
#include "im2col_nchw.h"
#include "csr.h"

int main()
{
    uint16_t errors;
    unsigned int instr, cycles;
    
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

    CSR_WRITE(CSR_REG_MCYCLE, 0);

    printf("...\n");
    
    im2col_nchw_f32();

    CSR_READ(CSR_REG_MCYCLE, &cycles);
    
    errors = verify();

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
    } 
    else 
    {
        printf("\n\rPASS\n");
        fflush(stdout);
    }

    #if DEBUG==1 || DEBUG==2
    printf("\n\r%d cycles", cycles);
    fflush(stdout);
    #endif

    return 0;
}
