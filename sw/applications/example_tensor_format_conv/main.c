/*
 *  Copyright EPFL contributors.
 *  Licensed under the Apache License, Version 2.0, see LICENSE for details.
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *                         <tommaso.terzano@gmail.com>
 *  
 *  Info: This application converts a HWC tensor to a CHW tensor and viceversa, leveraging the DMA subsytem.
 * 
 *        This is a simple example of a HWC tensor with 3 channels, 2 rows and 2 columns:
 *             (1, 2, 3) (4, 5, 6)
 *             (7, 8, 9) (10, 11, 12)
 *        (1, 2, 3) are the values of the first "pixel" of the tensor across different channels, 1 for CH0, 2 for CH1 and 3 for CH2.
 *        
 *        On the other hand, this is the same tensor represented with the CHW format:
 * 
 *             (1) (4)
 *             (7) (10)
 *             (2) (5)
 *             (8) (11)
 *             (3) (6)
 *             (9) (12)
 * 
 *        (1) is the first pixel of CH0, which is composed of the values 1, 4, 7, 10, and so on.
 * 
 *        The conversion hwc -> chw is performed in this manner:
 *        - Copy the first element (1)
 *        - Skip #channels elements
 *        - Copy the element (4) 
 *        And so on.
 * 
 *        To speed the conversion up, the application makes use of the DMA.
 *        Its transposition function is used in an unconventional way: the d1 increment is set to #channels, while the d2 increment
 *        is set to 1. 
 *        In this way, the DMA will copy H*W elements with a C stride, then start the next copy from the first element + 1, 
 *        and repeat until completion.
 *        This behaviour is similar to a 3D copy, because it performs a copy of a matrix (H*W) with a stride (C), even if 
 *        using just 2 counters. 
 * 
 *        The conversion chw -> hwc is performed in a similar way, but it needs H loops, making it a lot slower than the other type
 *        of conversion.  
 */

#include <stdio.h>
#include <stdlib.h>
#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "rv_plic.h"

/* Uncomment to disable performance analysis */
#define EN_PERF

#define C 3
#define H 4
#define W 3

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

/* CHW example array */
int chw_array[36] = {
    // Channel 1
    7,  8,  5,    // Row 1
    9,  3,  6,    // Row 2
    2,  4,  1,    // Row 3
    8, 10, 12,    // Row 4

    // Channel 2
    15, 16, 13,   // Row 1
    19, 11, 14,   // Row 2
    10, 18, 17,   // Row 3
    20, 22, 21,   // Row 4

    // Channel 3
    23, 25, 24,   // Row 1
    28, 26, 27,   // Row 2
    29, 31, 30,   // Row 3
    32, 34, 33    // Row 4
};

/* HWC example array */
int hwc_array[36] = {
    // Row 1
    7,  15, 23,   // W1C1, W1C2, W1C3
    8,  16, 25,   // W2C1, W2C2, W2C3
    5,  13, 24,   // W3C1, W3C2, W3C3

    // Row 2
    9,  19, 28,   // W1C1, W1C2, W1C3
    3,  11, 26,   // W2C1, W2C2, W2C3
    6,  14, 27,   // W3C1, W3C2, W3C3

    // Row 3
    2,  10, 29,   // W1C1, W1C2, W1C3
    4,  18, 31,   // W2C1, W2C2, W2C3
    1,  17, 30,   // W3C1, W3C2, W3C3

    // Row 4
    8,  20, 32,   // W1C1, W1C2, W1C3
    10, 22, 34,   // W2C1, W2C2, W2C3
    12, 21, 33    // W3C1, W3C2, W3C3
};

int convert_hwc_to_chw_int32(int *src, int *dst)
{
    #ifdef EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    static dma_target_t tgt_src;
    static dma_target_t tgt_dst;
    static dma_trans_t trans;
    int res_valid, res_load, res_launch, cycles_dma;

    /* Initialize the targets */
    tgt_src.ptr = (uint8_t *) src;
    tgt_src.inc_d1_du = 1;
    tgt_src.inc_d2_du = C;
    tgt_src.trig = DMA_TRIG_MEMORY;
    tgt_src.type = DMA_DATA_TYPE_WORD;
    
    tgt_dst.ptr = (uint8_t *)  dst;
    tgt_dst.inc_d1_du = 1;
    tgt_dst.inc_d2_du = 1;
    tgt_dst.trig = DMA_TRIG_MEMORY;
    tgt_dst.type = DMA_DATA_TYPE_WORD;

    trans.src = &tgt_src;
    trans.dst = &tgt_dst;
    trans.mode = DMA_TRANS_MODE_SINGLE;
    trans.dim = DMA_DIM_CONF_2D;
    trans.dim_inv        = 1; // Enable transposition function
    trans.size_d1_du     = H * W;
    trans.size_d2_du     = C;
    trans.win_du         = 0,
    trans.end            = DMA_TRANS_END_INTR_WAIT;

    dma_init(NULL);

    res_valid = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    res_load = dma_load_transaction(&trans);
    res_launch = dma_launch(&trans);
    if (res_launch | res_load | res_valid) {
        PRINTF("Error in the DMA transaction! %d - %d - %d\n", res_valid, res_load, res_launch);
        exit(1);
    }
    
    #ifdef EN_PERF

    /* Read the cycles count after the DMA run */
    CSR_READ(CSR_REG_MCYCLE, &cycles_dma);
    return cycles_dma;
    #endif

    return 0;
}

int convert_chw_to_hwc_int32(int *src, int *dst)
{
    static dma_target_t tgt_src;
    static dma_target_t tgt_dst;
    static dma_trans_t trans;
    int res_valid, res_load, res_launch, cycles_dma;
    int * src_ptr = src;
    int * dst_ptr = dst;

    #ifdef EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    dma_init(NULL);

    for (int i=0; i < H; i++) {

      /* Initialize the targets */
      tgt_src.ptr = (uint8_t *) src_ptr;
      tgt_src.inc_d1_du = 1;
      tgt_src.inc_d2_du = H * W;
      tgt_src.trig = DMA_TRIG_MEMORY;
      tgt_src.type = DMA_DATA_TYPE_WORD;

      tgt_dst.ptr = (uint8_t *) dst_ptr;
      tgt_dst.inc_d1_du = 1;
      tgt_dst.inc_d2_du = 1;
      tgt_dst.trig = DMA_TRIG_MEMORY;
      tgt_dst.type = DMA_DATA_TYPE_WORD;

      trans.src = &tgt_src;
      trans.dst = &tgt_dst;
      trans.mode = DMA_TRANS_MODE_SINGLE;
      trans.dim = DMA_DIM_CONF_2D;
      trans.dim_inv        = 1; // Enable transposition function
      trans.size_d1_du     = C;
      trans.size_d2_du     = W;
      trans.win_du         = 0,
      trans.end            = DMA_TRANS_END_INTR_WAIT;

      res_valid = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
      res_load = dma_load_transaction(&trans);
      res_launch = dma_launch(&trans);
      if (res_launch | res_load | res_valid) {
        PRINTF("Error in the DMA transaction! %d - %d - %d\n", res_valid, res_load, res_launch);
        exit(1);
      }

      src_ptr += W;
      dst_ptr += W * C;
    }

    #ifdef EN_PERF

    /* Read the cycles count after the DMA run */
    CSR_READ(CSR_REG_MCYCLE, &cycles_dma);
    return cycles_dma;
    #endif

    return 0;
}

int main()
{ 
    int cycles_dma;
    int dst_array[36];
    int passed_chw = 1;
    int passed_hwc = 1;

    /* Convert HWC to CHW */
    cycles_dma = convert_hwc_to_chw_int32(hwc_array, dst_array);

    #ifdef EN_PERF
    PRINTF("DMA cycles HWC -> CHW: %d\n\n\r", cycles_dma);
    #endif

    /* Verify that the computed and the expected outputs are the same */
    for (int i = 0; i < H * W * C; i++) {
        if (chw_array[i] != dst_array[i]) {
            passed_chw = 0;
        }
    }

    /* Convert CHW to HWC */
    cycles_dma = convert_chw_to_hwc_int32(chw_array, dst_array);

    #ifdef EN_PERF
    PRINTF("DMA cycles CHW -> HWC: %d\n\n\r", cycles_dma);
    #endif

    /* Verify that the computed and the expected outputs are the same */
    for (int i = 0; i < H * W * C; i++) {
        if (hwc_array[i] != dst_array[i]) {
            passed_hwc = 0;
        }
    }

    if (passed_hwc && passed_chw) {
        PRINTF("Success\n\n\r");
    } 
    else 
    {
        if (!passed_hwc) {
            PRINTF("Fail HWC -> CHW\n\r");
        }
        if (!passed_chw) {
            PRINTF("Fail CHW -> HWC\n\r");
        }
        return EXIT_FAILURE;
    }

    return 0;
}
