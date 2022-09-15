// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "soc_ctrl.h"
#include "spi_host.h"
#include "dma.h"

#define COPY_DATA_SIZE 1024*2

// Simple example to check the SPI host peripheral is working. It checks the ram and flash have the same content
#define DATA_CHUNK_ADDR 0x00000200

// int8_t spi_intr_flag;
int8_t dma_intr_flag;

// Interrupt controller variables
dif_plic_params_t rv_plic_params;
dif_plic_t rv_plic;
dif_plic_result_t plic_res;
dif_plic_irq_id_t intr_num;

spi_host_t spi_host;

void handler_irq_external(void) {
    // Claim/clear interrupt
    plic_res = dif_plic_irq_claim(&rv_plic, 0, &intr_num);
    // DMA Interrupt instead of SPI
    // if (plic_res == kDifPlicOk && intr_num == SPI_INTR_EVENT) {
    //     spi_intr_flag = 1;
    //     // Disable SPI interrupt otherwise it stays on until RX FIFO is read
    //     dif_plic_irq_set_enabled(&rv_plic, SPI_INTR_EVENT, 0, kDifPlicToggleDisabled);
    // }
    if (plic_res == kDifPlicOk && intr_num == DMA_INTR_DONE) {
        dma_intr_flag = 1;
    }
}

// Reserve 16kB
uint32_t flash_data[COPY_DATA_SIZE];

int main(int argc, char *argv[])
{
    // spi_host_t spi_host;
    spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

    // Init the PLIC
    rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)PLIC_START_ADDRESS);
    plic_res = dif_plic_init(rv_plic_params, &rv_plic);
    if (plic_res != kDifPlicOk) {
        printf("Unable to set the PLIC\n;");
    }

    // Set SPI interrupt priority to 1
    // plic_res = dif_plic_irq_set_priority(&rv_plic, SPI_INTR_EVENT, 1);
    // Set dma priority to 1 (target threshold is by default 0) to trigger an interrupt to the target (the processor)
    plic_res = dif_plic_irq_set_priority(&rv_plic, DMA_INTR_DONE, 1);
    if (plic_res != kDifPlicOk) {
        printf("Unable to set the PLIC priority\n;");
    }

    // Enable SPI interrupt
    // plic_res = dif_plic_irq_set_enabled(&rv_plic, SPI_INTR_EVENT, 0, kDifPlicToggleEnabled);
    // Enable DMA interrupt
    plic_res = dif_plic_irq_set_enabled(&rv_plic, DMA_INTR_DONE, 0, kDifPlicToggleEnabled);
    if (plic_res != kDifPlicOk) {
        printf("Unable to enable the PLIC irq\n;");
    }

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 11;// ;
    CSR_SET_BITS(CSR_REG_MIE, mask);
    // spi_intr_flag = 0;
    dma_intr_flag = 0;

    // Select SPI host as SPI output
    soc_ctrl_select_spi_host(&soc_ctrl);
    // Enable SPI host device
    spi_set_enable(&spi_host, true);

    // Enable event interrupt
    // spi_enable_evt_intr(&spi_host, true);
    // Enable RX watermark interrupt
    // spi_enable_rxwm_intr(&spi_host, true); // Only needed in the SPI event interrupt generation (is a mask)

    uint32_t *fifo_ptr = spi_host.base_addr.base + SPI_HOST_DATA_REG_OFFSET;
    // uint32_t* data_dst = flash_data;

    // DMA CONFIGURATION --
    // dma peripheral structure to access the registers
    dma_t dma;
    dma.base_addr = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);
    dma_set_read_ptr_inc(&dma, (uint32_t) 0); // Do not increment address when reading from the SPI (Pop from FIFO)
    dma_set_spi_mode(&dma, (uint32_t) 1); // The DMA will wait for the watermark signal to start the transaction
    dma_set_read_ptr(&dma, (uint32_t) fifo_ptr); // SPI RX FIFO addr
    dma_set_write_ptr(&dma, (uint32_t) flash_data);
    dma_set_cnt_start(&dma, (uint32_t) COPY_DATA_SIZE); // Size of data received by SPI
    // ---------------------

    // Configure chip 0 (flash memory)
    // Max 50 MHz core SPI clock --> Max 25 MHz SCK
    // Single data IO; keep timing as safe as possible.
    const uint32_t chip_cfg = spi_create_configopts((spi_configopts_t){
        .clkdiv     = 1,
        .csnidle    = 0xF,
        .csntrail   = 0xF,
        .csnlead    = 0xF,
        .fullcyc    = false,
        .cpha       = 0,
        .cpol       = 0
    });
    spi_set_configopts(&spi_host, 0, chip_cfg);
    spi_set_csid(&spi_host, 0);

    // Set RX watermark to 8 word
    // spi_set_rx_watermark(&spi_host, 4);

    // Power up flash
    const uint32_t powerup_byte_cmd = 0xab;
    spi_write_word(&spi_host, powerup_byte_cmd);
    // Load command FIFO with command (1 Byte at single speed)
    const uint32_t cmd_powerup = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_powerup);
    spi_wait_for_ready(&spi_host);

    const uint32_t read_byte_cmd = ((DATA_CHUNK_ADDR << 8) | 0x03); //0x03;

    // Fill TX FIFO with TX data (read command + 3B address)
    spi_write_word(&spi_host, read_byte_cmd);
    // Wait for readiness to process commands
    spi_wait_for_ready(&spi_host);

    // Load command FIFO with read command (1 Byte at single speed)
    const uint32_t cmd_read = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = true,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_read);
    spi_wait_for_ready(&spi_host);

    const uint32_t cmd_read_rx = spi_create_command((spi_command_t){
        .len        = (uint16_t)(COPY_DATA_SIZE*4 - 1),
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirRxOnly
    });
    spi_set_command(&spi_host, cmd_read_rx);
    spi_wait_for_ready(&spi_host);

    ////////////////////////////////////////////////////////////////

    // Wait copy is done
    while(dma_intr_flag==0) {
        wait_for_interrupt();
    }


    // Wait transaction is finished (polling register) -- Can I check the watermark?
    // or wait for SPI interrupt
    // spi_wait_for_rx_watermark(&spi_host);
    // while(spi_intr_flag==0) {
    //     wait_for_interrupt();
    // }

    // Complete interrupt
    plic_res = dif_plic_irq_complete(&rv_plic, 0, &intr_num);
    if (plic_res != kDifPlicOk || intr_num != DMA_INTR_DONE) {
        printf("IRQ complete incorrect\n;");
    }

    // Read data from SPI RX FIFO - Not needed anymore, the data has already been copied by the DMA
    // spi_read_chunk_32B(&spi_host, flash_data);

    printf("flash vs ram...\n");

    uint32_t errors = 0;
    uint32_t count = 0;
    uint32_t* ram_ptr = DATA_CHUNK_ADDR;
    int i;
    for (i = 0; i<COPY_DATA_SIZE; i++) {
        if(flash_data[i] != *ram_ptr) {
            //printf("@%x : %x != %x\n", DATA_CHUNK_ADDR+i*4, flash_data[i], *ram_ptr);
            errors++;
        }
        count++;
        ram_ptr++;
    }

    if (errors == 0) {
        printf("success! (Words checked: %d)\n", count);
    } else {
        printf("failure, %d errors!\n", errors);
    }
    return EXIT_SUCCESS;
}
