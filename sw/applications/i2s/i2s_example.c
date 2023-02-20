/*
 * Copyright 2020 ETH Zurich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Robert Balas <balasr@iis.ee.ethz.ch>
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

int8_t external_intr_flag;

// Interrupt controller variables
dif_plic_params_t rv_plic_params;
dif_plic_t rv_plic;
dif_plic_result_t plic_res;
dif_plic_irq_id_t intr_num;


#define AUDIO_DATA_NUM 2048
uint32_t audio_data[AUDIO_DATA_NUM] __attribute__ ((aligned (4)))  = { 0 };


void handler_irq_external(void) {
    // Claim/clear interrupt
    plic_res = dif_plic_irq_claim(&rv_plic, 0, &intr_num);
    if (plic_res == kDifPlicOk) {
        if (intr_num == I2S_INTR_EVENT) {
            external_intr_flag = external_intr_flag + 1;
        }
        dif_plic_irq_complete(&rv_plic, 0, &intr_num);
    }
}

int8_t dma_intr_flag;

void handler_irq_fast_dma(void)
{
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kDma_fic_e);
    dma_intr_flag = 1;
}

i2s_t i2s;

int main(int argc, char *argv[])
{
    printf("I2s DEMO\n");

    i2s.base_addr = mmio_region_from_addr((uintptr_t)I2S_START_ADDRESS);


    // dma peripheral structure to access the registers
    dma_t dma;
    dma.base_addr = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);

    uint32_t *fifo_ptr_rx = i2s.base_addr.base + I2S_RXDATA_REG_OFFSET;


     // -- DMA CONFIGURATION --
    dma_set_read_ptr_inc(&dma, (uint32_t) 0); // Do not increment address when reading from the SPI (Pop from FIFO)
    dma_set_write_ptr_inc(&dma, (uint32_t) 4);
    dma_set_read_ptr(&dma, (uint32_t) fifo_ptr_rx); // SPI RX FIFO addr
    dma_set_write_ptr(&dma, (uint32_t) audio_data); // audio data address
    dma_set_spi_mode(&dma, DMA_SPI_MODE_SPI_MODE_VALUE_DMA_I2S_RX); // The DMA will wait for the SPI RX FIFO valid signal
    dma_set_data_type(&dma, (uint32_t) 0);


    printf("Init PLIC..");
    rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)RV_PLIC_START_ADDRESS);
    plic_res = dif_plic_init(rv_plic_params, &rv_plic);

    if (plic_res == kDifPlicOk) {
        printf("1\n");
    } else {
        printf("0\n;");
    }

    printf("Set I2s intr prio=1..");
    // Set memcopy priority to 1 (target threshold is by default 0) to trigger an interrupt to the target (the processor)
    plic_res = dif_plic_irq_set_priority(&rv_plic, I2S_INTR_EVENT, 1);
    if (plic_res == kDifPlicOk) {
        printf("1\n");
    } else {
        printf("0\n;");
    }

    printf("EN I2s intr..");
    plic_res = dif_plic_irq_set_enabled(&rv_plic, I2S_INTR_EVENT, 0, kDifPlicToggleEnabled);
    if (plic_res == kDifPlicOk) {
        printf("1\n");
    } else {
        printf("0\n;");
    }

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    // bit 11 = external intr (used for i2s)
    // bit 19 = dma fast intr
    const uint32_t mask = 1 << 11 | 1 << 19; 
    CSR_SET_BITS(CSR_REG_MIE, mask);
    external_intr_flag = 0;



    dma_intr_flag = 0;
    dma_set_cnt_start(&dma, (uint32_t) (AUDIO_DATA_NUM*sizeof(*audio_data)));



    // enable I2s interrupt
    i2s_set_enable_intr(&i2s, 1);

    const u_int16_t batchsize = 0x20;



    i2s_set_clk_divider(&i2s, 0x10);
    i2s_set_intr_reach_count(&i2s, batchsize);
    i2s_set_data_width(&i2s, I2S_BYTEPERSAMPLE_COUNT_VALUE_32_BITS);
    i2s_set_enable(&i2s, 1, 1);

  
    printf("started hw");

    int8_t batch = 0;

    u_int8_t test_baches = 4;
    for (u_int8_t batch = 0; batch < test_baches; batch++) {
        while(external_intr_flag == batch) {
            printf(".");
        }

        uint32_t errors = 0;
        for (int i = 0; i < batchsize; i++) {
            if (audio_data[batch * batchsize + i] != batch * batchsize + i + 1) errors = errors + 1; 
        }
        printf("%x: errors %d\n", batch, errors);
    }


    return EXIT_SUCCESS;
}