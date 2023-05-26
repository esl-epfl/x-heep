/*
 * Copyright EPFL contributors.
 * Licensed under the Apache License, Version 2.0, see LICENSE for details.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Author: Tim Frey <tim.frey@epfl.ch>
 */


/**
 * This is a example for a i2s microphone.
 * 
 * Is recording an audiosample of a given size and then outputing it over UART.
 */

#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "i2s.h"
#include "i2s_structs.h"


#ifdef TARGET_PYNQ_Z2
#define I2S_TEST_BATCH_SIZE    128
#define I2S_TEST_BATCHES      16
#define I2S_CLK_DIV           8
#define AUDIO_DATA_NUM 2048                          // RECORDING LENGTH
// #define AUDIO_DATA_NUM 0x18000  // max 0x1c894                          // RECORDING LENGTH
#define I2S_USE_INTERRUPT true
#define USE_DMA
#else
#define I2S_TEST_BATCH_SIZE    128
#define I2S_TEST_BATCHES      4
#define I2S_CLK_DIV           32
#define AUDIO_DATA_NUM 4
#define I2S_USE_INTERRUPT true
//#define USE_DMA
#endif


#include "mmio.h"
#include "handler.h"
#include "rv_plic.h"
#include "csr.h"
#include "hart.h"

#ifdef USE_DMA
#include "dma.h"
#include "dma_regs.h"
#endif
#include "fast_intr_ctrl.h"



// Interrupt controller variables
plic_result_t plic_res;
plic_irq_id_t intr_num;


// I2s
int i2s_interrupt_flag = 0;


int32_t audio_data_0[AUDIO_DATA_NUM] __attribute__ ((aligned (4)))  = { 0 };


// DMA
#ifdef USE_DMA
dma_t dma;
int8_t dma_intr_flag;
#endif

//
// ISR
//
void handler_irq_ext(void) {
    i2s_interrupt_flag = 1;
    printf(".");
}

#ifdef USE_DMA
void fic_irq_dma(void)
{
    dma_intr_flag = 1;
}
#endif


//
// Setup
//
void setup() 
{

    #ifdef USE_DMA
    dma.base_addr = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);

     // -- DMA CONFIGURATION --
    dma_set_read_ptr_inc(&dma, (uint32_t) 0); // Do not increment address when reading from the SPI (Pop from FIFO)
    dma_set_write_ptr_inc(&dma, (uint32_t) 4);
    dma_set_read_ptr(&dma, I2S_RX_DATA_ADDRESS); // I2s RX FIFO addr
    dma_set_write_ptr(&dma, (uint32_t) audio_data_0); // audio data address
    dma_set_slot(&dma, DMA_I2S_RX_SLOT, 0); // The DMA will wait for the I2s RX FIFO valid signal
    dma_set_data_type(&dma, (uint32_t) 0);
    #endif


    // PLIC
    plic_Init();
    plic_res = plic_irq_set_priority(I2S_INTR_EVENT, 1);
    plic_res = plic_irq_set_enabled(I2S_INTR_EVENT, kPlicToggleEnabled);


    // enable I2s interrupt
    i2s_interrupt_flag = 0;
    i2s_init(I2S_CLK_DIV, I2S_32_BITS);
    i2s_rx_enable_watermark(AUDIO_DATA_NUM, I2S_USE_INTERRUPT);


    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    // bit 11 = external intr (used for i2s)
    // bit 19 = dma fast intr
    const uint32_t mask = 1 << 11 | 1 << 19; 
    CSR_SET_BITS(CSR_REG_MIE, mask);
}

#define I2S_WAIT_TIME_US 100000 
#define I2S_WAIT_CYCLES  ((1.5 * REFERENCE_CLOCK_Hz / 4))
// 1500000

int main(int argc, char *argv[]) {
    bool success = true;

#ifdef TARGET_PYNQ_Z2
    for (uint32_t i = 0; i < 0x10000; i++) asm volatile("nop");
#endif
    printf("I2s DEMO\r\n");

    setup();

    //printf("Setup done!\r\n");

#ifdef TARGET_PYNQ_Z2
    

    for (uint32_t i = 0; i < I2S_WAIT_CYCLES; i++) asm volatile("nop");

    //
    // FPGA code
    //
    #pragma message ( "this application never ends" )

    int batch = 0;
    while(1) {
        i2s_rx_start(I2S_LEFT_CH);

        #ifdef USE_DMA
        dma_set_cnt_start(&dma, (uint32_t) (AUDIO_DATA_NUM*4)); // start 

        // WAITING FOR DMA COPY TO FINISH
        while(!dma_intr_flag) {
            wait_for_interrupt();
        }
        dma_intr_flag = 0;
        #else
        // READING DATA MANUALLY OVER BUS
        for (int i = 0; i < AUDIO_DATA_NUM; i+=1) {
            if (i != i2s_rx_read_waterlevel()) printf("Waterlevel wrong\r\n");
            while (! i2s_rx_data_available()) { }
            audio_data_0[i] = i2s_rx_read_data();
        }
        #endif



        printf("index,data\r\n"); // <- csv header for python 
        int32_t* data = audio_data_0;
        for (int i = 0; i < AUDIO_DATA_NUM; i+=1) {
            printf("%4x,%d\r\n", i, (int16_t) (data[i] >> 16));
        }
        batch += 1;

        if (i2s_interrupt_flag) {
            printf("irq 1\n");
            i2s_interrupt_flag = 0;
        }

        break;

        i2s_rx_stop();

        #ifdef USE_DMA
        dma_set_cnt_start(&dma, (uint32_t) (AUDIO_DATA_NUM*4)); // restart 
        #endif
    }
#else
    //
    // Verilator Code
    //

    bool mic_connected = false;

    for (int batch = 0; batch < I2S_TEST_BATCHES; batch++) {
        i2s_rx_start(I2S_BOTH_CH);

        #ifdef USE_DMA
        dma_set_cnt_start(&dma, (uint32_t) (AUDIO_DATA_NUM*4)); // start 

        // WAITING FOR DMA COPY TO FINISH
        while(!dma_intr_flag) {
            wait_for_interrupt();
        }
        dma_intr_flag = 0;
        #else
        // READING DATA MANUALLY OVER BUS
        for (int i = 0; i < AUDIO_DATA_NUM; i+=1) {
            if (i != i2s_rx_read_waterlevel()) printf("Waterlevel wrong\r\n");
            while (!i2s_rx_data_available()) { }
            audio_data_0[i] = i2s_rx_read_data();
        }
        #endif
        i2s_rx_stop();

        if (i2s_interrupt_flag) {
            printf("irq 1\r\n");
            i2s_interrupt_flag = 0;
        }


        printf("B%x\r\n", batch);
        
        int32_t* data = audio_data_0;
        for (int i = 0; i < AUDIO_DATA_NUM; i+=2) {
            printf("0x%x 0x%x\r\n", data[i], data[i+1]);
            if (data[i] != 0) {
                mic_connected = true; // the microphone testbench is connected
                if (data[i] != 0x8765431) {
                    printf("ERROR left sample %d (B%d) = 0x%08x != 0x8765431\r\n", i, batch, data[i]);
                    success = false;
                }
            }
            if (data[i+1] != 0) {
                mic_connected = true; // the microphone testbench is connected
                if (data[i+1] != 0xfedcba9) {
                    printf("ERROR left sample data[%d] = 0x%08x != 0xfedcba9\r\n", i+1, batch, data[i+1]);
                    success = false;
                }
            }
        }
        #ifdef USE_DMA
        dma_set_cnt_start(&dma, (uint32_t) (AUDIO_DATA_NUM*4)); // restart 
        #endif
    }

    if (! mic_connected) {
        printf("WARNING: Please build with `make verilator-sim FUSESOC_FLAGS=\"--flag=use_external_device_example\"\r\n");
    }
#endif
    i2s_terminate();

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

