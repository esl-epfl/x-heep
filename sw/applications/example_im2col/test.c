/*
    Testbench for verification and performance analysis of im2col algorithm.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <im2col_nchw.h>
#include "csr.h"
#include <im2colGolden.h>

int main()
{
    uint16_t errors, OH, OW, patches_h, patches_w;

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
    input.dim[0] = 0; // batch
    input.dim[1] = CH; // channel
    input.dim[2] = IH; // input h
    input.dim[3] = IW; // input w

    patches_h = (IH + 2 * P - FH)/ S + 1;
    patches_w = (IW + 2 * P - FH)/ S + 1;
    OH = FW * FH * CH;
    OW = patches_h * patches_w;

    im2col_nchw_f32(&input, &output, &parameters);
    
    return 0;
}
