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

int main() {
 
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    CSR_SET_BITS(CSR_REG_MIE, (1 << 11) | (1 << 19));

    // dLC results buffer
    int16_t dlc_results[500];
    
    // dLC programming registers
    uint32_t* dlvl_log_level_width    = DLC_START_ADDRESS + DLC_DLVL_LOG_LEVEL_WIDTH_REG_OFFSET;
    uint32_t* dlvl_n_bits             = DLC_START_ADDRESS + DLC_DLVL_N_BITS_REG_OFFSET;
    uint32_t* dlvl_format             = DLC_START_ADDRESS + DLC_DLVL_FORMAT_REG_OFFSET;
    uint32_t* dlvl_mask               = DLC_START_ADDRESS + DLC_DLVL_MASK_REG_OFFSET;
    uint32_t* dt_mask                 = DLC_START_ADDRESS + DLC_DT_MASK_REG_OFFSET;
    uint32_t* dlc_rnw                 = DLC_START_ADDRESS + DLC_READNOTWRITE_REG_OFFSET;
    
    // dLC programming
    // dlvl_format: if set to '1' the result data for delta-levels are in two's complement format
    //              if set to '0' the result data for delta-levels are in sign and modulo format
    *dlvl_format = LC_PARAMS_DATA_IN_TWOS_COMPLEMENT;
    // dlvl_log_level_width: log2 of the delta-levels width
    *dlvl_log_level_width = LC_PARAMS_LC_LEVEL_WIDTH_BY_BITS;
    // dlvl_n_bits: number of bits for the delta-levels field
    //              if dlvl_format is set to '1' the number of bits for the delta-levels is dlvl_n_bits
    //              if dlvl_format is set to '0' the number of bits for the delta-levels is dlvl_n_bits - 1 to account for the sign bit 
    *dlvl_n_bits = (LC_PARAMS_DATA_IN_TWOS_COMPLEMENT) ? LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_AMPLITUDE:
                        LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_AMPLITUDE - 1;
    // dlvl_mask: mask for the delta-levels field (it has as many bits set to 1 as the number of bits for the delta-levels field)
    *dlvl_mask = (1 << (*dlvl_n_bits)) - 1;
    // dt_mask: mask for the delta-time field (it has as many bits set to 1 as the number of bits for the delta-time field)
    *dt_mask = (1 << (LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_TIME)) - 1;
    // dlc_rnw: if set to '1' the dLC decrements DMA downcounter each time it reads data from the HW_READ_FIFO
    //          if set to '0' the dLC decrements DMA downcounter each time it write data to the HW_WRITE_FIFO
    *dlc_rnw = 1;


    dma_target_t tgt_src;
    dma_target_t tgt_dst;
    dma_trans_t trans;

    tgt_src.ptr = (uint8_t *) ecg_data;
    tgt_src.inc_d1_du = 1;
    tgt_src.trig = DMA_TRIG_MEMORY;    
    tgt_src.type = DMA_DATA_TYPE_HALF_WORD;
    
    tgt_dst.ptr = (uint8_t *) dlc_results;
    tgt_dst.inc_d1_du = 1;
    tgt_dst.trig = DMA_TRIG_MEMORY;
    tgt_dst.type = DMA_DATA_TYPE_HALF_WORD;

    trans.src        = &tgt_src;
    trans.dst        = &tgt_dst;
    trans.mode       = DMA_TRANS_MODE_HW_FIFO;
    trans.dim        = DMA_DIM_CONF_1D;
    trans.size_d1_du = 3748;
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
        if(dlc_results[i] != lc_data_for_storage_data[i])
        {
            printf("Error at position %d: dlc result is %d, golden result is %d\n", i, dlc_results[i], lc_data_for_storage_data[i]);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
