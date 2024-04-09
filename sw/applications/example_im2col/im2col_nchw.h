#ifndef _IM2COL_
#define _IM2COL_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "im2colGolden.h"

/* 
    WORK IN PROGRESS!
    Used to choose between several HW configurations:
    - 0: Only CPU
    - 1: Exploit standard DMA
    - 2: Exploit multichannel DMA
*/
#define HW_CONFIG 0

// Define the dimensions of the input tensor and the kernel

#define MAX_DIM 8
#define N_PATCHES_H (IH + (P + P) - FH)/ S + 1
#define N_PATCHES_W (IW + (P + P) - FW)/ S + 1
#define OH FW * FH * CH
#define OW (N_PATCHES_W) * (N_PATCHES_H)

#define DEBUG 1 // Set to 1 to enable simple debug prints, 2 to enable more detailed debug prints

int im2col_nchw_f32();

int32_t get_index(int32_t index0, int32_t index1, int32_t index2, int32_t index3);
                
uint16_t verify();

#endif