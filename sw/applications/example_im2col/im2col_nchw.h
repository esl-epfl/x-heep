#ifndef _IM2COL_
#define _IM2COL_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "im2colGolden.h"

#define MAX_DIM 8
#define N_PATCHES_H (IH + (P + P) - FH)/ S + 1
#define N_PATCHES_W (IW + (P + P) - FW)/ S + 1
#define OH FW * FH * CH
#define OW N_PATCHES_W * N_PATCHES_H

int im2col_nchw_f32();

int32_t get_index(int32_t index0, int32_t index1, int32_t index2, int32_t index3);
                
uint16_t verify();

#endif