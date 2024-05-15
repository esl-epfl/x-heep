/*
 *  Copyright EPFL contributors.
 *  Licensed under the Apache License, Version 2.0, see LICENSE for details.
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 *  Info: Example application of matrix manipulation by exploiting the Multichannel Multidimensional Smart DMA (M2S DMA).
 *        In this code, there are some optional features:
 *        - Verification of matrix operations carried out by the M2S DMA
 *        - Performance comparison between the M2S DMA and the CPU, obtained by performing similar matrix operations
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
 *  
 *  0: Extract a NxM matrix, perform optional padding and copy the result to two separate
 *     AxB matrices using 2 channels at the same time
 *  1: 1D copy using 1 channel
 *  2: 2D copy using 2 channels and HALs
 * 
 *  DISCLAIMER: Before changing from test to test, change the number of channels in mcu_cfg.hjson
 *  and re-run "make mcu-gen" ... and "make verilator-sim".
 */

#define TEST_ID 0

/* Enable performance analysis */
#define EN_PERF 0

/* Enable verification */
#define EN_VERIF 1

/* Parameters */

/* Size of the extracted matrix (including strides on the input, excluding strides on the outputs) */
#define SIZE_EXTR_D1 10
#define SIZE_EXTR_D2 10

/* Set strides of the input ad output matrix */
#define STRIDE_IN_D1 1
#define STRIDE_IN_D2 2
#define STRIDE_OUT_D1 3
#define STRIDE_OUT_D2 2

/* Set the padding parameters */
#define TOP_PAD 2
#define BOTTOM_PAD 1
#define LEFT_PAD 3
#define RIGHT_PAD 2

/* Macros for dimensions computation */
#define OUT_D1_PAD ( SIZE_EXTR_D1 + LEFT_PAD + RIGHT_PAD )
#define OUT_D2_PAD ( SIZE_EXTR_D2  + TOP_PAD + BOTTOM_PAD )
#define OUT_D1_PAD_STRIDE ( (OUT_D1_PAD * STRIDE_OUT_D1) - (STRIDE_OUT_D1 - 1)  )
#define OUT_D2_PAD_STRIDE ( (OUT_D2_PAD * STRIDE_OUT_D2) - (STRIDE_OUT_D2 - 1)  )
#define OUT_DIM_1D ( OUT_D1_PAD_STRIDE  )
#define OUT_DIM_2D ( OUT_D1_PAD_STRIDE * OUT_D2_PAD_STRIDE )

/* Defines for M2S DMA */
#define M2S_DMA_CH0_IDX 0
#define M2S_DMA_CH1_IDX 1

/* Mask for direct register operations example */
#define DMA_CSR_REG_MIE_MASK (( 1 << 19 ) | (1 << 11 ))

/* Pointer increments computation */
#define SRC_INC_D1 STRIDE_IN_D1
#define DST_INC_D1 STRIDE_OUT_D1
#define SRC_INC_D2 (STRIDE_IN_D2 * SIZE_IN_D1 - (SIZE_EXTR_D1 - 1 + (STRIDE_IN_D1 - 1) * (SIZE_EXTR_D1 - 1)))
#define DST_INC_D2 ((STRIDE_OUT_D2 - 1) * OUT_DIM_1D + 1)

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   1

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

dma_input_data_type copied_data_2D_DMA_ch0[OUT_DIM_2D];
dma_input_data_type copied_data_2D_DMA_ch1[OUT_DIM_2D];
dma_input_data_type copied_data_2D_CPU_ch0[OUT_DIM_2D];
dma_input_data_type copied_data_2D_CPU_ch1[OUT_DIM_2D];

dma_config_flags_t res_valid, res_load, res_launch;

#if TEST_ID == 0
dma *peri_ch0 = dma_peri(0);
dma *peri_ch1 = dma_peri(1);
#elif TEST_ID == 1
dma *peri_ch0 = dma_peri(0);
#endif

m2s_dma *peri_m2s_dma = m2s_dma_peri;

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
uint16_t left_pad_cnt = 0;
uint16_t top_pad_cnt = 0;
uint8_t stride_1d_cnt = 0;
uint8_t stride_2d_cnt = 0;
char ch0_done = 0;
char ch1_done = 0;
uint8_t transaction_ifr[2];
uint8_t index_tr = 0;
char passed = 1;
char flag = 0;

void dma_intr_handler_trans_done()
{
    transaction_ifr[index_tr] = peri_m2s_dma->TRANSACTION_IFR;
    flag = 1;

    if (!(transaction_ifr[index_tr] & (0x1 << M2S_DMA_CH0_IDX)))
    {
        ch0_done = 1;
    }

    if (!(transaction_ifr[index_tr] & (0x1 << M2S_DMA_CH1_IDX)))
    {
        ch1_done = 1;
    }
    
   index_tr++;
}

/* Function used to simplify the register operations */
static inline volatile void write_register( uint32_t  p_val,
                                uint32_t  p_offset,
                                uint32_t  p_mask,
                                uint8_t   p_sel,
                                dma* peri_ch0 ) 
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
    uint32_t value  =  (( uint32_t * ) peri_ch0 ) [ index ];
    value           &= ~( p_mask << p_sel );
    value           |= (p_val & p_mask) << p_sel;
    (( uint32_t * ) peri_ch0 ) [ index ] = value;
};


int main()
{
    #if TEST_ID == 0

    /* 
     * Testing copy and padding of a NxM matrix using direct register operations.
     * This strategy allows for maximum performance but doesn't perform any checks on the data integrity.
     * The data is copied tusing both CH0 and CH1 to two different memory locations.
     */
    
    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    /* The DMA channels are initialized (i.e. Any current transaction is cleaned.) */
    dma_init(NULL);

    /* Enable the M2S DMA interrupt logic for both channels */
    write_register( ( 0x1 << M2S_DMA_CH0_IDX | 0x1 << M2S_DMA_CH1_IDX ),
                    M2S_DMA_TRANSACTION_IFR_REG_OFFSET,
                    M2S_DMA_TRANSACTION_IFR_EN_MASK,
                    M2S_DMA_TRANSACTION_IFR_EN_OFFSET,
                    m2s_dma_peri );

    /* Enable global interrupts */
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    /* Enable fast interrupts */
    CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK);

    /* Pointer set up */
    peri_ch0->SRC_PTR = &test_data[0];
    peri_ch0->DST_PTR = copied_data_2D_DMA_ch0;

    peri_ch1->SRC_PTR = &test_data[0];
    peri_ch1->DST_PTR = copied_data_2D_DMA_ch1;

    /* Dimensionality configuration */
    write_register( 0x1,
                    DMA_DIM_CONFIG_REG_OFFSET,
                    0x1,
                    DMA_DIM_CONFIG_DMA_DIM_BIT,
                    peri_ch0 );

    write_register( 0x1,
                    DMA_DIM_CONFIG_REG_OFFSET,
                    0x1,
                    DMA_DIM_CONFIG_DMA_DIM_BIT,
                    peri_ch1 );

    /* Operation mode configuration */
    write_register( DMA_TRANS_MODE_SINGLE,
                    DMA_MODE_REG_OFFSET,
                    DMA_MODE_MODE_MASK,
                    DMA_MODE_MODE_OFFSET,
                    peri_ch0 );

    write_register( DMA_TRANS_MODE_SINGLE,
                    DMA_MODE_REG_OFFSET,
                    DMA_MODE_MODE_MASK,
                    DMA_MODE_MODE_OFFSET,
                    peri_ch1 );
    
    /* Data type configuration */
    write_register( DMA_DATA_TYPE,
                    DMA_DATA_TYPE_REG_OFFSET,
                    DMA_DATA_TYPE_DATA_TYPE_MASK,
                    DMA_DATA_TYPE_DATA_TYPE_OFFSET,
                    peri_ch0 );

    write_register( DMA_DATA_TYPE,
                    DMA_DATA_TYPE_REG_OFFSET,
                    DMA_DATA_TYPE_DATA_TYPE_MASK,
                    DMA_DATA_TYPE_DATA_TYPE_OFFSET,
                    peri_ch1 );

    /* Set the source strides */
    write_register( SRC_INC_D1 * DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE),
                    DMA_SRC_PTR_INC_D1_REG_OFFSET,
                    DMA_SRC_PTR_INC_D1_INC_MASK,
                    DMA_SRC_PTR_INC_D1_INC_OFFSET,
                    peri_ch0 );
    
    write_register( SRC_INC_D2 * DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE),
                    DMA_SRC_PTR_INC_D2_REG_OFFSET,
                    DMA_SRC_PTR_INC_D2_INC_MASK,
                    DMA_SRC_PTR_INC_D2_INC_OFFSET,
                    peri_ch0 );
    
    write_register( DST_INC_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_DST_PTR_INC_D1_REG_OFFSET,
                    DMA_DST_PTR_INC_D1_INC_MASK,
                    DMA_DST_PTR_INC_D1_INC_OFFSET,
                    peri_ch0 );
    
    write_register( DST_INC_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_DST_PTR_INC_D2_REG_OFFSET,
                    DMA_DST_PTR_INC_D2_INC_MASK,
                    DMA_DST_PTR_INC_D2_INC_OFFSET,
                    peri_ch0 );

    write_register( SRC_INC_D1 * DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE),
                    DMA_SRC_PTR_INC_D1_REG_OFFSET,
                    DMA_SRC_PTR_INC_D1_INC_MASK,
                    DMA_SRC_PTR_INC_D1_INC_OFFSET,
                    peri_ch1 );
    
    write_register( SRC_INC_D2 * DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE),
                    DMA_SRC_PTR_INC_D2_REG_OFFSET,
                    DMA_SRC_PTR_INC_D2_INC_MASK,
                    DMA_SRC_PTR_INC_D2_INC_OFFSET,
                    peri_ch1 );
    
    write_register( DST_INC_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_DST_PTR_INC_D1_REG_OFFSET,
                    DMA_DST_PTR_INC_D1_INC_MASK,
                    DMA_DST_PTR_INC_D1_INC_OFFSET,
                    peri_ch1 );
    
    write_register( DST_INC_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_DST_PTR_INC_D2_REG_OFFSET,
                    DMA_DST_PTR_INC_D2_INC_MASK,
                    DMA_DST_PTR_INC_D2_INC_OFFSET,
                    peri_ch1 );

    /* Padding configuration */
    write_register( TOP_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_TOP_REG_OFFSET,
                    DMA_PAD_TOP_PAD_MASK,
                    DMA_PAD_TOP_PAD_OFFSET,
                    peri_ch0 );

    write_register( RIGHT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_RIGHT_REG_OFFSET,
                    DMA_PAD_RIGHT_PAD_MASK,
                    DMA_PAD_RIGHT_PAD_OFFSET,
                    peri_ch0 );

    write_register( LEFT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_LEFT_REG_OFFSET,
                    DMA_PAD_LEFT_PAD_MASK,
                    DMA_PAD_LEFT_PAD_OFFSET,
                    peri_ch0 );

    write_register( BOTTOM_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_BOTTOM_REG_OFFSET,
                    DMA_PAD_BOTTOM_PAD_MASK,
                    DMA_PAD_BOTTOM_PAD_OFFSET,
                    peri_ch0 );

    write_register( TOP_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_TOP_REG_OFFSET,
                    DMA_PAD_TOP_PAD_MASK,
                    DMA_PAD_TOP_PAD_OFFSET,
                    peri_ch1 );

    write_register( RIGHT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_RIGHT_REG_OFFSET,
                    DMA_PAD_RIGHT_PAD_MASK,
                    DMA_PAD_RIGHT_PAD_OFFSET,
                    peri_ch1 );

    write_register( LEFT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_LEFT_REG_OFFSET,
                    DMA_PAD_LEFT_PAD_MASK,
                    DMA_PAD_LEFT_PAD_OFFSET,
                    peri_ch1 );

    write_register( BOTTOM_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_BOTTOM_REG_OFFSET,
                    DMA_PAD_BOTTOM_PAD_MASK,
                    DMA_PAD_BOTTOM_PAD_OFFSET,
                    peri_ch1 );

    /* Set the sizes */

    write_register( SIZE_EXTR_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_SIZE_D2_REG_OFFSET,
                    DMA_SIZE_D2_SIZE_MASK,
                    DMA_SIZE_D2_SIZE_OFFSET,
                    peri_ch0 );

    write_register( SIZE_EXTR_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_SIZE_D1_REG_OFFSET,
                    DMA_SIZE_D1_SIZE_MASK,
                    DMA_SIZE_D1_SIZE_OFFSET,
                    peri_ch0 );

    write_register( SIZE_EXTR_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_SIZE_D2_REG_OFFSET,
                    DMA_SIZE_D2_SIZE_MASK,
                    DMA_SIZE_D2_SIZE_OFFSET,
                    peri_ch1 );

    write_register( SIZE_EXTR_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_SIZE_D1_REG_OFFSET,
                    DMA_SIZE_D1_SIZE_MASK,
                    DMA_SIZE_D1_SIZE_OFFSET,
                    peri_ch1 );

    wait_for_interrupt();
    

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
                copied_data_2D_CPU_ch0[dst_ptr] = 0;
                copied_data_2D_CPU_ch1[dst_ptr] = 0;
            }
            else
            {
                copied_data_2D_CPU_ch0[dst_ptr] = test_data[src_ptr];
                copied_data_2D_CPU_ch1[dst_ptr] = test_data[src_ptr];
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
    
    PRINTF("%d %d %d %d %d %d\n\r", ch0_done, ch1_done, flag, transaction_ifr[0], transaction_ifr[1], index_tr);

    /* Verify that the DMA and the CPU outputs are the same */
    for (int i = 0; i < OUT_D2_PAD_STRIDE; i++) {
        for (int j = 0; j < OUT_D1_PAD_STRIDE; j++) {
            if ((copied_data_2D_DMA_ch0[i * OUT_D1_PAD_STRIDE + j] != copied_data_2D_CPU_ch0[i * OUT_D1_PAD_STRIDE + j]) &
                (copied_data_2D_DMA_ch1[i * OUT_D1_PAD_STRIDE + j] != copied_data_2D_CPU_ch1[i * OUT_D1_PAD_STRIDE + j])) 
            {
                passed = 0;
            }
        }
    }

    if (passed) {
        PRINTF("Success\n\r");
    } 
    else 
    {
        PRINTF("Fail\n\r");
        return EXIT_FAILURE;
    }

    #endif 

    #elif TEST_ID == 1

    /* Testing copy and padding of a NxM matrix using Low Level control, i.e. register writes.
     * This strategy allows for maximum performance but doesn't perform any checks on the data integrity.
     * The data is copied using both ch0 and ch1 to two different memory locations, obtaining two identical copies.
     */
    
    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    /* The M2S DMA is initialized (i.e. Any current transaction is cleaned.) */
    dma_init(NULL);

    /* Enable the M2S DMA interrupt logic for channel 0 */
    write_register( ( 0x1 << M2S_DMA_CH0_IDX),
                    M2S_DMA_TRANSACTION_IFR_REG_OFFSET,
                    M2S_DMA_TRANSACTION_IFR_EN_MASK,
                    M2S_DMA_TRANSACTION_IFR_EN_OFFSET,
                    m2s_dma_peri );

    /* Enable global interrupts */
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    /* Enable fast interrupts */
    CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK);

    /* Pointer set up */
    peri_ch0->SRC_PTR = &test_data[0];
    peri_ch0->DST_PTR = copied_data_2D_DMA_ch0;

    /* Dimensionality configuration */
    write_register( 0x1,
                    DMA_DIM_CONFIG_REG_OFFSET,
                    0x1,
                    DMA_DIM_CONFIG_DMA_DIM_BIT,
                    peri_ch0 );

    /* Operation mode configuration */
    write_register( DMA_TRANS_MODE_SINGLE,
                    DMA_MODE_REG_OFFSET,
                    DMA_MODE_MODE_MASK,
                    DMA_MODE_MODE_OFFSET,
                    peri_ch0 );
    
    /* Data type configuration */
    write_register( DMA_DATA_TYPE,
                    DMA_DATA_TYPE_REG_OFFSET,
                    DMA_DATA_TYPE_DATA_TYPE_MASK,
                    DMA_DATA_TYPE_DATA_TYPE_OFFSET,
                    peri_ch0 );

    /* Set the source strides */
    write_register( SRC_INC_D1 * DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE),
                    DMA_SRC_PTR_INC_D1_REG_OFFSET,
                    DMA_SRC_PTR_INC_D1_INC_MASK,
                    DMA_SRC_PTR_INC_D1_INC_OFFSET,
                    peri_ch0 );
    
    write_register( SRC_INC_D2 * DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE),
                    DMA_SRC_PTR_INC_D2_REG_OFFSET,
                    DMA_SRC_PTR_INC_D2_INC_MASK,
                    DMA_SRC_PTR_INC_D2_INC_OFFSET,
                    peri_ch0 );
    
    write_register( DST_INC_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_DST_PTR_INC_D1_REG_OFFSET,
                    DMA_DST_PTR_INC_D1_INC_MASK,
                    DMA_DST_PTR_INC_D1_INC_OFFSET,
                    peri_ch0 );
    
    write_register( DST_INC_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_DST_PTR_INC_D2_REG_OFFSET,
                    DMA_DST_PTR_INC_D2_INC_MASK,
                    DMA_DST_PTR_INC_D2_INC_OFFSET,
                    peri_ch0 );

    /* Padding configuration */
    write_register( TOP_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_TOP_REG_OFFSET,
                    DMA_PAD_TOP_PAD_MASK,
                    DMA_PAD_TOP_PAD_OFFSET,
                    peri_ch0 );

    write_register( RIGHT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_RIGHT_REG_OFFSET,
                    DMA_PAD_RIGHT_PAD_MASK,
                    DMA_PAD_RIGHT_PAD_OFFSET,
                    peri_ch0 );

    write_register( LEFT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_LEFT_REG_OFFSET,
                    DMA_PAD_LEFT_PAD_MASK,
                    DMA_PAD_LEFT_PAD_OFFSET,
                    peri_ch0 );

    write_register( BOTTOM_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_PAD_BOTTOM_REG_OFFSET,
                    DMA_PAD_BOTTOM_PAD_MASK,
                    DMA_PAD_BOTTOM_PAD_OFFSET,
                    peri_ch0 );

    /* Set the sizes */

    write_register( SIZE_OUT_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_SIZE_D2_REG_OFFSET,
                    DMA_SIZE_D2_SIZE_MASK,
                    DMA_SIZE_D2_SIZE_OFFSET,
                    peri_ch0 );

    write_register( SIZE_OUT_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE),
                    DMA_SIZE_D1_REG_OFFSET,
                    DMA_SIZE_D1_SIZE_MASK,
                    DMA_SIZE_D1_SIZE_OFFSET,
                    peri_ch0 );

    wait_for_interrupt();
    /*
    while( ! dma_is_ready()) {
        #if !EN_PERF
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready() == 0 ) {
            wait_for_interrupt();
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
        #endif
    }*/

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
    for (int i=0; i < OUT_D2; i++)
    {
        for (int j=0; j < OUT_D1; j++)
        {
            read_ptr = i * STRIDE_OUT_D2 * OUT_D1 + j * STRIDE_OUT_D1;
            write_ptr = (i - top_pad_cnt ) * STRIDE_IN_D2 * SIZE_IN_D1 + (j - left_pad_cnt) * STRIDE_IN_D1;
            if (i < TOP_PAD || i >= SIZE_OUT_D2 + BOTTOM_PAD || j < LEFT_PAD || j >= SIZE_OUT_D1 + RIGHT_PAD)
            {
                copied_data_2D_CPU_ch0[read_ptr] = 0;
            }
            else
            {
                copied_data_2D_CPU_ch0[read_ptr] = test_data[write_ptr];
            }
            if (j < LEFT_PAD && i >= TOP_PAD)
            {
                left_pad_cnt++;
            }
        }
        if (i < TOP_PAD)
        {
            top_pad_cnt++;
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
    
    PRINTF("%d %d %d\n\r", ch0_done, ch1_done, flag);

    /* Verify that the DMA and the CPU outputs are the same */
    for (int j = 0; j < OUT_D1; j++) {
        if (copied_data_2D_DMA_ch0[j] != copied_data_2D_CPU_ch0[j]) {
            passed = 0;
        }
    }

    if (passed) {
        PRINTF("Success\n\r");
    } 
    else 
    {
        PRINTF("Fail\n\r");
        return EXIT_FAILURE;
    }

    #endif 
    #elif TEST_ID == 2

    /* Testing copy and padding of a NxM matrix to two different locations using 2 channels and HALs */
    
    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    dma_target_t tgt_src = {
                                .ptr            = &test_data[0],
                                .inc_du         = SRC_INC_D1,
                                .inc_d2_du      = SRC_INC_D2,
                                .size_du        = SIZE_OUT_D1,
                                .size_d2_du     = SIZE_OUT_D2,
                                .trig           = DMA_TRIG_MEMORY,
                                .type           = DMA_DATA_TYPE
                            };

    dma_target_t tgt_dst_ch0 = {
                                .ptr            = copied_data_2D_DMA_ch0,
                                .inc_du         = DST_INC_D1,
                                .inc_d2_du      = DST_INC_D2,
                                .trig           = DMA_TRIG_MEMORY
                            };
    
    dma_target_t tgt_dst_ch1 = {
                                .ptr            = copied_data_2D_DMA_ch0,
                                .inc_du         = DST_INC_D1,
                                .inc_d2_du      = DST_INC_D2,
                                .trig           = DMA_TRIG_MEMORY
                            };

    dma_trans_t trans_ch0 =     {
                                .src            = &tgt_src,
                                .dst            = &tgt_dst_ch0,
                                .mode           = DMA_TRANS_MODE_SINGLE,
                                .dim            = DMA_DIM_CONF_2D,
                                .pad_top_du     = TOP_PAD,
                                .pad_bottom_du  = BOTTOM_PAD,
                                .pad_left_du    = LEFT_PAD,
                                .pad_right_du   = RIGHT_PAD,
                                .win_du         = 0,
                                .end            = DMA_TRANS_END_INTR,
                                .channel        = 0
                            };

    dma_trans_t trans_ch1 =     {
                                .src            = &tgt_src,
                                .dst            = &tgt_dst_ch1,
                                .mode           = DMA_TRANS_MODE_SINGLE,
                                .dim            = DMA_DIM_CONF_2D,
                                .pad_top_du     = TOP_PAD,
                                .pad_bottom_du  = BOTTOM_PAD,
                                .pad_left_du    = LEFT_PAD,
                                .pad_right_du   = RIGHT_PAD,
                                .win_du         = 0,
                                .end            = DMA_TRANS_END_INTR,
                                .channel        = 1
                            };
    
    dma_init(NULL);
    
    #if EN_PERF

    res_valid = dma_validate_transaction(&trans_ch0, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    res_valid = dma_validate_transaction(&trans_ch1, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    res_load = dma_load_transaction(&trans_ch0);
    res_load = dma_load_transaction(&trans_ch1);
    res_launch = dma_launch(&trans_ch0);
    res_launch = dma_launch(&trans_ch1);
    
    #else

    res_valid = dma_validate_transaction(&trans_ch0, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res_valid, res_valid == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_valid = dma_validate_transaction(&trans_ch1, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res_valid, res_valid == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_load = dma_load_transaction(&trans_ch0);
    PRINTF("load: %u \t%s\n\r", res_load, res_load == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_load = dma_load_transaction(&trans_ch1);
    PRINTF("load: %u \t%s\n\r", res_load, res_load == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_launch = dma_launch(&trans_ch0);
    PRINTF("laun: %u \t%s\n\r", res_launch, res_launch == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_launch = dma_launch(&trans_ch1);
    PRINTF("laun: %u \t%s\n\r", res_launch, res_launch == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    #endif

    while( ! dma_is_ready(0)) {
        #if !EN_PERF
        /* Disable_interrupts */
        /* This does not prevent waking up the core as this is controlled by the MIP register */
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready() == 0 ) {
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
    for (int i=0; i < OUT_D2; i++)
    {
        for (int j=0; j < OUT_D1; j++)
        {
            read_ptr = i * STRIDE_OUT_D2 * OUT_D1 + j * STRIDE_OUT_D1;
            write_ptr = (i - top_pad_cnt ) * STRIDE_IN_D2 * SIZE_IN_D1 + (j - left_pad_cnt) * STRIDE_IN_D1;
            if (i < TOP_PAD || i >= SIZE_OUT_D2 + TOP_PAD || j < LEFT_PAD || j >= SIZE_OUT_D1 + LEFT_PAD)
            {
                copied_data_2D_CPU_ch0[read_ptr] = 0;
            }
            else
            {
                copied_data_2D_CPU_ch0[read_ptr] = test_data[write_ptr];
            }
            if (j < LEFT_PAD && i >= TOP_PAD)
            {
                left_pad_cnt++;
            }
        }
        if (i < TOP_PAD)
        {
            top_pad_cnt++;
        }
        left_pad_cnt = 0;
    }
    #endif

    #if EN_PERF

    /* Read the cycles count after the CPU run */
    CSR_READ(CSR_REG_MCYCLE, &cycles_cpu);
    /*PRINTF("DMA cycles: %d\n\r", cycles_dma);
    PRINTF("CPU cycles: %d \n\r", cycles_cpu);
    PRINTF("\n\r");*/
    PRINTF("%d\n\r", cycles_dma);
    PRINTF("%d \n\r", cycles_cpu);
    PRINTF("\n\r");

    #endif

    #if EN_VERIF
/*
    for (int i = 0; i < OUT_D2; i++) {
        for (int j = 0; j < OUT_D1; j++) {
            PRINTF("%d ", copied_data_2D_DMA[i * OUT_D1 + j]);
        }
        PRINTF("\n\r");
    }

    for (int i = 0; i < OUT_D2; i++) {
        for (int j = 0; j < OUT_D1; j++) {
            PRINTF("%d ", copied_data_2D_CPU_ch0[i * OUT_D1 + j]);
        }
        PRINTF("\n\r");
    }*/
    
    /* Verify that the DMA and the CPU outputs are the same */
    for (int i = 0; i < OUT_D2; i++) {
        for (int j = 0; j < OUT_D1; j++) {
            if (copied_data_2D_DMA[i * OUT_D1 + j] != copied_data_2D_CPU_ch0[i * OUT_D1 + j]) {
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