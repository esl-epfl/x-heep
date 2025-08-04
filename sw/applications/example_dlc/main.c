// Copyright 2024 EPFL and Politecnico di Torino
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: example_dlc.c
// Author: Alessio Naclerio
// Date: 28/03/2025
// Description: Example application to test the digital Level Crossing (dLC) IP
//              along with the DMA Hardware Fifo Mode.

#include <stdio.h>
#include <stdlib.h>

#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "dlc.h"
#include "rv_plic.h"
#include "test_ecg.h"
#include "math.h"

/* 
 * Enable one or more of the following tests by defining the correct TEST_ID_* macro: 
 *  
 * 0: Test the dLC tighly-coupled accelerator with the DMA in single transaction mode
 * 1: Test the dLC tighly-coupled accelerator with the DMA in circular transaction mode
 */

#define TEST_ID_0
#define TEST_ID_1

#define PRINTF_IN_SIM 0
#define PRINTF_IN_FPGA 1

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#define DLC_START_ADDRESS EXT_PERIPHERAL_START_ADDRESS + 0x5000

#define DATA_SIZE 3748
#define DLC_BUFFER_SIZE 500
#define DLC_WINDOWS 8

// dLC results buffers
int16_t dlc_results_buffer[DLC_BUFFER_SIZE];
int16_t dlc_circular_mode_results_buffer[DLC_WINDOWS][DLC_BUFFER_SIZE];
    
// dLC programming registers
uint32_t* dlvl_log_level_width    = DLC_START_ADDRESS + DLC_DLVL_LOG_LEVEL_WIDTH_REG_OFFSET;
uint32_t* dlvl_discard_bits       = DLC_START_ADDRESS + DLC_DISCARD_BITS_REG_OFFSET;
uint32_t* dlvl_n_bits             = DLC_START_ADDRESS + DLC_DLVL_N_BITS_REG_OFFSET;
uint32_t* dlvl_format             = DLC_START_ADDRESS + DLC_DLVL_FORMAT_REG_OFFSET;
uint32_t* dlvl_mask               = DLC_START_ADDRESS + DLC_DLVL_MASK_REG_OFFSET;
uint32_t* dt_mask                 = DLC_START_ADDRESS + DLC_DT_MASK_REG_OFFSET;
uint32_t* dlc_size                = DLC_START_ADDRESS + DLC_TRANS_SIZE_REG_OFFSET;

dma_target_t tgt_src;
dma_target_t tgt_dst;
dma_trans_t trans;

dma *peri = dma_peri(0);

volatile char trans_count = 0;

/* Strong transaction ISR implementation */
void dma_intr_handler_trans_done(uint8_t channel)
{
    trans_count ++;
    if (trans_count == DLC_WINDOWS)
    {
      /* Stop circular mode by setting it to single */
      write_register( DMA_TRANS_MODE_SINGLE,
                      DMA_MODE_REG_OFFSET,
                      DMA_MODE_MODE_MASK,
                      DMA_MODE_MODE_OFFSET,
                      peri );
    }
    return;
}

int main() {
 
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    CSR_SET_BITS(CSR_REG_MIE, (1 << 19) | (1 << 30) | (1 << 31));

    #ifdef TEST_ID_0

    PRINTF("Starting dLC test 0...\n\r");
    
    // dLC programming
    // dlvl_format: if set to '1' the result data for delta-levels are in two's complement format
    //              if set to '0' the result data for delta-levels are in sign and modulo format
    *dlvl_format = LC_PARAMS_DATA_IN_TWOS_COMPLEMENT;
    // dlvl_log_level_width: log2 of the delta-levels width
    *dlvl_log_level_width = LC_PARAMS_LC_LEVEL_WIDTH_BY_BITS;
    *dlvl_discard_bits = 0;
    // dlvl_n_bits: number of bits for the delta-levels field
    //              if dlvl_format is set to '1' the number of bits for the delta-levels is dlvl_n_bits
    //              if dlvl_format is set to '0' the number of bits for the delta-levels is dlvl_n_bits - 1 to account for the sign bit 
    *dlvl_n_bits = (LC_PARAMS_DATA_IN_TWOS_COMPLEMENT) ? LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_AMPLITUDE:
                        LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_AMPLITUDE - 1;
    // dlvl_mask: mask for the delta-levels field (it has as many bits set to 1 as the number of bits for the delta-levels field)
    *dlvl_mask = (1 << (*dlvl_n_bits)) - 1;
    // dt_mask: mask for the delta-time field (it has as many bits set to 1 as the number of bits for the delta-time field)
    *dt_mask = (1 << (LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_TIME)) - 1;
    *dlc_size = DATA_SIZE;

    tgt_src.ptr = (uint8_t *) ecg_data;
    tgt_src.inc_d1_du = 1;
    tgt_src.trig = DMA_TRIG_MEMORY;    
    tgt_src.type = DMA_DATA_TYPE_HALF_WORD;
    
    tgt_dst.ptr = (uint8_t *) dlc_results_buffer;
    tgt_dst.inc_d1_du = 1;
    tgt_dst.trig = DMA_TRIG_MEMORY;
    tgt_dst.type = DMA_DATA_TYPE_HALF_WORD;

    trans.src        = &tgt_src;
    trans.dst        = &tgt_dst;
    trans.mode       = DMA_TRANS_MODE_SINGLE;
    trans.hw_fifo_en = 1;
    trans.dim        = DMA_DIM_CONF_1D;
    trans.size_d1_du = DATA_SIZE;
    trans.end        = DMA_TRANS_END_INTR;

    dma_init(NULL);

    if(dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY) != DMA_CONFIG_OK){
        PRINTF("Error: dma_validate_transaction\n");
        return EXIT_FAILURE;
    }
    if(dma_load_transaction(&trans) != DMA_CONFIG_OK){
        PRINTF("Error: dma_load_transaction\n");
        return EXIT_FAILURE;
    }
    if(dma_launch(&trans) != DMA_CONFIG_OK){
        PRINTF("Error: dma_launch\n");
        return EXIT_FAILURE;
    }

    while(!dma_is_ready(0)) {        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    // Checking  the results
    for (int i = 0; i < LC_STATS_CROSSINGS; i++)
    {
        if(dlc_results_buffer[i] != lc_data_for_storage_data[i])
        {
            PRINTF("Error at position %d: dlc result is %d, golden result is %d\n", i, dlc_results_buffer[i], lc_data_for_storage_data[i]);
            return EXIT_FAILURE;
        }
    }

    PRINTF("Success test 0!\n\r");

    #endif

    #ifdef TEST_ID_1

    PRINTF("Starting dLC test 1...\n\r");

    dma_init(NULL);

    *dlc_size = DLC_BUFFER_SIZE;

    tgt_src.ptr = (uint8_t *) ecg_data;
    tgt_src.inc_d1_du = 1;
    tgt_src.trig = DMA_TRIG_MEMORY;    
    tgt_src.type = DMA_DATA_TYPE_HALF_WORD;
    
    tgt_dst.ptr = (uint8_t *) dlc_circular_mode_results_buffer[trans_count][0];
    tgt_dst.inc_d1_du = 1;
    tgt_dst.trig = DMA_TRIG_MEMORY;
    tgt_dst.type = DMA_DATA_TYPE_HALF_WORD;

    trans.src        = &tgt_src;
    trans.dst        = &tgt_dst;
    trans.mode       = DMA_TRANS_MODE_CIRCULAR;
    trans.hw_fifo_en = 1;
    trans.dim        = DMA_DIM_CONF_1D;
    trans.size_d1_du = DLC_BUFFER_SIZE;
    trans.end        = DMA_TRANS_END_INTR;

    dma_init(NULL);

    if(dma_load_transaction(&trans) != DMA_CONFIG_OK){
        PRINTF("Error: dma_load_transaction\n");
        return EXIT_FAILURE;
    }
    if(dma_launch(&trans) != DMA_CONFIG_OK){
        PRINTF("Error: dma_launch\n");
        return EXIT_FAILURE;
    }

    while(!dma_is_ready(0)) {        
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready(0) == 0 ) {
            wait_for_interrupt();
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    // Extract the result from the buffers and combine them to a single buffer
    uint16_t count = 0;

    for (int i = 0; i < DLC_WINDOWS; i ++)
    {
      for (int j = 0; j < DLC_BUFFER_SIZE; j++)
      {
        if (dlc_circular_mode_results_buffer[i][j] != 0)
        {
          dlc_results_buffer[count] = dlc_circular_mode_results_buffer[i][j];
          count ++;
        }
      }
    }

    // Checking  the results
    for (int i = 0; i < LC_STATS_CROSSINGS; i++)
    {
        if(dlc_results_buffer[i] != lc_data_for_storage_data[i])
        {
            PRINTF("Error at position %d: dlc result is %d, golden result is %d\n", i, dlc_results_buffer[i], lc_data_for_storage_data[i]);
            return EXIT_FAILURE;
        }
    }

    PRINTF("Success test 1!\n\r");

    #endif

    return EXIT_SUCCESS;
}
