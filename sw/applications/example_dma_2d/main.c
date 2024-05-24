/*
 *  Copyright EPFL contributors.
 *  Licensed under the Apache License, Version 2.0, see LICENSE for details.
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
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
 *  Select which test to run:
 *  0: Extract a NxM matrix, perform optional padding and copy it to a AxB matrix, using HALs
 *  1: Extract a NxM matrix and copy its transposed version to AxB matrix, using HALs
 *  2: Extract a 1xN matrix (array), perform optional padding and copy it to an array, using HALs
 *  3: Extract a NxM matrix, perform optional padding and copy it to a AxB matrix, using direct register operations
 */

#define TEST_ID 0

/* Enable performance analysis */
#define EN_PERF 1

/* Enable verification */
#define EN_VERIF 1

/* Parameters */

/* Size of the extracted matrix (including strides on the input, excluding strides on the outputs) */
#define SIZE_EXTR_D1 10
#define SIZE_EXTR_D2 10

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
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
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
uint32_t j_in_last;
uint16_t left_pad_cnt = 0;
uint16_t top_pad_cnt = 0;
uint8_t stride_1d_cnt = 0;
uint8_t stride_2d_cnt = 0;
char passed = 1;

#if TEST_ID == 3

/* Function used to simplify register operations */
static inline volatile void write_register( uint32_t  p_val,
                                uint32_t  p_offset,
                                uint32_t  p_mask,
                                uint8_t   p_sel,
                                dma* peri ) 
{
    /*
     * The index is computed to avoid needing to access the structure
     * as a structure.
     */
    uint8_t index = p_offset / sizeof(int);

    /*
     * An intermediate variable "value" is used to prevent writing twice into
     * the register.
     */
    uint32_t value  =  (( uint32_t * ) peri ) [ index ];
    value           &= ~( p_mask << p_sel );
    value           |= (p_val & p_mask) << p_sel;
    (( uint32_t * ) peri ) [ index ] = value;
};
#endif

int main()
{    
    #if TEST_ID == 0

    /* Testing copy and padding of a NxM matrix using HALs */
    
    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    dma_target_t tgt_src = {
                                .ptr            = &test_data[0],
                                .inc_du         = SRC_INC_D1,
                                .inc_d2_du      = SRC_INC_D2,
                                .size_du        = SIZE_EXTR_D1,
                                .size_d2_du     = SIZE_EXTR_D2,
                                .trig           = DMA_TRIG_MEMORY,
                                .type           = DMA_DATA_TYPE,
                            };

    dma_target_t tgt_dst = {
                                .ptr            = copied_data_2D_DMA,
                                .inc_du         = DST_INC_D1,
                                .inc_d2_du      = DST_INC_D2,
                                .trig           = DMA_TRIG_MEMORY,
                            };

    dma_trans_t trans =     {
                                .src            = &tgt_src,
                                .dst            = &tgt_dst,
                                .mode           = DMA_TRANS_MODE_SINGLE,
                                .dim            = DMA_DIM_CONF_2D,
                                .pad_top_du     = TOP_PAD,
                                .pad_bottom_du  = BOTTOM_PAD,
                                .pad_left_du    = LEFT_PAD,
                                .pad_right_du   = RIGHT_PAD,
                                .win_du         = 0,
                                .end            = DMA_TRANS_END_INTR
                            };
    
    dma_init(NULL);
    
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
        #if !EN_PERF
        /* Disable_interrupts */
        /* This does not prevent waking up the core as this is controlled by the MIP register */
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
            /* From here the core wakes up even if we did not jump to the ISR */
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
        #endif
    }

    #if EN_PERF    

    /* Read the cycles count after the DMA run */
    CSR_READ(CSR_REG_MCYCLE, &cycles_dma);

    /* Reset the performance counter to evaluate the CPU performance */
    CSR_SET_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
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
    CSR_READ(CSR_REG_MCYCLE, &cycles_cpu);
    PRINTF("DMA cycles: %d\n\r", cycles_dma);
    PRINTF("CPU cycles: %d \n\r", cycles_cpu);
    PRINTF("\n\r");

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
        PRINTF("Success\n\r");
        return EXIT_SUCCESS;
    } 
    else 
    {
        PRINTF("Fail\n\r");
        return EXIT_FAILURE;
    }
    #endif

    #elif TEST_ID == 1

    /* Testing transposition and copy of a NxM matrix using HALs */
 
    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    dma_target_t tgt_src = {
                                .ptr            = &test_data[0],
                                .inc_du         = SRC_INC_TRSP_D1,
                                .inc_d2_du      = SRC_INC_TRSP_D2,
                                .size_du        = SIZE_EXTR_D1,
                                .size_d2_du     = SIZE_EXTR_D2,
                                .trig           = DMA_TRIG_MEMORY,
                                .type           = DMA_DATA_TYPE,
                            };

    dma_target_t tgt_dst = {
                                .ptr            = copied_data_2D_DMA,
                                .inc_du         = DST_INC_D1,
                                .inc_d2_du      = DST_INC_D2,
                                .trig           = DMA_TRIG_MEMORY,
                            };

    dma_trans_t trans =     {
                                .src            = &tgt_src,
                                .dst            = &tgt_dst,
                                .mode           = DMA_TRANS_MODE_SINGLE,
                                .dim            = DMA_DIM_CONF_2D,
                                .pad_top_du     = TOP_PAD,
                                .pad_bottom_du  = BOTTOM_PAD,
                                .pad_left_du    = LEFT_PAD,
                                .pad_right_du   = RIGHT_PAD,
                                .dim_inv        = TRANSPOSITION_EN,
                                .win_du         = 0,
                                .end            = DMA_TRANS_END_INTR
                            };
    
    dma_init(NULL);
    
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
        #if !EN_PERF
        /* Disable_interrupts */
        /* This does not prevent waking up the core as this is controlled by the MIP register */
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
            /* From here the core wakes up even if we did not jump to the ISR */
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
        #endif
    }

    #if EN_PERF    

    /* Read the cycles count after the DMA run */
    CSR_READ(CSR_REG_MCYCLE, &cycles_dma);

    /* Reset the performance counter to evaluate the CPU performance */
    CSR_SET_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    #endif

    #if EN_VERIF

    /* Run the same computation on the CPU */
    j_in_last = -1;
    for (int i=0; i < OUT_D2_PAD_STRIDE; i++)
    {
        stride_1d_cnt = 0;
        i_in = 0;
        top_pad_cnt = 0;

        if (j_in < TOP_PAD && j_in != j_in_last && stride_1d_cnt == 0 && stride_2d_cnt == 0)
        {
            left_pad_cnt++;
        }

        j_in_last = j_in;

        for (int j=0; j < OUT_D1_PAD_STRIDE; j++)
        {   
            dst_ptr = i * OUT_D1_PAD_STRIDE + j;
            src_ptr = (i_in - top_pad_cnt ) * STRIDE_IN_D2 * SIZE_IN_D1 + (j_in - left_pad_cnt) * STRIDE_IN_D1;
            if (i_in < LEFT_PAD || i_in >= SIZE_EXTR_D1 + LEFT_PAD || j_in < TOP_PAD || j_in >= SIZE_EXTR_D2 + TOP_PAD ||
                stride_1d_cnt != 0 || stride_2d_cnt != 0)
            {
                copied_data_2D_CPU[dst_ptr] = 0;
            }
            else
            {
                copied_data_2D_CPU[dst_ptr] = test_data[src_ptr];
            }
                        
            if (i_in < LEFT_PAD && stride_2d_cnt == 0)
            {
                top_pad_cnt++;
            }           
            
            if (stride_1d_cnt == STRIDE_OUT_D1 - 1)
            {
                stride_1d_cnt = 0;
                i_in++;
            }
            else
            {
                stride_1d_cnt++;
            }

        }

        if (stride_2d_cnt == STRIDE_OUT_D2 - 1)
        {
            stride_2d_cnt = 0;
            j_in++;
        }
        else
        {
            stride_2d_cnt++;
        }
        
        

    }
    #endif

    #if EN_PERF

    /* Read the cycles count after the CPU run */
    CSR_READ(CSR_REG_MCYCLE, &cycles_cpu);
    PRINTF("DMA cycles: %d\n\r", cycles_dma);
    PRINTF("CPU cycles: %d \n\r", cycles_cpu);
    PRINTF("\n\r");

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
        PRINTF("Success\n\r");
        return EXIT_SUCCESS;
    } 
    else 
    {
        PRINTF("Fail\n\r");
        return EXIT_FAILURE;
    }
    #endif

    #elif TEST_ID == 2

    /* Testing copy and padding of a 1xN matrix (an array) */

    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    dma_target_t tgt_src = {
                                .ptr            = &test_data[0],
                                .inc_du         = SRC_INC_D1,
                                .size_du        = SIZE_EXTR_D1,
                                .trig           = DMA_TRIG_MEMORY,
                                .type           = DMA_DATA_TYPE,
                            };

    dma_target_t tgt_dst = {
                                .ptr            = copied_data_1D_DMA,
                                .inc_du         = DST_INC_D1,
                                .trig           = DMA_TRIG_MEMORY,
                            };

    dma_trans_t trans =     {
                                .src            = &tgt_src,
                                .dst            = &tgt_dst,
                                .mode           = DMA_TRANS_MODE_SINGLE,
                                .dim            = DMA_DIM_CONF_1D,
                                .pad_left_du    = LEFT_PAD,
                                .pad_right_du   = RIGHT_PAD,
                                .win_du         = 0,
                                .end            = DMA_TRANS_END_INTR
                            };

    dma_init(NULL);
    
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
        #if !EN_PERF
        /* Disable_interrupts */
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
        #endif
    }

    #if EN_PERF

    /* Read the cycles count after the DMA run */
    CSR_READ(CSR_REG_MCYCLE, &cycles_dma);

    /* Reset the performance counter to evaluate the CPU performance */
    CSR_SET_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
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
    CSR_READ(CSR_REG_MCYCLE, &cycles_cpu);

    PRINTF("DMA cycles: %d\n\r", cycles_dma);
    PRINTF("CPU cycles: %d \n\r", cycles_cpu);
    PRINTF("\n\r");
    #endif

    #if EN_VERIF
    
    /* Verify that the DMA and the CPU outputs are the same */
    for (int i = 0; i < OUT_D1_PAD_STRIDE; i++) {
        if (copied_data_1D_DMA[i] != copied_data_1D_CPU[i]) {
            passed = 0;
        }
    }

    if (passed) {
        PRINTF("Success\n\r");
        return EXIT_SUCCESS;
    } 
    else 
    {
        PRINTF("Fail\n\r");
        return EXIT_FAILURE;
    }
    #endif

    #elif TEST_ID == 3

    /* Testing copy and padding of a NxM matrix using direct register operations.
     * This strategy allows for maximum performance but doesn't perform any checks on the data integrity.
     */
    
    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    /* The DMA is initialized (i.e. Any current transaction is cleaned.) */
    dma_init(NULL);

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
    peri->SRC_PTR = &test_data[0];
    peri->DST_PTR = copied_data_2D_DMA;

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
                    DMA_DATA_TYPE_REG_OFFSET,
                    DMA_DATA_TYPE_DATA_TYPE_MASK,
                    DMA_DATA_TYPE_DATA_TYPE_OFFSET,
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
    write_register( TOP_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_TOP_REG_OFFSET,
                    DMA_PAD_TOP_PAD_MASK,
                    DMA_PAD_TOP_PAD_OFFSET,
                    peri );

    write_register( RIGHT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_RIGHT_REG_OFFSET,
                    DMA_PAD_RIGHT_PAD_MASK,
                    DMA_PAD_RIGHT_PAD_OFFSET,
                    peri );

    write_register( LEFT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_LEFT_REG_OFFSET,
                    DMA_PAD_LEFT_PAD_MASK,
                    DMA_PAD_LEFT_PAD_OFFSET,
                    peri );

    write_register( BOTTOM_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_BOTTOM_REG_OFFSET,
                    DMA_PAD_BOTTOM_PAD_MASK,
                    DMA_PAD_BOTTOM_PAD_OFFSET,
                    peri );

    /* Set the sizes */

    write_register( SIZE_EXTR_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_SIZE_D2_REG_OFFSET,
                    DMA_SIZE_D2_SIZE_MASK,
                    DMA_SIZE_D2_SIZE_OFFSET,
                    peri );

    write_register( SIZE_EXTR_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_SIZE_D1_REG_OFFSET,
                    DMA_SIZE_D1_SIZE_MASK,
                    DMA_SIZE_D1_SIZE_OFFSET,
                    peri );

    while( ! dma_is_ready(0)) {
        #if !EN_PERF
        /* Disable_interrupts */
        /* This does not prevent waking up the core as this is controlled by the MIP register */
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
            /* From here the core wakes up even if we did not jump to the ISR */
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
        #endif
    }

    #if EN_PERF

    /* Read the cycles count after the DMA run */
    CSR_READ(CSR_REG_MCYCLE, &cycles_dma);

    /* Reset the performance counter to evaluate the CPU performance */
    CSR_SET_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
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
    CSR_READ(CSR_REG_MCYCLE, &cycles_cpu);

    PRINTF("DMA cycles: %d\n\r", cycles_dma);
    PRINTF("CPU cycles: %d \n\r", cycles_cpu);
    PRINTF("\n\r");
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
        PRINTF("Success\n\r");
        return EXIT_SUCCESS;
    } 
    else 
    {
        PRINTF("Fail\n\r");
        return EXIT_FAILURE;
    }
    #endif

    #endif
}