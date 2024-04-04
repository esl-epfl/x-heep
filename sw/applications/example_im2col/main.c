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
#include "im2colGolden.h"

int main()
{
    uint16_t errors, OH, OW, patches_h, patches_w;
    unsigned int instr, cycles;
    
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

    CSR_WRITE(CSR_REG_MCYCLE, 0);

    struct im2col_params parameters;
    parameters.kernel_h = FH;
    parameters.kernel_w = FW;
    parameters.pad_down = P;
    parameters.pad_left = P;
    parameters.pad_right = P;
    parameters.pad_top = P;
    parameters.stride_h = S;
    parameters.stride_w = S;

    struct tensor input, output;
    input.data = input_image;
    input.dim[0] = 1; // batch
    input.dim[1] = CH; // channel
    input.dim[2] = IH; // input h
    input.dim[3] = IW; // input w

    patches_h = (IH + 2 * P - FH)/ S + 1;
    patches_w = (IW + 2 * P - FH)/ S + 1;
    OH = FW * FH * CH;
    OW = patches_h * patches_w;

    //output.data = malloc(OH * OW * sizeof(uint32_t));

    im2col_nchw_f32(&input, &output, &parameters);

    CSR_READ(CSR_REG_MCYCLE, &cycles);
    
    errors = verify(output.data, golden_im2col, OH, OW);

    //free(output.data);
    //output.data = NULL;

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
