/*
 *  Copyright EPFL contributors.
 *  Licensed under the Apache License, Version 2.0, see LICENSE for details.
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *                         <tommaso.terzano@gmail.com>
 *  
 *  Info: Example application of matrix manipulation by exploiting the 2D DMA.
 *        In this code, there are some optional features:
 *        - Verification of several matrix operations carried out by the 2D DMA
 *        - Performance comparison between the DMA and the CPU, obtained by performing similar matrix operations
 *          and monitoring the performance counter.
 */

#include <stdio.h>
#include <stdlib.h>
#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "rv_plic.h"
#include "test_data.h"

/*  
 *  This code contains four different tests that can be run by defining the corresponding TEST_ID_* macro.
 *  - Extract a NxM matrix, perform optional padding and copy it to a AxB matrix, using HALs
 *  - Extract a NxM matrix and copy its transposed version to AxB matrix, using HALs
 *  - Extract a 1xN matrix (array), perform optional padding and copy it to an array, using HALs
 *  - Extract a NxM matrix, perform optional padding and copy it to a AxB matrix, using direct register operations
 */

#define TEST_ID_0
#define TEST_ID_1
#define TEST_ID_2
#define TEST_ID_3

/* Enable performance analysis */
#define EN_PERF 1

/* Enable verification */
#define EN_VERIF 1

/* Parameters */

/* Size of the extracted matrix (including strides on the input, excluding strides on the outputs) */
#define SIZE_EXTR_D1 3
#define SIZE_EXTR_D2 3

/* Set strides of the input ad output matrix */
#define STRIDE_IN_D1 1
#define STRIDE_IN_D2 1
#define STRIDE_OUT_D1 1
#define STRIDE_OUT_D2 1

/* Set the padding parameters */
#define TOP_PAD 1
#define BOTTOM_PAD 1
#define LEFT_PAD 1
#define RIGHT_PAD 1

/* Macros for dimensions computation */
#define OUT_D1_PAD ( SIZE_EXTR_D1 + LEFT_PAD + RIGHT_PAD )
#define OUT_D2_PAD ( SIZE_EXTR_D2  + TOP_PAD + BOTTOM_PAD )
#define OUT_D1_PAD_STRIDE ( (OUT_D1_PAD * STRIDE_OUT_D1) - (STRIDE_OUT_D1 - 1)  )
#define OUT_D2_PAD_STRIDE ( (OUT_D2_PAD * STRIDE_OUT_D2) - (STRIDE_OUT_D2 - 1)  )
#define OUT_DIM_1D ( OUT_D1_PAD_STRIDE  )
#define OUT_DIM_2D ( OUT_D1_PAD_STRIDE * OUT_D2_PAD_STRIDE )

/* Mask for the direct register operations example */
#define DMA_CSR_REG_MIE_MASK (( 1 << 19 ) | (1 << 11 ))

/* Transposition example def */
#define TRANSPOSITION_EN 1

/* Enables test format */
#define TEST_EN 0

/* Define the input datatype */
typedef uint32_t dma_input_data_type;

/* Pointer increments computation */
#define SRC_INC_D1 STRIDE_IN_D1
#define DST_INC_D1 STRIDE_OUT_D1
#define SRC_INC_D2 (STRIDE_IN_D2 * SIZE_IN_D1 - (SIZE_EXTR_D1 - 1 + (STRIDE_IN_D1 - 1) * (SIZE_EXTR_D1 - 1)))
#define DST_INC_D2 ((STRIDE_OUT_D2 - 1) * OUT_DIM_1D + 1)
#define SRC_INC_TRSP_D1 SRC_INC_D1
#define SRC_INC_TRSP_D2 (STRIDE_IN_D2 * SIZE_IN_D1)

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

dma_input_data_type copied_data_2D_DMA[OUT_DIM_2D];
dma_input_data_type copied_data_1D_DMA[OUT_DIM_1D];
dma_input_data_type copied_data_2D_CPU[OUT_DIM_2D];
dma_input_data_type copied_data_1D_CPU[OUT_DIM_2D];

dma_config_flags_t res_valid, res_load, res_launch;

dma *peri = dma_peri(0);

dma_target_t tgt_src;
dma_target_t tgt_dst;
dma_trans_t trans;

uint32_t dst_ptr = 0, src_ptr = 0;
uint32_t cycles_dma, cycles_cpu;
uint32_t size_dst_trans_d1;
uint32_t dst_stride_d1;
uint32_t dst_stride_d2;
uint32_t size_src_trans_d1;
uint32_t src_stride_d1;
uint32_t src_stride_d2;
uint32_t i_in;
uint32_t j_in;
uint32_t i_in_last;
uint16_t left_pad_cnt = 0;
uint16_t top_pad_cnt = 0;
uint8_t stride_1d_cnt = 0;
uint8_t stride_2d_cnt = 0;
char passed = 1;

int main()
{    
    #ifdef TEST_ID_0

    /* Testing copy and padding of a NxM matrix using HALs */
    
    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    timer_cycles_init();

    #endif

    tgt_src.ptr = (uint8_t *) test_data;
    tgt_src.inc_d1_du = SRC_INC_D1;
    tgt_src.inc_d2_du = SRC_INC_D2;
    tgt_src.trig = DMA_TRIG_MEMORY;
    tgt_src.type = DMA_DATA_TYPE;
    
    tgt_dst.ptr = (uint8_t *)  copied_data_2D_DMA;
    tgt_dst.inc_d1_du = DST_INC_D1;
    tgt_dst.inc_d2_du = DST_INC_D2;
    tgt_dst.trig = DMA_TRIG_MEMORY;
    tgt_dst.type = DMA_DATA_TYPE;

    trans.src = &tgt_src;
    trans.dst = &tgt_dst;
    trans.mode = DMA_TRANS_MODE_SINGLE;
    trans.dim = DMA_DIM_CONF_2D;
    trans.pad_top_du     = TOP_PAD,
    trans.pad_bottom_du  = BOTTOM_PAD,
    trans.pad_left_du    = LEFT_PAD,
    trans.pad_right_du   = RIGHT_PAD,
    trans.size_d1_du     = SIZE_EXTR_D1;
    trans.size_d2_du     = SIZE_EXTR_D2;
    trans.win_du         = 0,
    trans.end            = DMA_TRANS_END_INTR;

    dma_init(NULL);
    
    timer_start();
    
    #if EN_PERF

    res_valid = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    res_load = dma_load_transaction(&trans);
    res_launch = dma_launch(&trans);
    
    #else

    res_valid = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res_valid, res_valid == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_load = dma_load_transaction(&trans);
    PRINTF("load: %u \t%s\n\r", res_load, res_load == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_launch = dma_launch(&trans);
    PRINTF("laun: %u \t%s\n\r", res_launch, res_launch == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    #endif

    while( ! dma_is_ready(0)) {
        /* Disable_interrupts */
        /* This does not prevent waking up the core as this is controlled by the MIP register */
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
            /* From here the core wakes up even if we did not jump to the ISR */
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    #if EN_PERF    

    /* Read the cycles count after the DMA run */
    cycles_dma = timer_stop();

    /* Reset the performance counter to evaluate the CPU performance */
    timer_cycles_init();    
    timer_start();
    #endif

    #if EN_VERIF

    /* Run the same computation on the CPU */
    for (int i=0; i < OUT_D2_PAD_STRIDE; i++)
    {
        stride_1d_cnt = 0;
        j_in = 0;

        for (int j=0; j < OUT_D1_PAD_STRIDE; j++)
        {
            dst_ptr = i * OUT_D1_PAD_STRIDE + j;
            src_ptr = (i_in - top_pad_cnt ) * STRIDE_IN_D2 * SIZE_IN_D1 + (j_in - left_pad_cnt) * STRIDE_IN_D1;
            if (i_in < TOP_PAD || i_in >= SIZE_EXTR_D2 + TOP_PAD || j_in < LEFT_PAD || j_in >= SIZE_EXTR_D1 + LEFT_PAD ||
                stride_1d_cnt != 0 || stride_2d_cnt != 0)
            {
                copied_data_2D_CPU[dst_ptr] = 0;
            }
            else
            {
                copied_data_2D_CPU[dst_ptr] = test_data[src_ptr];
            }

            if (j_in < LEFT_PAD && i_in >= TOP_PAD && stride_1d_cnt == 0 && stride_2d_cnt == 0)
            {
                left_pad_cnt++;
            }

            if (stride_1d_cnt == STRIDE_OUT_D1 - 1)
            {
                stride_1d_cnt = 0;
                j_in++;
            }
            else
            {
                stride_1d_cnt++;
            }

        }

        if (i_in < TOP_PAD && stride_2d_cnt == 0)
        {
            top_pad_cnt++;
        }
        
        if (stride_2d_cnt == STRIDE_OUT_D2 - 1)
        {
            stride_2d_cnt = 0;
            i_in++;
        }
        else
        {
            stride_2d_cnt++;
        }

        left_pad_cnt = 0;
    }
    #endif

    #if EN_PERF

    /* Read the cycles count after the CPU run */
    cycles_cpu = timer_stop();
    
    #if TEST_EN == 0
    PRINTF("Total number of cycles CPU: [%d]\n\r", cycles_cpu);
    PRINTF("Total number of cycles DMA: [%d]\n\r", cycles_dma);
    #endif

    #endif

    #if EN_VERIF
    
    /* Verify that the DMA and the CPU outputs are the same */
    for (int i = 0; i < OUT_D2_PAD_STRIDE; i++) {
        for (int j = 0; j < OUT_D1_PAD_STRIDE; j++) {
            if (copied_data_2D_DMA[i * OUT_D1_PAD_STRIDE + j] != copied_data_2D_CPU[i * OUT_D1_PAD_STRIDE + j]) {
                passed = 0;
            }
        }
    }

    if (passed) {
        #if TEST_EN == 0
        PRINTF("TEST 0 PASSED!\n\r\n\r");
        #else
        PRINTF("0a:%d:0\n\r", cycles_cpu);   
        PRINTF("0b:%d:0\n\r", cycles_dma);               
        #endif
    } 
    else 
    {
        #if TEST_EN == 0
        PRINTF("TEST 0 FAILED\n\r");
        #else
        PRINTF("0a:%d:1\n\r", cycles_cpu); 
        PRINTF("0b:%d:1\n\r", cycles_dma); 
        #endif
        return EXIT_FAILURE;
    }
    #endif

    #endif
    
    /* Reset for second test */
    passed = 1;
    i_in = 0;
    j_in = 0;
    left_pad_cnt = 0;
    top_pad_cnt = 0;
    stride_1d_cnt = 0;
    stride_2d_cnt = 0;

    for (int i = 0; i < OUT_DIM_2D; i++) {
        copied_data_2D_DMA[i] = 0;
        copied_data_2D_CPU[i] = 0;
    }

    #ifdef TEST_ID_1

    /* Testing transposition and copy of a NxM matrix using HALs */
 
    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    timer_cycles_init();
    #endif

    tgt_src.ptr            = (uint8_t *) test_data;
    tgt_src.inc_d1_du      = SRC_INC_TRSP_D1;
    tgt_src.inc_d2_du      = SRC_INC_TRSP_D2;
    tgt_src.trig           = DMA_TRIG_MEMORY;
    tgt_src.type           = DMA_DATA_TYPE;

    tgt_dst.ptr            = (uint8_t *) copied_data_2D_DMA;
    tgt_dst.inc_d1_du      = DST_INC_D1;
    tgt_dst.inc_d2_du      = DST_INC_D2;
    tgt_dst.trig           = DMA_TRIG_MEMORY;

    trans.src            = &tgt_src;
    trans.dst            = &tgt_dst;
    trans.mode           = DMA_TRANS_MODE_SINGLE;
    trans.dim            = DMA_DIM_CONF_2D;
    trans.pad_top_du     = TOP_PAD;
    trans.pad_bottom_du  = BOTTOM_PAD;
    trans.pad_left_du    = LEFT_PAD;
    trans.pad_right_du   = RIGHT_PAD;
    trans.dim_inv        = TRANSPOSITION_EN;
    trans.size_d1_du     = SIZE_EXTR_D1;
    trans.size_d2_du     = SIZE_EXTR_D2;
    trans.win_du         = 0,
    trans.end            = DMA_TRANS_END_INTR;
    
    dma_init(NULL);
    
    timer_start();
    
    #if EN_PERF

    res_valid = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    res_load = dma_load_transaction(&trans);
    res_launch = dma_launch(&trans);
    
    #else

    res_valid = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res_valid, res_valid == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_load = dma_load_transaction(&trans);
    PRINTF("load: %u \t%s\n\r", res_load, res_load == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_launch = dma_launch(&trans);
    PRINTF("laun: %u \t%s\n\r", res_launch, res_launch == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    #endif

    while( ! dma_is_ready(0)) {
        /* Disable_interrupts */
        /* This does not prevent waking up the core as this is controlled by the MIP register */
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
            /* From here the core wakes up even if we did not jump to the ISR */
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    #if EN_PERF    

    /* Read the cycles count after the DMA run */
    cycles_dma = timer_stop();

    /* Reset the performance counter to evaluate the CPU performance */
    timer_cycles_init();
    timer_start();
    #endif

    #if EN_VERIF

    /* Run the same computation on the CPU */
    for (int i=0; i < OUT_D2_PAD_STRIDE; i++)
    {
        stride_1d_cnt = 0;
        j_in = 0;

        for (int j=0; j < OUT_D1_PAD_STRIDE; j++)
        {
            dst_ptr = i * OUT_D1_PAD_STRIDE + j;
            src_ptr = (j_in - left_pad_cnt) * STRIDE_IN_D2 * SIZE_IN_D1 + (i_in - top_pad_cnt ) * STRIDE_IN_D1;
            if (i_in < TOP_PAD || i_in >= SIZE_EXTR_D2 + TOP_PAD || j_in < LEFT_PAD || j_in >= SIZE_EXTR_D1 + LEFT_PAD ||
                stride_1d_cnt != 0 || stride_2d_cnt != 0)
            {
                copied_data_2D_CPU[dst_ptr] = 0;
            }
            else
            {
                copied_data_2D_CPU[dst_ptr] = test_data[src_ptr];
            }

            if (j_in < LEFT_PAD && i_in >= TOP_PAD && stride_1d_cnt == 0 && stride_2d_cnt == 0)
            {
                left_pad_cnt++;
            }

            if (stride_1d_cnt == STRIDE_OUT_D1 - 1)
            {
                stride_1d_cnt = 0;
                j_in++;
            }
            else
            {
                stride_1d_cnt++;
            }

        }

        if (i_in < TOP_PAD && stride_2d_cnt == 0)
        {
            top_pad_cnt++;
        }
        
        if (stride_2d_cnt == STRIDE_OUT_D2 - 1)
        {
            stride_2d_cnt = 0;
            i_in++;
        }
        else
        {
            stride_2d_cnt++;
        }

        left_pad_cnt = 0;
    }
    #endif

    #if EN_PERF

    /* Read the cycles count after the CPU run */
    cycles_cpu = timer_stop();
    #if TEST_EN == 0
    PRINTF("Total number of cycles CPU: [%d]\n\r", cycles_cpu);
    PRINTF("Total number of cycles DMA: [%d]\n\r", cycles_dma);
    #endif

    #endif

    #if EN_VERIF
    
    /* Verify that the DMA and the CPU outputs are the same */
    for (int i = 0; i < OUT_D2_PAD_STRIDE; i++) {
        for (int j = 0; j < OUT_D1_PAD_STRIDE; j++) {
            if (copied_data_2D_DMA[i * OUT_D1_PAD_STRIDE + j] != copied_data_2D_CPU[i * OUT_D1_PAD_STRIDE + j]) {
                passed = 0;
            }
        }
    }

    if (passed) {
        #if TEST_EN == 0
        PRINTF("TEST 1 PASSED!\n\r\n\r");
        #else
        PRINTF("1a:%d:0\n\r", cycles_cpu);  
        PRINTF("1b:%d:0\n\r", cycles_dma);                
        #endif
    } 
    else 
    {
        #if TEST_EN == 0
        PRINTF("TEST 1 FAILED\n\r");
        #else
        PRINTF("1a:%d:1\n\r", cycles_cpu);
        PRINTF("1b:%d:1\n\r", cycles_dma);  
        #endif
        return EXIT_FAILURE;
    }
    #endif

    #endif
    
    /* Reset for third test */
    passed = 1;
    i_in = 0;
    j_in = 0;
    left_pad_cnt = 0;
    top_pad_cnt = 0;
    stride_1d_cnt = 0;
    stride_2d_cnt = 0;

    #ifdef TEST_ID_2

    /* Testing copy and padding of a 1xN matrix (an array) */

    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    timer_cycles_init();
    #endif

    tgt_src.ptr            = (uint8_t *) test_data;
    tgt_src.inc_d1_du      = SRC_INC_D1;
    tgt_src.inc_d2_du      = 0;
    tgt_src.trig           = DMA_TRIG_MEMORY;
    tgt_src.type           = DMA_DATA_TYPE;

    tgt_dst.ptr            = (uint8_t *) copied_data_1D_DMA;
    tgt_dst.inc_d1_du      = DST_INC_D1;
    tgt_dst.inc_d2_du      = 0;
    tgt_dst.trig           = DMA_TRIG_MEMORY;

    trans.src            = &tgt_src;
    trans.dst            = &tgt_dst;
    trans.mode           = DMA_TRANS_MODE_SINGLE;
    trans.dim            = DMA_DIM_CONF_1D;
    trans.pad_top_du     = 0;
    trans.pad_bottom_du  = 0;
    trans.pad_left_du    = LEFT_PAD;
    trans.pad_right_du   = RIGHT_PAD;
    trans.dim_inv        = 0;
    trans.win_du         = 0;
    trans.size_d1_du     = SIZE_EXTR_D1;
    trans.size_d2_du     = 0;
    trans.end            = DMA_TRANS_END_INTR;

    dma_init(NULL);

    timer_start();
    
    #if EN_PERF

    res_valid = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    res_load = dma_load_transaction(&trans);
    res_launch = dma_launch(&trans);
    
    #else

    res_valid = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res_valid, res_valid == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_load = dma_load_transaction(&trans);
    PRINTF("load: %u \t%s\n\r", res_load, res_load == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_launch = dma_launch(&trans);
    PRINTF("laun: %u \t%s\n\r", res_launch, res_launch == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    #endif

    while( ! dma_is_ready(0)) {
        /* Disable_interrupts */
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    #if EN_PERF

    /* Read the cycles count after the DMA run */
    cycles_dma = timer_stop();

    /* Reset the performance counter to evaluate the CPU performance */
    timer_cycles_init();
    timer_start();
    #endif

    #if EN_VERIF

    /* Run the same computation on the CPU */
    for (int j=0; j < OUT_D1_PAD_STRIDE; j++)
    {
        dst_ptr = j;
        src_ptr = (j_in - left_pad_cnt) * STRIDE_IN_D1;

        if (j_in < LEFT_PAD || j_in >= SIZE_EXTR_D1 + LEFT_PAD ||
            stride_1d_cnt != 0)
        {
            copied_data_1D_CPU[dst_ptr] = 0;
        }
        else
        {
            copied_data_1D_CPU[dst_ptr] = test_data[src_ptr];
        }

        if (j_in < LEFT_PAD && stride_1d_cnt == 0)
        {
            left_pad_cnt++;
        }

        if (stride_1d_cnt == STRIDE_OUT_D1 - 1)
        {
            stride_1d_cnt = 0;
            j_in++;
        }
        else
        {
            stride_1d_cnt++;
        }
    }
    
    #endif

    #if EN_PERF

    /* Read the cycles count after the CPU run */
    cycles_cpu = timer_stop();
    #if TEST_EN == 0
    PRINTF("Total number of cycles CPU: [%d]\n\r", cycles_cpu);
    PRINTF("Total number of cycles DMA: [%d]\n\r", cycles_dma);
    #endif

    #endif

    #if EN_VERIF
    
    /* Verify that the DMA and the CPU outputs are the same */
    for (int i = 0; i < OUT_D1_PAD_STRIDE; i++) {
        if (copied_data_1D_DMA[i] != copied_data_1D_CPU[i]) {
            passed = 0;
        }
    }

    if (passed) {
        #if TEST_EN == 0
        PRINTF("TEST 2 PASSED!\n\r\n\r");
        #else
        PRINTF("2a:%d:0\n\r", cycles_cpu);  
        PRINTF("2b:%d:0\n\r", cycles_dma);                
        #endif
    } 
    else 
    {
        #if TEST_EN == 0
        PRINTF("TEST 2 FAILED\n\r");
        #else
        PRINTF("2a:%d:1\n\r", cycles_cpu);
        PRINTF("2b:%d:1\n\r", cycles_dma);  
        #endif
        return EXIT_FAILURE;
    }
    #endif

    #endif
    
    /* Reset for fourth test */
    passed = 1;
    i_in = 0;
    j_in = 0;
    left_pad_cnt = 0;
    top_pad_cnt = 0;
    stride_1d_cnt = 0;
    stride_2d_cnt = 0;
    for (int i = 0; i < OUT_DIM_2D; i++) {
        copied_data_2D_DMA[i] = 0;
        copied_data_2D_CPU[i] = 0;
    }

    #ifdef TEST_ID_3

    /* Testing copy and padding of a NxM matrix using direct register operations.
     * This strategy allows for maximum performance but doesn't perform any checks on the data integrity.
     */
    
    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    timer_cycles_init();
    #endif

    /* The DMA is initialized (i.e. Any current transaction is cleaned.) */
    dma_init(NULL);

    timer_start();

    /* Enable the DMA interrupt logic */
    write_register( 0x1,
                    DMA_INTERRUPT_EN_REG_OFFSET,
                    0xffff,
                    DMA_INTERRUPT_EN_TRANSACTION_DONE_BIT,
                    peri );

    /* Enable global interrupts */
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    /* Enable fast interrupts */
    CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK);

    /* Pointer set up */
    peri->SRC_PTR = (uint32_t) (test_data);
    peri->DST_PTR = (uint32_t) (copied_data_2D_DMA);

    /* Dimensionality configuration */
    write_register( 0x1,
                    DMA_DIM_CONFIG_REG_OFFSET,
                    0xffff,
                    DMA_DIM_CONFIG_DMA_DIM_BIT,
                    peri );

    /* Operation mode configuration */
    write_register( DMA_TRANS_MODE_SINGLE,
                    DMA_MODE_REG_OFFSET,
                    DMA_MODE_MODE_MASK,
                    DMA_MODE_MODE_OFFSET,
                    peri );
    
    /* Data type configuration */
    write_register( DMA_DATA_TYPE,
                    DMA_DST_DATA_TYPE_REG_OFFSET,
                    DMA_DST_DATA_TYPE_DATA_TYPE_MASK,
                    DMA_DST_DATA_TYPE_DATA_TYPE_OFFSET,
                    peri );
    write_register( DMA_DATA_TYPE,
                    DMA_SRC_DATA_TYPE_REG_OFFSET,
                    DMA_SRC_DATA_TYPE_DATA_TYPE_MASK,
                    DMA_SRC_DATA_TYPE_DATA_TYPE_OFFSET,
                    peri );

    /* Set the source strides */
    write_register( SRC_INC_D1 * DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE),
                    DMA_SRC_PTR_INC_D1_REG_OFFSET,
                    DMA_SRC_PTR_INC_D1_INC_MASK,
                    DMA_SRC_PTR_INC_D1_INC_OFFSET,
                    peri );
    
    write_register( SRC_INC_D2 * DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE),
                    DMA_SRC_PTR_INC_D2_REG_OFFSET,
                    DMA_SRC_PTR_INC_D2_INC_MASK,
                    DMA_SRC_PTR_INC_D2_INC_OFFSET,
                    peri );
    
    write_register( DST_INC_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_DST_PTR_INC_D1_REG_OFFSET,
                    DMA_DST_PTR_INC_D1_INC_MASK,
                    DMA_DST_PTR_INC_D1_INC_OFFSET,
                    peri );
    
    write_register( DST_INC_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_DST_PTR_INC_D2_REG_OFFSET,
                    DMA_DST_PTR_INC_D2_INC_MASK,
                    DMA_DST_PTR_INC_D2_INC_OFFSET,
                    peri );

    /* Padding configuration */
    write_register( TOP_PAD,
                    DMA_PAD_TOP_REG_OFFSET,
                    DMA_PAD_TOP_PAD_MASK,
                    DMA_PAD_TOP_PAD_OFFSET,
                    peri );

    write_register( RIGHT_PAD,
                    DMA_PAD_RIGHT_REG_OFFSET,
                    DMA_PAD_RIGHT_PAD_MASK,
                    DMA_PAD_RIGHT_PAD_OFFSET,
                    peri );

    write_register( LEFT_PAD,
                    DMA_PAD_LEFT_REG_OFFSET,
                    DMA_PAD_LEFT_PAD_MASK,
                    DMA_PAD_LEFT_PAD_OFFSET,
                    peri );

    write_register( BOTTOM_PAD,
                    DMA_PAD_BOTTOM_REG_OFFSET,
                    DMA_PAD_BOTTOM_PAD_MASK,
                    DMA_PAD_BOTTOM_PAD_OFFSET,
                    peri );

    /* Set the sizes */

    write_register( SIZE_EXTR_D2,
                    DMA_SIZE_D2_REG_OFFSET,
                    DMA_SIZE_D2_SIZE_MASK,
                    DMA_SIZE_D2_SIZE_OFFSET,
                    peri );

    write_register( SIZE_EXTR_D1,
                    DMA_SIZE_D1_REG_OFFSET,
                    DMA_SIZE_D1_SIZE_MASK,
                    DMA_SIZE_D1_SIZE_OFFSET,
                    peri );

    while( ! dma_is_ready(0)) {
        /* Disable_interrupts */
        /* This does not prevent waking up the core as this is controlled by the MIP register */
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
            /* From here the core wakes up even if we did not jump to the ISR */
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    #if EN_PERF

    /* Read the cycles count after the DMA run */
    cycles_dma = timer_stop();

    /* Reset the performance counter to evaluate the CPU performance */
    timer_cycles_init();
    timer_start();
    #endif

    #if EN_VERIF

    /* Run the same computation on the CPU */
    for (int i=0; i < OUT_D2_PAD_STRIDE; i++)
    {
        stride_1d_cnt = 0;
        j_in = 0;

        for (int j=0; j < OUT_D1_PAD_STRIDE; j++)
        {
            dst_ptr = i * OUT_D1_PAD_STRIDE + j;
            src_ptr = (i_in - top_pad_cnt ) * STRIDE_IN_D2 * SIZE_IN_D1 + (j_in - left_pad_cnt) * STRIDE_IN_D1;
            if (i_in < TOP_PAD || i_in >= SIZE_EXTR_D2 + TOP_PAD || j_in < LEFT_PAD || j_in >= SIZE_EXTR_D1 + LEFT_PAD ||
                stride_1d_cnt != 0 || stride_2d_cnt != 0)
            {
                copied_data_2D_CPU[dst_ptr] = 0;
            }
            else
            {
                copied_data_2D_CPU[dst_ptr] = test_data[src_ptr];
            }

            if (j_in < LEFT_PAD && i_in >= TOP_PAD && stride_1d_cnt == 0 && stride_2d_cnt == 0)
            {
                left_pad_cnt++;
            }

            if (stride_1d_cnt == STRIDE_OUT_D1 - 1)
            {
                stride_1d_cnt = 0;
                j_in++;
            }
            else
            {
                stride_1d_cnt++;
            }

        }

        if (i_in < TOP_PAD && stride_2d_cnt == 0)
        {
            top_pad_cnt++;
        }
        
        if (stride_2d_cnt == STRIDE_OUT_D2 - 1)
        {
            stride_2d_cnt = 0;
            i_in++;
        }
        else
        {
            stride_2d_cnt++;
        }

        left_pad_cnt = 0;
    }
    
    #endif

    #if EN_PERF

    /* Read the cycles count after the CPU run */
    cycles_cpu = timer_stop();
    #if TEST_EN == 0
    PRINTF("Total number of cycles CPU: [%d]\n\r", cycles_cpu);
    PRINTF("Total number of cycles DMA: [%d]\n\r", cycles_dma);
    #endif

    #endif

    #if EN_VERIF
    
    /* Verify that the DMA and the CPU outputs are the same */
    if (passed) {
        #if TEST_EN == 0
        PRINTF("TEST 3 PASSED!\n\r\n\r");
        #else
        PRINTF("3a:%d:0\n\r", cycles_cpu);  
        PRINTF("3b:%d:0\n\r", cycles_dma);                
        #endif
    } 
    else 
    {
        #if TEST_EN == 0
        PRINTF("TEST 3 FAILED\n\r");
        #else
        PRINTF("3a:%d:1\n\r", cycles_cpu);
        PRINTF("3b:%d:1\n\r", cycles_dma);  
        #endif
        return EXIT_FAILURE;
    }
    #endif

    #endif

    #if TEST_EN
    PRINTF("&\n\r");
    #endif

    return EXIT_SUCCESS;
}