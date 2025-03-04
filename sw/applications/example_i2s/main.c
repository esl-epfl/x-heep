/*
 * Copyright EPFL contributors.
 * Licensed under the Apache License, Version 2.0, see LICENSE for details.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Author: Tim Frey <tim.frey@epfl.ch>
 */


/**
 * This is a example for a i2s microphone.
 * It is recording an audiosample of a given size and then outputing it over UART.
 *
 * Tested with SPH0645 microphone module from Adafruit
 * https://www.adafruit.com/product/3421
 *
 * check `pin_assign.xdc` for the pinout of the FPGA to connect:
 * 3V -> 3.3V
 * GND -> GND
 * BCLK -> I2S_SCK
 * LRCL -> I2S_WS
 * DOUT -> I2S_SD
 * SEL -> left floating (pulldown) to transmit data on the left channel
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"

#include "x-heep.h"
#include "i2s.h"
#include "i2s_structs.h"


#ifdef TARGET_IS_FPGA
#define I2S_TEST_BATCH_SIZE    128
#define I2S_TEST_BATCHES      16
#define I2S_CLK_DIV           8
// #define AUDIO_DATA_NUM 2048                          // RECORDING LENGTH
#define AUDIO_DATA_NUM 100//0x18000  // max 0x1c894                          // RECORDING LENGTH
#define I2S_USE_INTERRUPT true
#define USE_DMA
#else
#define I2S_TEST_BATCH_SIZE    128
#define I2S_TEST_BATCHES      4
#define I2S_CLK_DIV           32
#define AUDIO_DATA_NUM 4
#define I2S_USE_INTERRUPT true
#define USE_DMA
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

// Interrupt controller variables
plic_result_t plic_res;
uint32_t intr_num;


// I2s
int i2s_interrupt_flag = 0;
i2s_result_t i2s_res;


int32_t audio_data_0[AUDIO_DATA_NUM] __attribute__ ((aligned (4)))  = { 0 };


// DMA
#ifdef USE_DMA
int8_t dma_intr_flag;
#endif

//
// ISR
//
void handler_irq_i2s(uint32_t id) {
    i2s_interrupt_flag = 1;
}

#ifdef USE_DMA
void dma_intr_handler_trans_done(uint8_t channel)
{
    dma_intr_flag = 1;
}
#endif


static dma_target_t tgt_src;
static dma_target_t tgt_dst;
static dma_trans_t trans;

//
// Setup
//
void setup()
{

    #ifdef USE_DMA

    dma_init(NULL);

     // -- DMA CONFIGURATION --

    tgt_src.ptr        = I2S_RX_DATA_ADDRESS;
    tgt_src.inc_d1_du     = 0;
    tgt_src.trig       = DMA_TRIG_SLOT_I2S;
    tgt_src.type       = DMA_DATA_TYPE_WORD;

    tgt_dst.ptr        = audio_data_0;
    tgt_dst.inc_d1_du     = 1;
    tgt_dst.trig       = DMA_TRIG_MEMORY;
    tgt_dst.type       = DMA_DATA_TYPE_WORD;

    trans.src        = &tgt_src;
    trans.dst        = &tgt_dst;
    trans.size_d1_du    = AUDIO_DATA_NUM;
    trans.end        = DMA_TRANS_END_INTR;

    dma_config_flags_t res;
    res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("Valid:  %d\n\r", res);
    res = dma_load_transaction(&trans);
    PRINTF("Load:   %d\n\r", res);
    #endif


    // PLIC
    plic_Init();
    plic_res = plic_irq_set_priority(I2S_INTR_EVENT, 1);
    plic_res = plic_irq_set_enabled(I2S_INTR_EVENT, kPlicToggleEnabled);


    // enable I2s interrupt
    i2s_interrupt_flag = 0;
    i2s_res = i2s_init(I2S_CLK_DIV, I2S_32_BITS);
    if (i2s_res != kI2sOk) {
        PRINTF("I2S init failed with %d\n\r", i2s_res);
    }
    i2s_rx_enable_watermark(AUDIO_DATA_NUM, I2S_USE_INTERRUPT);

}

#define I2S_WAIT_TIME_US 100000
#define I2S_WAIT_CYCLES  ((1.5 * REFERENCE_CLOCK_Hz / 4))
// 1500000

int main(int argc, char *argv[]) {
    bool success = true;

#ifdef TARGET_IS_FPGA
    for (uint32_t i = 0; i < 0x10000; i++) asm volatile("nop");
#endif
    PRINTF("I2S DEMO\r\n\r");

    setup();

    //PRINTF("Setup done!\r\n\r");

#ifdef TARGET_IS_FPGA


    for (uint32_t i = 0; i < I2S_WAIT_CYCLES; i++) asm volatile("nop");

    //
    // FPGA code
    //
    #pragma message ( "this application never ends" )

    int batch = 0;
    for (int batch = 0; batch < I2S_TEST_BATCHES; batch++){
        PRINTF("starting\r\n\r"); // <- csv header for python
        #ifdef USE_DMA
            dma_launch( &trans );
        #endif // USE_DMA

        i2s_res = i2s_rx_start(I2S_LEFT_CH);
        if (i2s_res != kI2sOk) {
            PRINTF("I2S rx start failed with %d\n\r", i2s_res);
        }

        #ifdef USE_DMA
        // WAITING FOR DMA COPY TO FINISH
        while(!dma_intr_flag) {
            wait_for_interrupt();
        }
        dma_intr_flag = 0;
        #else
        // READING DATA MANUALLY OVER BUS
        for (int i = 0; i < AUDIO_DATA_NUM; i+=1) {
            if (i != i2s_rx_read_waterlevel()) PRINTF("Waterlevel wrong\r\n\r");
            while (! i2s_rx_data_available()) { }
            audio_data_0[i] = i2s_rx_read_data();
        }
        #endif

        if (i2s_rx_overflow()) {
            PRINTF("I2S rx FIFO overflowed\n\r");
        }

        i2s_res = i2s_rx_stop();
        if (i2s_res != kI2sOk) {
            if (i2s_res == kI2sOverflow) {
                PRINTF("I2S rx overflow occured and cleared\n\r");
            }
            else {
                PRINTF("I2S rx stop failed with %d\n\r", i2s_res);
            }
        }


        // this takes wayyy longer than reading the samples, so no continuous mode is possible with UART dump
        int32_t* data = audio_data_0;
        for (int i = 0; i < AUDIO_DATA_NUM; i+=1) {
            PRINTF("%d\r\n\r",(int16_t) (data[i] >> 16));
        }
        batch += 1;
        PRINTF("Batch done!\r\n\r", batch);

        if (i2s_interrupt_flag) {
            PRINTF("irq 1\n\r");
            i2s_interrupt_flag = 0;
        }
        success = 1;
    }
#else
    //
    // Verilator Code
    //

    bool mic_connected = false;

    for (int batch = 0; batch < I2S_TEST_BATCHES; batch++) {
        #ifdef USE_DMA
        dma_launch( &trans );
        #endif // USE_DMA
        i2s_res = i2s_rx_start(I2S_BOTH_CH);
        if (i2s_res != kI2sOk) {
            PRINTF("I2S rx start failed with %d\n\r", i2s_res);
        }
        #ifdef USE_DMA

        // WAITING FOR DMA COPY TO FINISH
        while(!dma_intr_flag) {
            wait_for_interrupt();
        }
        dma_intr_flag = 0;
        #else
        // READING DATA MANUALLY OVER BUS
        for (int i = 0; i < AUDIO_DATA_NUM; i+=1) {
            if (i != i2s_rx_read_waterlevel()) PRINTF("Waterlevel wrong\r\n\r");
            while (!i2s_rx_data_available()) { }
            audio_data_0[i] = i2s_rx_read_data();
        }
        #endif
        if (i2s_rx_overflow()) {
            PRINTF("I2S rx FIFO overflowed\n\r");
        }

        i2s_res = i2s_rx_stop();
        if (i2s_res != kI2sOk) {
            if (i2s_res == kI2sOverflow) {
                PRINTF("I2S rx overflow occured and cleared\n\r");
            }
            else {
                PRINTF("I2S rx stop failed with %d\n\r", i2s_res);
            }
        }

        if (i2s_interrupt_flag) {
            PRINTF("irq 1\r\n\r");
            i2s_interrupt_flag = 0;
        }


        PRINTF("B%x\r\n\r", batch);

        int32_t* data = audio_data_0;
        int32_t data_even = 0;
        int32_t data_odd  = 0;
        for (int i = 0; i < AUDIO_DATA_NUM; i++) {
            PRINTF("0x%x\r\n\r", data[i]);
            if (data[i] != 0) {
                mic_connected = true; // the microphone testbench is connected
            } else {
                if(i == 0) {
                    //check what is the first data to expect, left or right
                    //the testbench alternatively sends these 2 data, catching which one is first for each BATCH
                    if ( (data[0] == 0x8765431) || (data[0] == 0xfedcba9) ) {
                        if (data[0] == 0x8765431) {
                            data_even = 0x8765431;
                            data_odd  = 0xfedcba9;
                        } else {
                            data_odd  = 0x8765431;
                            data_even = 0xfedcba9;
                        }
                    } else {
                        PRINTF("ERROR sample %d (B%d) = 0x%08x != 0x8765431 or 0xfedcba9\r\n\r", i, batch, data[0]);
                        success = false;
                    }

                } else {
                    //check whether even or odd
                    if (i & 0x1) {
                        //odd
                        if (data[i] != data_odd) {
                                PRINTF("ERROR left sample %d (B%d) = 0x%08x != 0x%08x\r\n\r", i, batch, data[i], data_odd);
                                success = false;
                        }
                    }
                    else {
                        //even
                        if (data[i] != data_even) {
                                PRINTF("ERROR left sample %d (B%d) = 0x%08x != 0x%08x\r\n\r", i, batch, data[i], data_even);
                                success = false;
                        }
                    }
                }
             }
        }
    }

    if (! mic_connected) {
        PRINTF("WARNING: Microphone not connected!\r\n\r");
    }
#endif

    i2s_terminate();

    if( success ){
        PRINTF("Success. \n\r");
        return EXIT_SUCCESS;
    }else{
        PRINTF("Failure. \n\r");
        return EXIT_FAILURE;
    }
}

