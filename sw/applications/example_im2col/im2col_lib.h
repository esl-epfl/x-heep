#ifndef _IM2COL_
#define _IM2COL_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "im2colGolden.h"
#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "rv_plic.h"

/* 
    WORK IN PROGRESS!
    Used to choose between several HW configurations:
    - 0: Only CPU
    - 1: Exploit standard DMA
    - 2: Exploit 2D DMA
*/ 
#define HW_CONFIG 0

// By default, printfs are activated for FPGA and disabled for simulation.
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0
#define TARGET_PYNQ_Z2  1
#define DEBUG 0 // Set to 1 to enable debug prints
#define TIMING 0 // Set to 1 to enable timing measurements

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
        #define PRINTF_DEB(...)    
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
    #if DEBUG
        #define PRINTF_DEB(fmt, ...)    printf(fmt, ## __VA_ARGS__)
    #else
        #define PRINTF_DEB(...)
    #endif
#else
    #define PRINTF(...)
    #define PRINTF_DEB(...)
#endif

// Define the dimensions of the input tensor and the kernel

#define N_PATCHES_H (IH + (PAD + PAD) - FH)/ STRIDES + 1
#define N_PATCHES_W (IW + (PAD + PAD) - FW)/ STRIDES + 1
#define OH FW * FH * CH * BATCH
#define OW (N_PATCHES_W) * (N_PATCHES_H)

int im2col_nchw_int32();
int im2col_nhwc_int32();

int32_t get_index(int32_t dim1, int32_t dim2, int32_t dim3, int32_t index0, int32_t index1, int32_t index2, int32_t index3);
                
uint16_t verify();

#endif