/**
 * @file main.c
 * @brief 
 *
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "rv_plic.h"
#include "sin_1_8_LC_1_2_FM_8_l_0_3_4.h"

int8_t dlc_data_buffer[240];

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif


void protected_wait_for_dma_interrupt(void)
{
    while (!dma_is_ready(0))
    {
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if (!dma_is_ready(0))
        {
            wait_for_interrupt();
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }
}

int main() {

    uint32_t* reg_log_wl         = EXT_PERIPHERAL_START_ADDRESS + 0x5000;
    uint32_t* reg_dlvl_bits      = EXT_PERIPHERAL_START_ADDRESS + 0x5004;
    uint32_t* reg_dt_bits        = EXT_PERIPHERAL_START_ADDRESS + 0x5008;
    uint32_t* reg_signed         = EXT_PERIPHERAL_START_ADDRESS + 0x500C;
    uint32_t* reg_output_format  = EXT_PERIPHERAL_START_ADDRESS + 0x5010;
    uint32_t* reg_dlvl_bits_mask = EXT_PERIPHERAL_START_ADDRESS + 0x5014;
    uint32_t* reg_dt_bits_mask   = EXT_PERIPHERAL_START_ADDRESS + 0x5018;
    
    *reg_signed = LC_PARAMS_DATA_IN_TWOS_COMPLEMENT;
    *reg_log_wl = LC_PARAMS_LC_LEVEL_WIDTH_BY_BITS;
    *reg_dlvl_bits = (LC_PARAMS_DATA_IN_TWOS_COMPLEMENT) ? LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_AMPLITUDE:
                        LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_AMPLITUDE - 1;
    *reg_dt_bits = LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_TIME;
    *reg_dlvl_bits_mask = (1 << (*reg_dlvl_bits)) - 1;
    *reg_dt_bits_mask = (1 << (*reg_dt_bits)) - 1;
    *reg_output_format = 0; // 0 -> 8 bit / 1 -> 16 bit

    /*
        DMA Transaction in Hardwdare Fifo Continuous Mode
    */

    dma* dma_peri;
    dma_peri = dma_peri(0);
    // Sign Extension: maybe we do not care 
    dma_peri -> SIGN_EXT = 1;
    // DMA Mode: Circular + Hw Fifo (+ Subaddress)
    dma_peri -> MODE = 5;
    // DMA Transfer Dimension: 1D
    dma_peri -> DIM_CONFIG = DMA_DIM_CONF_1D;
    // DMA Interrupt Enable: Enable
    dma_peri -> INTERRUPT_EN = 1;
    // DMA Slot: 0 for now (SPI then)
    dma_peri -> SLOT = 0;
    // Source DMA Pointer: sin_data
    dma_peri -> SRC_PTR = sin_data;
    // Destination DMA Pointer: memory buffer
    dma_peri -> DST_PTR = dlc_data_buffer;
    // Source Data Type: Byte
    dma_peri -> SRC_DATA_TYPE = DMA_DATA_TYPE_BYTE;
    // Destination Data Type: Byte
    dma_peri -> DST_DATA_TYPE = DMA_DATA_TYPE_BYTE;
    // Source Address Increment: 1 for now (0 if SPI)
    dma_peri -> SRC_PTR_INC_D1 = 1;
    // Destination Address Increment: 1
    dma_peri -> DST_PTR_INC_D1 = 1;

    return 0;
}