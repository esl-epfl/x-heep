/*
    Copyright EPFL contributors.
    Licensed under the Apache License, Version 2.0, see LICENSE for details.
    SPDX-License-Identifier: Apache-2.0

    Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
    
    Info: Header file of im2col_lib.c, containing the function prototypes, parameters macros and the configuration of prints and performance analysis.
*/

#ifndef _IM2COL_
#define _IM2COL_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "im2colGolden.h"
#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "rv_plic.h"
#include "csr.h"

// By default, printfs are activated for FPGA and for simulation.
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0
#define DEBUG 0 // Set to 1 to enable debug prints
#define TIMING 0 // Set to 1 to enable timing measurements

// Format is defined in im2colGolden.h

#if TARGET_SIM && PRINTF_IN_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
    #define PRINTF_DEB(...) 
    #define PRINTF_TIM(...)   
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
    #if DEBUG
        #define PRINTF_DEB(fmt, ...)    printf(fmt, ## __VA_ARGS__)
    #else
        #define PRINTF_DEB(...)
    #endif
    #if TIMING
        #define PRINTF_TIM(fmt, ...)    printf(fmt, ## __VA_ARGS__)
    #else
        #define PRINTF_TIM(...)
    #endif
#else
    #define PRINTF(...)
    #define PRINTF_DEB(...)
    #define PRINTF_TIM(...)
#endif

// Define the dimensions of the input tensor and the kernel

#define N_PATCHES_H ((IH + (PAD + PAD) - FH)/ STRIDES + 1)
#define N_PATCHES_W ((IW + (PAD + PAD) - FW)/ STRIDES + 1)

#define CH_COL (CH * FH * FW)

#define OH_NCHW (CH * FH * FW * BATCH)
#define OW_NCHW (N_PATCHES_H) * (N_PATCHES_W)

#define OW_NHWC (FW * FH * CH * BATCH)
#define OH_NHWC (N_PATCHES_W) * (N_PATCHES_H)

int im2col_nchw_int32();
int im2col_nhwc_int32();

int get_index(int dim1, int dim2, int dim3, int index0, int index1, int index2, int index3);
                
int verify(int format);

#endif