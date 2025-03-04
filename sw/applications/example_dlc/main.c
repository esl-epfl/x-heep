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
#include "handler.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "rv_plic_structs.h"
#include "hart.h"
#include "fast_intr_ctrl.h"
//#include "test_sin_16to8.h"
#include "test_ecg.h"

#define PRINTF_IN_SIM 1 // TOBE CHANGED
#define PRINTF_IN_FPGA 0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#define DLC_RESULTS_SIZE 7499
#define CIRCULAR_BUFFER_SIZE 160
#define WINDOW_SIZE_DU 80

int16_t dlc_circular_buffer[DLC_RESULTS_SIZE];
int16_t * ptr_window_0 = &dlc_circular_buffer[0];
int16_t * ptr_window_1 = &dlc_circular_buffer[0] + (CIRCULAR_BUFFER_SIZE >> 2); // TODO: Check if this is correct

volatile bool is_w0_free;
volatile bool is_w1_free;
volatile bool is_w0_processed;
volatile bool is_w1_processed;
volatile bool is_dma_finished;
volatile int count;


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

void dma_intr_handler_window_done(uint8_t channel) {
    count++;
    if(!is_w0_free) {
        is_w0_free = true;
        is_w1_free = false;
    }else if(!is_w1_free){
        is_w1_free = true;
        is_w0_free = false;
    }
}

void dma_intr_handler_trans_done(uint8_t channel){
    is_dma_finished = true;
}

int main() {
    /* Enable global interrupt. */
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    /* Enable machine-level fast interrupt. */
    CSR_SET_BITS(CSR_REG_MIE, (1 << 11) | (1 << 19));

    plic_Init();
    plic_irq_set_priority(DMA_WINDOW_INTR, 1);
    plic_irq_set_enabled(DMA_WINDOW_INTR, kPlicToggleEnabled);

    is_w0_free = false;
    is_w1_free = false;
    is_w0_processed = false;
    is_w1_processed = false;
    is_dma_finished = false;

    count = 0;
    
    uint32_t* reg_log_wl         = EXT_PERIPHERAL_START_ADDRESS + 0x5000;
    uint32_t* reg_dlvl_bits      = EXT_PERIPHERAL_START_ADDRESS + 0x5004;
    uint32_t* reg_dt_bits        = EXT_PERIPHERAL_START_ADDRESS + 0x5008;
    uint32_t* reg_signed         = EXT_PERIPHERAL_START_ADDRESS + 0x500C;
    uint32_t* reg_output_format  = EXT_PERIPHERAL_START_ADDRESS + 0x5010;
    uint32_t* reg_dlvl_bits_mask = EXT_PERIPHERAL_START_ADDRESS + 0x5014;
    uint32_t* reg_dt_bits_mask   = EXT_PERIPHERAL_START_ADDRESS + 0x5018;
    uint32_t* reg_read_write     = EXT_PERIPHERAL_START_ADDRESS + 0x501C;
    
    *reg_signed = LC_PARAMS_DATA_IN_TWOS_COMPLEMENT;
    *reg_log_wl = LC_PARAMS_LC_LEVEL_WIDTH_BY_BITS;
    *reg_dlvl_bits = (LC_PARAMS_DATA_IN_TWOS_COMPLEMENT) ? LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_AMPLITUDE:
                        LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_AMPLITUDE - 1;
    *reg_dt_bits = LC_PARAMS_LC_ACQUISITION_WORD_SIZE_OF_TIME;
    *reg_dlvl_bits_mask = (1 << (*reg_dlvl_bits)) - 1;
    *reg_dt_bits_mask = (1 << (*reg_dt_bits)) - 1;
    *reg_output_format = LC_PARAMS_SIZE_PER_SAMPLE_BITS == 16 ? 1 : 0; // 0 -> 8 bit / 1 -> 16 bit
    *reg_read_write = 1;

    dma* dma_peri;
    dma_peri = dma_peri(0);
    dma_init(dma_peri);
    // Sign Extension: maybe we do not care 
    dma_peri -> SIGN_EXT = 0;
    // DMA Mode: Hw Fifo (+ Subaddress + Circular)
    dma_peri -> MODE = DMA_TRANS_MODE_HW_FIFO;
    // DMA Transfer Dimension: 1D
    dma_peri -> DIM_CONFIG = DMA_DIM_CONF_1D;
    // DMA Interrupt Enable: 1 or 3 if window intr enabled
    dma_peri -> INTERRUPT_EN = 1;
    // DMA Slot: 0 for now (SPI then)
    dma_peri -> SLOT = 0;
    // Source DMA Pointer: sin_data
    dma_peri -> SRC_PTR = ecg_data;
    // Destination DMA Pointer: memory buffer
    dma_peri -> DST_PTR = dlc_circular_buffer;
    // Source Data Type: Half Word
    dma_peri -> SRC_DATA_TYPE = DMA_DATA_TYPE_HALF_WORD;
    // Destination Data Type: Byte
    dma_peri -> DST_DATA_TYPE = DMA_DATA_TYPE_HALF_WORD;
    // Source Address Increment: 1 for now (0 if SPI)
    dma_peri -> SRC_PTR_INC_D1 = 2;
    // Destination Address Increment: 1
    dma_peri -> DST_PTR_INC_D1 = 2;
    // Size of the DMA Transfer Window: 80
    //dma_peri -> WINDOW_SIZE = WINDOW_SIZE_DU;
    // Size of the whole DMA Transfer Circular Buffer: 160
    dma_peri -> SIZE_D1 = DLC_RESULTS_SIZE;

    while(!is_dma_finished){
        /* if(is_w0_free && !is_w0_processed) {
            for (int i = 0; i < CIRCULAR_BUFFER_SIZE >> 2; i++) {
                ptr_window_0[i] = i;
            }
            is_w0_processed = true;
            is_w1_processed = false;
        } else if (is_w1_free && !is_w1_processed) {
            for (int i = 0; i < CIRCULAR_BUFFER_SIZE >> 2; i++) {
                ptr_window_1[i] = i;
            }
            is_w1_processed = true;
            is_w0_processed = false;
        } */
    }

    /* for(int i = 0; i < DLC_RESULTS_SIZE; i++) {
        if(lc_data_for_storage_data[i] != dlc_circular_buffer[i]) {
            PRINTF("Error at position %d, got %d instead of %d\n", i, dlc_circular_buffer[i], lc_data_for_storage_data[i]);
            //return EXIT_FAILURE;
        }
    } */

    //printf("%d\n", count);
    return 0;
}
