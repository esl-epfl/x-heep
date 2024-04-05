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

    im2col_nchw_f32();

    CSR_READ(CSR_REG_MCYCLE, &cycles);
    
    errors = verify();

    if (errors != 0)
    {
        printf("Im2col test FAILED with %d errors.\n");
    } 
    else 
    {
        printf("Im2col test PASSED!\n");
    }

    printf("Performance anlysis: this iteration lastes %d cycles\n\n", cycles);

    return 0;
}
