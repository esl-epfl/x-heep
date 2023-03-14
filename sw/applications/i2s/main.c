/*
 * Copyright EPFL contributors.
 * Licensed under the Apache License, Version 2.0, see LICENSE for details.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Author: Tim Frey <tim.frey@epfl.ch>
 */

#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "i2s.h"

#include "mmio.h"
#include "handler.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "csr.h"
#include "hart.h"
#include "dma.h"
#include "dma_regs.h"
#include "fast_intr_ctrl.h"
#include "fast_intr_ctrl_regs.h"



// Interrupt controller variables
dif_plic_params_t rv_plic_params;
dif_plic_t rv_plic;
dif_plic_result_t plic_res;
dif_plic_irq_id_t intr_num;


// I2s
i2s_t i2s;
int8_t i2s_interrupt_flag;

#define I2S_TEST_BATCH_SIZE    8
#define I2S_TEST_BATCHES      16
#define I2S_CLK_DIV           256

#define AUDIO_DATA_NUM 32
uint32_t audio_data_0[AUDIO_DATA_NUM] __attribute__ ((aligned (4)))  = { 0 };
uint32_t audio_data_1[AUDIO_DATA_NUM] __attribute__ ((aligned (4)))  = { 0 };


// DMA
dma_t dma;
int8_t dma_intr_flag;
bool dma_buffer_id = 0;


//
// ISR
//
void handler_irq_external(void) {
    // Claim/clear interrupt
    plic_res = dif_plic_irq_claim(&rv_plic, 0, &intr_num);
    if (plic_res == kDifPlicOk) {
        if (intr_num == I2S_INTR_EVENT) {
            i2s_interrupt_flag = i2s_interrupt_flag + 1;
        }
        dif_plic_irq_complete(&rv_plic, 0, &intr_num);
    }
}

void handler_irq_fast_dma(void)
{
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kDma_fic_e);
    dma_intr_flag = 1;

    // dma peripheral structure to access the registers
    dma_t dma;
    dma.base_addr = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);

    dma_buffer_id = !dma_buffer_id;
    dma_set_write_ptr(&dma,  (dma_buffer_id ? (uint32_t) audio_data_1 : (uint32_t) audio_data_0) ); // audio data address
    dma_set_cnt_start(&dma, (uint32_t) (AUDIO_DATA_NUM*4));
}


//
// Setup
//
void setup() 
{
    i2s.base_addr = mmio_region_from_addr((uintptr_t)I2S_START_ADDRESS);
    dma.base_addr = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);
    rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)RV_PLIC_START_ADDRESS);

    const uint32_t *i2s_rx_fifo_ptr = i2s.base_addr.base + I2S_RXDATA_REG_OFFSET;


     // -- DMA CONFIGURATION --
    dma_set_read_ptr_inc(&dma, (uint32_t) 0); // Do not increment address when reading from the SPI (Pop from FIFO)
    dma_set_write_ptr_inc(&dma, (uint32_t) 4);
    dma_set_read_ptr(&dma, (uint32_t) i2s_rx_fifo_ptr); // I2s RX FIFO addr
    dma_set_write_ptr(&dma, (uint32_t) audio_data_0); // audio data address
    dma_set_rx_wait_mode(&dma, DMA_RX_WAIT_I2S); // The DMA will wait for the I2s RX FIFO valid signal
    dma_set_data_type(&dma, (uint32_t) 0);
    dma_set_cnt_start(&dma, (uint32_t) (AUDIO_DATA_NUM*4)); // start 


    // PLIC
    dif_plic_init(rv_plic_params, &rv_plic);
    plic_res = dif_plic_irq_set_priority(&rv_plic, I2S_INTR_EVENT, 1);
    plic_res = dif_plic_irq_set_enabled(&rv_plic, I2S_INTR_EVENT, 0, kDifPlicToggleEnabled);


    // enable I2s interrupt
    i2s_set_enable_intr(&i2s, 1);
    i2s_set_clk_divider(&i2s, I2S_CLK_DIV);
    i2s_set_intr_reach_count(&i2s, I2S_TEST_BATCH_SIZE);
    i2s_set_data_width(&i2s, I2S_BYTEPERSAMPLE_COUNT_VALUE_32_BITS);
    i2s_set_enable(&i2s, 1, 1);

    i2s_interrupt_flag = 0;


    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    // bit 11 = external intr (used for i2s)
    // bit 19 = dma fast intr
    const uint32_t mask = 1 << 11 | 1 << 19; 
    CSR_SET_BITS(CSR_REG_MIE, mask);
}


int main(int argc, char *argv[]) {
    printf("I2s DEMO\r\n");

    setup();

    printf("Setup done!\r\n");

    for (u_int8_t batch = 0; batch < I2S_TEST_BATCHES; batch++) {
        while(i2s_interrupt_flag == batch) {
            printf(".");
        }

        // uint32_t errors = 0;
        // for (int i = 0; i < batchsize; i++) {
        //     if (audio_data_0[batch * batchsize + i] != batch * batchsize + i + 1) errors = errors + 1; 
        // }
        printf("%x\r\n", batch);
    }

    return EXIT_SUCCESS;
}