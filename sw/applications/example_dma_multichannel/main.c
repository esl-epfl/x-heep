/*
 *  Copyright EPFL contributors.
 *  Licensed under the Apache License, Version 2.0, see LICENSE for details.
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 *  Info: Example application of matrix manipulation by exploiting the multichannel feature of the DMA subsystem.
 *        This code is capable of testing the following features:
 *        - Verification of matrix operations carried out by the DMA subsystem
 *        - Performance comparison between the DMA multichannel and the CPU, obtained by performing similar matrix operations
 *          and monitoring the performance counter. 
 *          The performance of the DMA is compared against sequential CPU loops for each operation performed by a single channel.
 *          A typical case in which the DMA could be used to improve the performance is a series of matrix operations, 
 *          like extracting 3 matrices from a larger one.
 *          By exploiting the DMA, these three separate calls can be performed in parallel.
 */

#include <stdio.h>
#include <stdlib.h>
#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "rv_plic.h"
#include "test_data.h"
#include "test_data_flash.h"
#include "w25q128jw.h"

/*  
 *  DISCLAIMER: In order to perform every test, the DMA has by default 4 channels. If the number of channels in mcu_cfg.hjson
 *  has been reduced, some tests might not be performed. 
 *  When using the default memory configuration (64kB), pay attention to the dimensions of the output matrices.
 * 
 *  Enable one or more of the following tests by defining the correct TEST_ID_*:
 *  
 *  0: Extract a NxM matrix, perform optional padding and copy the result to two separate
 *     AxB matrices using 2 channels at the same time and using direct register writes.
 * 
 *  1: 1D copy using 1 channel using direct register writes.
 * 
 *  2: Extract a NxM matrix, perform optional padding and copy the result to two separate
 *     AxB matrices using 2 channels at the same time 2D copy using 2 channels using HALs.
 * 
 *  3: Complex example by using 4 channels and HALS. The distribution of the workload is the following:
 *      - CH0: Extract a NxM matrix, perform optional padding and copy the result
 *      - CH1: Extract a NxM matrix, perform optional padding and copy the result
 *      - CH2: Extract a NxM matrix, perform optional padding, transpose it and copy the result
 *      - CH3: Extract a NxM matrix, perform optional padding, transpose it and copy the result
 * 
 *  4: Extract a NxM matrix, perform optional padding and copy the result to a location using one channel (with HALs), 
 *     while at the same time read a buffer from SPI and copy it to another location using another channel (with HALs).
 *     This test can only be performed on FPGA boards or using QuestaSim, by setting the correct macro.
 * 
 */

#define TEST_ID_0
#define TEST_ID_1
#define TEST_ID_2
#define TEST_ID_3
#define TEST_ID_4

/* Enable performance analysis */
#define EN_PERF 1

/* Enable verification */
#define EN_VERIF 1

/* Parameters */

/* Size of the extracted matrix (including strides on the input, excluding strides on the outputs) */
#define SIZE_EXTR_D1 4
#define SIZE_EXTR_D2 4

/* Set strides of the input ad output matrix */
#define STRIDE_IN_D1 1
#define STRIDE_IN_D2 1
#define STRIDE_OUT_D1 1
#define STRIDE_OUT_D2 1

/* Set the padding parameters */
#define TOP_PAD 0
#define BOTTOM_PAD 0
#define LEFT_PAD 0
#define RIGHT_PAD 0

/* Macros for dimensions computation */
#define OUT_D1_PAD ( SIZE_EXTR_D1 + LEFT_PAD + RIGHT_PAD )
#define OUT_D2_PAD ( SIZE_EXTR_D2  + TOP_PAD + BOTTOM_PAD )
#define OUT_D1_PAD_STRIDE ( (OUT_D1_PAD * STRIDE_OUT_D1) - (STRIDE_OUT_D1 - 1)  )
#define OUT_D2_PAD_STRIDE ( (OUT_D2_PAD * STRIDE_OUT_D2) - (STRIDE_OUT_D2 - 1)  )
#define OUT_DIM_1D ( OUT_D1_PAD_STRIDE  )
#define OUT_DIM_2D ( OUT_D1_PAD_STRIDE * OUT_D2_PAD_STRIDE )

/* Defines for DMA channels */
#define DMA_CH0_IDX 0
#define DMA_CH1_IDX 1
#define DMA_CH2_IDX 2
#define DMA_CH3_IDX 3

/* Assigning a pointer to a define writes the pointed array in the flash */
#define TEST_DATA_FLASH_PTR test_data_flash

/* Mask for direct register operations example */
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

/* FPGA SPI board selection */
#if !TARGET_SIM
    #define USE_SPI_FLASH
#endif

/* QuestaSim macro to enable test 4, by default is disabled */
//#define SIM_QUESTASIM

/* Memory allocation for examples */
uint32_t copied_test_data_flash[TEST_DATA_FLASH_SIZE];
dma_input_data_type copied_data_1D_DMA[OUT_DIM_1D];
dma_input_data_type copied_data_1D_CPU[OUT_DIM_1D];
dma_input_data_type copied_data_2D_DMA_ch0[OUT_DIM_2D];
dma_input_data_type copied_data_2D_DMA_ch1[OUT_DIM_2D];
dma_input_data_type copied_data_2D_DMA_ch2[OUT_DIM_2D];
dma_input_data_type copied_data_2D_DMA_ch3[OUT_DIM_2D];
dma_input_data_type copied_data_2D_CPU_ch0[OUT_DIM_2D];
dma_input_data_type copied_data_2D_CPU_ch1[OUT_DIM_2D];
dma_input_data_type copied_data_2D_CPU_ch2[OUT_DIM_2D];
dma_input_data_type copied_data_2D_CPU_ch3[OUT_DIM_2D];

/* DMA source, destination and transaction */
dma_target_t tgt_src;
dma_target_t tgt_dst_ch0;
dma_target_t tgt_dst_ch1;
dma_target_t tgt_dst_ch2;
dma_target_t tgt_dst_ch3;
dma_trans_t trans_ch0;
dma_trans_t trans_ch1;
dma_trans_t trans_ch2;
dma_trans_t trans_ch3;
dma_target_t tgt_src_trsp;

dma_config_flags_t res_valid, res_load, res_launch;

#if defined(TEST_ID_0) || defined(TEST_ID_1)
dma *peri_ch0 = dma_peri(0);
dma *peri_ch1 = dma_peri(1);
#endif

/* CPU computation variables */
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
char flag = 0;

/* Function used to simplify the register operations */
static inline volatile void write_register( uint32_t  p_val,
                                uint32_t  p_offset,
                                uint32_t  p_mask,
                                uint8_t   p_sel,
                                dma* peri_chx ) 
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
    uint32_t value  =  (( uint32_t * ) peri_chx ) [ index ];
    value           &= ~( p_mask << p_sel );
    value           |= (p_val & p_mask) << p_sel;
    (( uint32_t * ) peri_chx ) [ index ] = value;
};

int main()
{

    #ifdef TEST_ID_0

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

    /* Enable the DMA interrupt logic for both channels */
    write_register(  
                    0x1,
                    DMA_INTERRUPT_EN_REG_OFFSET,
                    0xffff,
                    DMA_INTERRUPT_EN_TRANSACTION_DONE_BIT,
                    peri_ch0
                    );

    write_register(  
                    0x1,
                    DMA_INTERRUPT_EN_REG_OFFSET,
                    0xffff,
                    DMA_INTERRUPT_EN_TRANSACTION_DONE_BIT,
                    peri_ch1
                    );

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

    /* Wait for CH1 to end */
    while(!dma_is_ready(1)) {
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
    #endif

    #if EN_VERIF

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
        PRINTF("Success test 0\n\n\r");
    } 
    else 
    {
        PRINTF("Fail test 0\n\r");
        return EXIT_FAILURE;
    }

    #endif 

    #endif
    
    #ifdef TEST_ID_1

    /* 
     * Testing copy and padding of a NxM matrix using Low Level control direct register writes.
     * This strategy allows for maximum performance but doesn't perform any checks on the data integrity.
     * The data is copied using CH0.
     */

    /* Reset for second test */
    passed = 1;
    i_in = 0;
    j_in = 0;
    left_pad_cnt = 0;
    top_pad_cnt = 0;
    stride_1d_cnt = 0;
    stride_2d_cnt = 0;

    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    /* The DMA is initialized (i.e. Any current transaction is cleaned.) */
    dma_init(NULL);

    /* Enable the DMA interrupt logic for channel 0 */
    write_register( ( 0x1 << DMA_CH0_IDX),
                    DMA_TRANSACTION_IFR_REG_OFFSET,
                    0x1,
                    DMA_TRANSACTION_IFR_FLAG_BIT,
                    peri_ch0 );

    /* Enable global interrupts */
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    /* Enable fast interrupts */
    CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK);

    /* Pointer set up */
    peri_ch0->SRC_PTR = &test_data[0];
    peri_ch0->DST_PTR = copied_data_1D_DMA;

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

    /* Wait for CH0 to end */
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
    
    while( ! dma_is_ready(0)) {
        #if !EN_PERF
        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready() == 0 ) {
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
    #endif

    #if EN_VERIF

    /* Verify that the DMA and the CPU outputs are the same */
    for (int j = 0; j < OUT_D1_PAD_STRIDE; j++) {
        if (copied_data_1D_DMA[j] != copied_data_1D_CPU[j]) {
            passed = 0;
        }
    }

    if (passed) {
        PRINTF("Success test 1\n\n\r");
    } 
    else 
    {
        PRINTF("Fail test 1\n\r");
        return EXIT_FAILURE;
    }

    #endif 

    #endif 

    #ifdef TEST_ID_2

    /* Testing copy and padding of a NxM matrix to two different locations using 2 channels and HALs */
    
    /* Reset for third test */
    passed = 1;
    i_in = 0;
    j_in = 0;
    left_pad_cnt = 0;
    top_pad_cnt = 0;
    stride_1d_cnt = 0;
    stride_2d_cnt = 0;
    for (int i = 0; i < OUT_DIM_2D; i++) {
        copied_data_2D_DMA_ch0[i] = 0;
        copied_data_2D_CPU_ch0[i] = 0;
        copied_data_2D_DMA_ch1[i] = 0;
        copied_data_2D_CPU_ch1[i] = 0;
    }

    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    tgt_src.ptr            = &test_data[0];
    tgt_src.inc_du         = SRC_INC_D1;
    tgt_src.inc_d2_du      = SRC_INC_D2;
    tgt_src.size_du        = SIZE_EXTR_D1;
    tgt_src.size_d2_du     = SIZE_EXTR_D2;
    tgt_src.trig           = DMA_TRIG_MEMORY;
    tgt_src.type           = DMA_DATA_TYPE;

    tgt_dst_ch0.ptr            = copied_data_2D_DMA_ch0;
    tgt_dst_ch0.inc_du         = DST_INC_D1;
    tgt_dst_ch0.inc_d2_du      = DST_INC_D2;
    tgt_dst_ch0.trig           = DMA_TRIG_MEMORY;
    
    tgt_dst_ch1.ptr            = copied_data_2D_DMA_ch1;
    tgt_dst_ch1.inc_du         = DST_INC_D1;
    tgt_dst_ch1.inc_d2_du      = DST_INC_D2;
    tgt_dst_ch1.trig           = DMA_TRIG_MEMORY;

    trans_ch0.src            = &tgt_src;
    trans_ch0.dst            = &tgt_dst_ch0;
    trans_ch0.mode           = DMA_TRANS_MODE_SINGLE;
    trans_ch0.dim            = DMA_DIM_CONF_2D;
    trans_ch0.pad_top_du     = TOP_PAD;
    trans_ch0.pad_bottom_du  = BOTTOM_PAD;
    trans_ch0.pad_left_du    = LEFT_PAD;
    trans_ch0.pad_right_du   = RIGHT_PAD;
    trans_ch0.win_du         = 0;
    trans_ch0.end            = DMA_TRANS_END_INTR;
    trans_ch0.channel        = 0;

    trans_ch1.src            = &tgt_src;
    trans_ch1.dst            = &tgt_dst_ch1;
    trans_ch1.mode           = DMA_TRANS_MODE_SINGLE;
    trans_ch1.dim            = DMA_DIM_CONF_2D;
    trans_ch1.pad_top_du     = TOP_PAD;
    trans_ch1.pad_bottom_du  = BOTTOM_PAD;
    trans_ch1.pad_left_du    = LEFT_PAD;
    trans_ch1.pad_right_du   = RIGHT_PAD;
    trans_ch1.win_du         = 0;
    trans_ch1.end            = DMA_TRANS_END_INTR;
    trans_ch1.channel        = 1;
    
    dma_init(NULL);
    
    #if EN_PERF

    dma_validate_transaction(&trans_ch0, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    dma_validate_transaction(&trans_ch1, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    dma_load_transaction(&trans_ch0);
    dma_load_transaction(&trans_ch1);
    dma_launch(&trans_ch0);
    dma_launch(&trans_ch1);
    
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

    /* Wait for CH1 to end */
    while(!dma_is_ready(1)) {
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

    #endif

    #if EN_VERIF

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
        PRINTF("Success test 2\n\n\r");
    } 
    else 
    {
        PRINTF("Fail test 2\n\r");
        return EXIT_FAILURE;
    }
    #endif

    #endif
    
    #ifdef TEST_ID_3
    
    /* Complex 4 channel DMA test */

    /* Reset for fourth test */
    passed = 1;
    i_in = 0;
    j_in = 0;
    left_pad_cnt = 0;
    top_pad_cnt = 0;
    stride_1d_cnt = 0;
    stride_2d_cnt = 0;
    for (int i = 0; i < OUT_DIM_2D; i++) {
        copied_data_2D_DMA_ch0[i] = 0;
        copied_data_2D_CPU_ch0[i] = 0;
        copied_data_2D_DMA_ch1[i] = 0;
        copied_data_2D_CPU_ch1[i] = 0;
    }
    
    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    tgt_src.ptr            = &test_data[0];
    tgt_src.inc_du         = SRC_INC_D1;
    tgt_src.inc_d2_du      = SRC_INC_D2;
    tgt_src.size_du        = SIZE_EXTR_D1;
    tgt_src.size_d2_du     = SIZE_EXTR_D2;
    tgt_src.trig           = DMA_TRIG_MEMORY;
    tgt_src.type           = DMA_DATA_TYPE;

    tgt_src_trsp.ptr            = &test_data[0];
    tgt_src_trsp.inc_du         = SRC_INC_TRSP_D1;
    tgt_src_trsp.inc_d2_du      = SRC_INC_TRSP_D2;
    tgt_src_trsp.size_du        = SIZE_EXTR_D1;
    tgt_src_trsp.size_d2_du     = SIZE_EXTR_D2;
    tgt_src_trsp.trig           = DMA_TRIG_MEMORY;
    tgt_src_trsp.type           = DMA_DATA_TYPE;

    tgt_dst_ch0.ptr            = copied_data_2D_DMA_ch0;
    tgt_dst_ch0.inc_du         = DST_INC_D1;
    tgt_dst_ch0.inc_d2_du      = DST_INC_D2;
    tgt_dst_ch0.trig           = DMA_TRIG_MEMORY;
    
    tgt_dst_ch1.ptr            = copied_data_2D_DMA_ch1;
    tgt_dst_ch1.inc_du         = DST_INC_D1;
    tgt_dst_ch1.inc_d2_du      = DST_INC_D2;
    tgt_dst_ch1.trig           = DMA_TRIG_MEMORY;

    tgt_dst_ch2.ptr            = copied_data_2D_DMA_ch2;
    tgt_dst_ch2.inc_du         = DST_INC_D1;
    tgt_dst_ch2.inc_d2_du      = DST_INC_D2;
    tgt_dst_ch2.trig           = DMA_TRIG_MEMORY;

    tgt_dst_ch3.ptr            = copied_data_2D_DMA_ch3;
    tgt_dst_ch3.inc_du         = DST_INC_D1;
    tgt_dst_ch3.inc_d2_du      = DST_INC_D2;
    tgt_dst_ch3.trig           = DMA_TRIG_MEMORY;

    trans_ch0.src            = &tgt_src;
    trans_ch0.dst            = &tgt_dst_ch0;
    trans_ch0.mode           = DMA_TRANS_MODE_SINGLE;
    trans_ch0.dim            = DMA_DIM_CONF_2D;
    trans_ch0.win_du         = 0;
    trans_ch0.end            = DMA_TRANS_END_INTR;
    trans_ch0.channel        = 0;

    trans_ch1.src            = &tgt_src;
    trans_ch1.dst            = &tgt_dst_ch1;
    trans_ch1.mode           = DMA_TRANS_MODE_SINGLE;
    trans_ch1.dim            = DMA_DIM_CONF_2D;
    trans_ch1.pad_top_du     = TOP_PAD;
    trans_ch1.pad_bottom_du  = BOTTOM_PAD;
    trans_ch1.pad_left_du    = LEFT_PAD;
    trans_ch1.pad_right_du   = RIGHT_PAD;
    trans_ch1.win_du         = 0;
    trans_ch1.end            = DMA_TRANS_END_INTR;
    trans_ch1.channel        = DMA_CH1_IDX;

    trans_ch2.src            = &tgt_src_trsp;
    trans_ch2.dst            = &tgt_dst_ch1;
    trans_ch2.mode           = DMA_TRANS_MODE_SINGLE;
    trans_ch2.dim            = DMA_DIM_CONF_2D;
    trans_ch2.win_du         = 0;
    trans_ch2.end            = DMA_TRANS_END_INTR;
    trans_ch2.dim_inv        = TRANSPOSITION_EN;
    trans_ch2.channel        = DMA_CH2_IDX;

    trans_ch3.src            = &tgt_src_trsp;
    trans_ch3.dst            = &tgt_dst_ch1;
    trans_ch3.mode           = DMA_TRANS_MODE_SINGLE;
    trans_ch3.dim            = DMA_DIM_CONF_2D;
    trans_ch3.pad_top_du     = TOP_PAD;
    trans_ch3.pad_bottom_du  = BOTTOM_PAD;
    trans_ch3.pad_left_du    = LEFT_PAD;
    trans_ch3.pad_right_du   = RIGHT_PAD;
    trans_ch3.win_du         = 0;
    trans_ch3.end            = DMA_TRANS_END_INTR;
    trans_ch3.dim_inv        = TRANSPOSITION_EN;
    trans_ch3.channel        = DMA_CH3_IDX;
    
    dma_init(NULL);
    
    #if EN_PERF

    dma_validate_transaction(&trans_ch0, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    dma_validate_transaction(&trans_ch1, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    dma_validate_transaction(&trans_ch2, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    dma_validate_transaction(&trans_ch3, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    dma_load_transaction(&trans_ch0);
    dma_load_transaction(&trans_ch1);
    dma_load_transaction(&trans_ch2);
    dma_load_transaction(&trans_ch3);
    dma_launch(&trans_ch0);
    dma_launch(&trans_ch1);
    dma_launch(&trans_ch2);
    dma_launch(&trans_ch3);
    
    #else

    res_valid = dma_validate_transaction(&trans_ch0, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res_valid, res_valid == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_valid = dma_validate_transaction(&trans_ch1, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res_valid, res_valid == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_valid = dma_validate_transaction(&trans_ch2, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res_valid, res_valid == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_valid = dma_validate_transaction(&trans_ch3, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res_valid, res_valid == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_load = dma_load_transaction(&trans_ch0);
    PRINTF("load: %u \t%s\n\r", res_load, res_load == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_load = dma_load_transaction(&trans_ch1);
    PRINTF("load: %u \t%s\n\r", res_load, res_load == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_load = dma_load_transaction(&trans_ch2);
    PRINTF("load: %u \t%s\n\r", res_load, res_load == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_load = dma_load_transaction(&trans_ch3);
    PRINTF("load: %u \t%s\n\r", res_load, res_load == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_launch = dma_launch(&trans_ch0);
    PRINTF("laun: %u \t%s\n\r", res_launch, res_launch == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_launch = dma_launch(&trans_ch1);
    PRINTF("laun: %u \t%s\n\r", res_launch, res_launch == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_launch = dma_launch(&trans_ch2);
    PRINTF("laun: %u \t%s\n\r", res_launch, res_launch == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_launch = dma_launch(&trans_ch3);
    PRINTF("laun: %u \t%s\n\r", res_launch, res_launch == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    #endif

    /* Wait for CH3 to end */
    while(!dma_is_ready(3)) {
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

    /* Run the same computation on the CPU for non transposed cases*/
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
            }
            else
            {
                copied_data_2D_CPU_ch0[dst_ptr] = test_data[src_ptr];
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
                copied_data_2D_CPU_ch1[dst_ptr] = 0;
            }
            else
            {
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

    /* Run the same computation on the CPU for the transposed cases*/
    j_in_last = -1;
    stride_2d_cnt = 0;
    left_pad_cnt = 0;
    j_in = 0;
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
                copied_data_2D_CPU_ch2[dst_ptr] = 0;
            }
            else
            {
                copied_data_2D_CPU_ch2[dst_ptr] = test_data[src_ptr];
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

    j_in_last = -1;
    stride_2d_cnt = 0;
    left_pad_cnt = 0;
    j_in = 0;
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
                copied_data_2D_CPU_ch3[dst_ptr] = 0;
            }
            else
            {
                copied_data_2D_CPU_ch3[dst_ptr] = test_data[src_ptr];
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

    #endif

    #if EN_VERIF

    /* Verify that the DMA and the CPU outputs are the same */
    for (int i = 0; i < OUT_D2_PAD_STRIDE; i++) {
        for (int j = 0; j < OUT_D1_PAD_STRIDE; j++) {
            if ((copied_data_2D_DMA_ch0[i * OUT_D1_PAD_STRIDE + j] != copied_data_2D_CPU_ch0[i * OUT_D1_PAD_STRIDE + j]) &
                (copied_data_2D_DMA_ch1[i * OUT_D1_PAD_STRIDE + j] != copied_data_2D_CPU_ch1[i * OUT_D1_PAD_STRIDE + j]) &
                (copied_data_2D_DMA_ch2[i * OUT_D1_PAD_STRIDE + j] != copied_data_2D_CPU_ch2[i * OUT_D1_PAD_STRIDE + j]) &
                (copied_data_2D_DMA_ch3[i * OUT_D1_PAD_STRIDE + j] != copied_data_2D_CPU_ch3[i * OUT_D1_PAD_STRIDE + j]))
            {
                passed = 0;
            }
        }
    }

    if (passed) {
        PRINTF("Success test 3\n\n\r");
    } 
    else 
    {
        PRINTF("Fail test 3\n\r");
        return EXIT_FAILURE;
    }
    #endif

    #endif

    #if defined(TEST_ID_4) && (TARGET_SIM == 0 || defined(TARGET_QUESTASIM))

    /* Testing SPI2RAM & RAM2RAM operations on 2 channels */

    /* Reset for third test */
    passed = 1;
    i_in = 0;
    j_in = 0;
    left_pad_cnt = 0;
    top_pad_cnt = 0;
    stride_1d_cnt = 0;
    stride_2d_cnt = 0;
    for (int i = 0; i < OUT_DIM_2D; i++) {
        copied_data_2D_DMA_ch1[i] = 0;
        copied_data_2D_CPU_ch1[i] = 0;
    }

    #if EN_PERF

    /* Reset the counter to evaluate the performance of the DMA */
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    #endif

    tgt_src.ptr            = &test_data[0];
    tgt_src.inc_du         = SRC_INC_D1;
    tgt_src.inc_d2_du      = SRC_INC_D2;
    tgt_src.size_du        = SIZE_EXTR_D1;
    tgt_src.size_d2_du     = SIZE_EXTR_D2;
    tgt_src.trig           = DMA_TRIG_MEMORY;
    tgt_src.type           = DMA_DATA_TYPE;

    tgt_dst_ch1.ptr            = copied_data_2D_DMA_ch1;
    tgt_dst_ch1.inc_du         = DST_INC_D1;
    tgt_dst_ch1.inc_d2_du      = DST_INC_D2;
    tgt_dst_ch1.trig           = DMA_TRIG_MEMORY;

    trans_ch1.src            = &tgt_src;
    trans_ch1.dst            = &tgt_dst_ch1;
    trans_ch1.mode           = DMA_TRANS_MODE_SINGLE;
    trans_ch1.dim            = DMA_DIM_CONF_2D;
    trans_ch1.pad_top_du     = TOP_PAD;
    trans_ch1.pad_bottom_du  = BOTTOM_PAD;
    trans_ch1.pad_left_du    = LEFT_PAD;
    trans_ch1.pad_right_du   = RIGHT_PAD;
    trans_ch1.win_du         = 0;
    trans_ch1.end            = DMA_TRANS_END_INTR;
    trans_ch1.channel        = DMA_CH1_IDX;
    
    /* Initialize the SPI */
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

    if ( get_spi_flash_mode(&soc_ctrl) == SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO ) {
        PRINTF("This application cannot work with the memory mapped SPI FLASH"
            "module - do not use the FLASH_EXEC linker script for this application\n");
        return EXIT_SUCCESS;
    }

    /* Pick the correct spi device based on simulation type */
    spi_host_t spi;
    #ifndef USE_SPI_FLASH
    spi.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    #else
    spi.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    #endif

    /* Init SPI host and SPI<->Flash bridge parameters */
    if (w25q128jw_init(spi) != FLASH_OK)
    {
        PRINTF("Error initializing the flash SPI\n\r");
        return EXIT_FAILURE;
    }

    /* Start the reading process from the SPI, avoiding both sanity checks and waiting for the DMA to finish */
    w25q_error_codes_t status = w25q128jw_read_standard_dma(TEST_DATA_FLASH_PTR, copied_test_data_flash, TEST_DATA_FLASH_SIZE*4, 1, 1);
    if (status != FLASH_OK)
    {
        PRINTF("Error reading from flash\n\r");
        return EXIT_FAILURE;
    }
    
    #if EN_PERF

    dma_validate_transaction(&trans_ch1, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    dma_load_transaction(&trans_ch1);
    dma_launch(&trans_ch1);
    
    #else

    res_valid = dma_validate_transaction(&trans_ch1, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("tran: %u \t%s\n\r", res_valid, res_valid == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_load = dma_load_transaction(&trans_ch1);
    PRINTF("load: %u \t%s\n\r", res_load, res_load == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res_launch = dma_launch(&trans_ch1);
    PRINTF("laun: %u \t%s\n\r", res_launch, res_launch == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    #endif

    /* Wait for CH1 to end */
    while(!dma_is_ready(0)) {
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
                copied_data_2D_CPU_ch1[dst_ptr] = 0;
            }
            else
            {
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

    #endif

    #if EN_VERIF

    /* Verify that the DMA and the CPU outputs are the same */
    for (int i = 0; i < OUT_D2_PAD_STRIDE; i++) {
        for (int j = 0; j < OUT_D1_PAD_STRIDE; j++) {
            if ((copied_data_2D_DMA_ch1[i * OUT_D1_PAD_STRIDE + j] != copied_data_2D_CPU_ch1[i * OUT_D1_PAD_STRIDE + j])) 
            {
                passed = 0;
            }
        }
    }

    /* Verify that the SPI copy was successful */
    for (int i=0; i < TEST_DATA_FLASH_SIZE; i++)
    {
        if (copied_test_data_flash[i] != test_data_flash[i])
        {
            passed = 0;
        }
    }

    if (passed) {
        PRINTF("Success test 4\n\r");
    } 
    else 
    {
        PRINTF("Fail test 4\n\r");
        return EXIT_FAILURE;
    }
    #endif

    #endif

    return EXIT_SUCCESS;
}