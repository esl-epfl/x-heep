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

/* 
    Used to choose between several HW configurations:
    - 0: Only CPU
    - 1: Exploit standard DMA
    - 2: Exploit 2D DMA
*/ 
#define HW_CONFIG_CPU
#define HW_CONFIG_DMA_1D
#define HW_CONFIG_DMA_2D

/* By default, printfs are activated for FPGA and for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   1

/* Set to 1 to enable debug prints */
#define DEBUG 0
/* Set to 1 to enable timing measurements */
#define TIMING 1

#if TARGET_SIM && PRINTF_IN_SIM
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
#elif PRINTF_IN_FPGA
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

#define TMP_PAD_W (PAD - (2 * PAD + IW) - STRIDES * (N_PATCHES_W - 1) - FW)
#define TMP_PAD_H (PAD - (2 * PAD + IH) - STRIDES * (N_PATCHES_H - 1) - FH)

// Computations for 2D DMA
#define SRC_INC_D2 (STRIDES * IW - (FW - 1 + (STRIDES - 1) * (FW - 1)))

int im2col_nchw_int32(uint8_t test_id, unsigned int *cycles);
int im2col_nhwc_int32(uint8_t test_id, unsigned int *cycles);

int get_index(int dim1, int dim2, int dim3, int index0, int index1, int index2, int index3);
                
int verify(int format);

#endif