/*
    Copyright EPFL contributors.
    Licensed under the Apache License, Version 2.0, see LICENSE for details.
    SPDX-License-Identifier: Apache-2.0

    Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *                         <tommaso.terzano@gmail.com>
    
    Info: Header file of im2col_lib.c, containing the function prototypes, parameters macros and the configuration of prints and performance analysis.
*/

#ifndef _IM2COL_
#define _IM2COL_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "im2col_golden.h"
#include "im2col_input.h"
#include "im2col.h"
#include "dma.h"
#include "im2col_spc_regs.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "rv_plic.h"
#include "csr.h"
#include <math.h>

#include "mmio.h"
#include "handler.h"
#include "hart.h"
#include "fast_intr_ctrl.h"
#include "timer_sdk.h"

/* 
    Choose between several HW configurations:
    - 0: Only CPU
    - 1: Exploit standard DMA
    - 2: Exploit 2D DMA
    - 3: Exploit Smart Peripheral Controller (SPC)
*/ 
#define HW_CONFIG_CPU
#define HW_CONFIG_DMA_2D
#define HW_CONFIG_SPC

/* Defines which DMA channels are available to the SPC, depending on HW specifications */
#define SPC_CH_MASK 0b0001 

/* Defines the datatype of the input */
#define INPUT_DATATYPE 0

/* Defines the offset of the im2col SPC in the system (0x4000 in testharness.sv) */
#define IM2COL_PER_OFFSET 0x4000

/* By default, printfs are activated for FPGA and for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

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

#define N_PATCHES_H ((IH + (TOP_PAD + BOTTOM_PAD) - FH)/ STRIDE_D2 + 1)
#define N_PATCHES_W ((IW + (RIGHT_PAD + LEFT_PAD) - FW)/ STRIDE_D1 + 1)

#define ADPT_PAD_BOTTOM (STRIDE_D2 * (N_PATCHES_H - 1) + FH - (TOP_PAD + IH))
#define ADPT_PAD_RIGHT (STRIDE_D1 * (N_PATCHES_W - 1) + FW - (LEFT_PAD + IW))

#define CH_COL (CH * FH * FW)

#define OH_NCHW (CH * FH * FW * BATCH)
#define OW_NCHW (N_PATCHES_H) * (N_PATCHES_W)

#define START_ID 0

#define TEST_EN 0

// Computations for 2D DMA
#define SRC_INC_D2 (STRIDE_D2 * IW - (FW - 1 + (STRIDE_D1 - 1) * (FW - 1)))

int im2col_nchw_int32(uint8_t test_id, unsigned int *cycles);

__attribute__((weak, optimize("O0"))) void handler_irq_im2col_spc(void);

int get_index(int dim1, int dim2, int dim3, int index0, int index1, int index2, int index3);
                
int verify(void);

#endif
