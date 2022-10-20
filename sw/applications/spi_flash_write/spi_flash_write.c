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
#include "fast_intr_ctrl.h"
#include "fast_intr_ctrl_regs.h"

// Un-comment this line to use the SPI FLASH instead of the default SPI
// #define USE_SPI_FLASH

#define COPY_DATA_BYTES 256 // Flash page size = 256
#define SPI_BYTES (4 * (uint32_t)((COPY_DATA_BYTES-1) / 4 + 1)) // Only sends data when an entire word has been received

#define FLASH_ADDR 0x00008500 // 256B data alignment

int8_t spi_intr_flag;
spi_host_t spi_host;

#ifndef USE_SPI_FLASH
void handler_irq_fast_spi(void)
{
    // Disable SPI interrupts
    spi_enable_evt_intr(&spi_host, false);
    spi_enable_rxwm_intr(&spi_host, false);
    // Clear fast interrupt
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kSpi_e);
    spi_intr_flag = 1;
}
#else
void handler_irq_fast_spi_flash(void)
{
    // Disable SPI interrupts
    spi_enable_evt_intr(&spi_host, false);
    spi_enable_rxwm_intr(&spi_host, false);
    // Clear fast interrupt
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kSpiFlash);
    spi_intr_flag = 1;
}
#endif

// Reserve memory array
uint32_t flash_data[SPI_BYTES / 4] __attribute__ ((aligned (4))) = {0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,0x679852fe,0xff8252bb,0x763b4521,0x6875adaa,0x09ac65bb,0x666ba334,0x44556677,0x0000ba98};

int main(int argc, char *argv[])
{
    #ifndef USE_SPI_FLASH
        spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_START_ADDRESS);
    #else
        spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    #endif

    // dma peripheral structure to access the registers
    dma_t dma;
    dma.base_addr = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level fast spi interrupt
    #ifndef USE_SPI_FLASH
        const uint32_t mask = 1 << 20;
    #else
        const uint32_t mask = 1 << 21;
    #endif
    CSR_SET_BITS(CSR_REG_MIE, mask);
    spi_intr_flag = 0;

    // Select SPI host as SPI output
    soc_ctrl_select_spi_host(&soc_ctrl);
    // Enable SPI host device
    spi_set_enable(&spi_host, true);

    // SPI and SPI_FLASH are the same IP so same register map
    uint32_t *fifo_ptr = spi_host.base_addr.base + SPI_HOST_DATA_REG_OFFSET;

    // -- SPI CONFIGURATION -- 
    // Configure chip 0 (flash memory)
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

    // The actual implementation of the SPI HOST has HW bugs when sending single bytes per transaction,
    // This dummy read and write commands are used to send empty commands to the devide to fush and discard internal TX and RX words
    const uint32_t cmd_dummy_write = spi_create_command((spi_command_t){
        .len        = 2,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });

    const uint32_t cmd_dummy_read = spi_create_command((spi_command_t){
        .len        = 2,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirRxOnly
    });

    /////////////// WRITE ////////////////

    // Reset
    const uint32_t reset_cmd = 0xFFFFFFFF;
    spi_write_word(&spi_host, reset_cmd);
    const uint32_t cmd_reset = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_reset);
    spi_wait_for_ready(&spi_host);
    spi_set_rx_watermark(&spi_host,1);

    // Power up flash
    const uint32_t powerup_byte_cmd = 0xab;
    spi_write_word(&spi_host, powerup_byte_cmd);
    const uint32_t cmd_powerup = spi_create_command((spi_command_t){
        .len        = 0,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_powerup);
    spi_wait_for_ready(&spi_host);
    spi_set_command(&spi_host, cmd_dummy_write);
    spi_wait_for_ready(&spi_host);

    // Write enable
    const uint32_t write_enable_cmd = 0x06;
    spi_write_word(&spi_host, write_enable_cmd);
    const uint32_t cmd_write_en = spi_create_command((spi_command_t){
        .len        = 0,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_write_en);
    spi_wait_for_ready(&spi_host);
    spi_set_command(&spi_host, cmd_dummy_write);
    spi_wait_for_ready(&spi_host);

    // Write command
    const uint32_t write_byte_cmd = ((FLASH_ADDR << 8) | 0x02); // Program Page + addr
    spi_write_word(&spi_host, write_byte_cmd);
    const uint32_t cmd_write = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = true,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_write);
    spi_wait_for_ready(&spi_host); 

    // -- DMA CONFIGURATION --
    dma_set_read_ptr_inc(&dma, (uint32_t) 4); // Do not increment address when reading from the SPI (Pop from FIFO)
    dma_set_write_ptr_inc(&dma, (uint32_t) 0); // Do not increment address when reading from the SPI (Pop from FIFO)
    dma_set_read_ptr(&dma, (uint32_t) flash_data); // SPI RX FIFO addr
    dma_set_write_ptr(&dma, (uint32_t) fifo_ptr); // copy data address
    // Set the correct SPI-DMA mode:
    // (0) disable
    // (1) receive from SPI (use SPI_START_ADDRESS for spi_host pointer)
    // (2) send to SPI (use SPI_START_ADDRESS for spi_host pointer)
    // (3) receive from SPI FLASH (use SPI_FLASH_START_ADDRESS for spi_host pointer)
    // (4) send to SPI FLASH (use SPI_FLASH_START_ADDRESS for spi_host pointer)
    #ifndef USE_SPI_FLASH
        dma_set_spi_mode(&dma, (uint32_t) 2); // The DMA will wait for the SPI TX FIFO ready signal
    #else
        dma_set_spi_mode(&dma, (uint32_t) 4); // The DMA will wait for the SPI FLASH TX FIFO ready signal
    #endif
    dma_set_cnt_start(&dma, (uint32_t) COPY_DATA_BYTES); // Size of data received by SPI

    // Wait for the first data to arrive to the TX FIFO before enabling interrupt
    spi_wait_for_tx_not_empty(&spi_host);
    // Enable event interrupt
    spi_enable_evt_intr(&spi_host, true);
    // Enable TX empty interrupt
    spi_enable_txempty_intr(&spi_host, true);

    const uint32_t cmd_write_tx = spi_create_command((spi_command_t){
        .len        = SPI_BYTES - 1,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_write_tx);
    spi_wait_for_ready(&spi_host); 

    // Wait for SPI interrupt
    printf("Waiting for the SPI interrupt...\n");
    while(spi_intr_flag == 0) {
        wait_for_interrupt();
    }
    printf("triggered!\n");

    // Check status register status waiting for ready
    bool flash_busy = true;
    uint8_t flash_resp[4] = {0xff,0xff,0xff,0xff};
    while(flash_busy){
        uint32_t flash_cmd = 0x00000005; // [CMD] Read status register
        spi_write_word(&spi_host, flash_cmd); // Push TX buffer
        uint32_t spi_bi_cmd = spi_create_command((spi_command_t){
            .len        = 1,
            .csaat      = false,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirBidir
        });
        spi_set_command(&spi_host, spi_bi_cmd);
        spi_wait_for_ready(&spi_host);
        spi_wait_for_rx_watermark(&spi_host);
        spi_read_word(&spi_host, &flash_resp[0]);
        if ((flash_resp[0] & 0x01) == 0) flash_busy = false;
    }

    // Fix bug with the previous Bidirectional additional byte
    spi_set_command(&spi_host, cmd_dummy_read);
    spi_wait_for_ready(&spi_host);
    spi_wait_for_rx_watermark(&spi_host);
    spi_read_word(&spi_host, &flash_resp[0]);

    // Power down flash
    const uint32_t powerdown_byte_cmd = 0xb9;
    spi_write_word(&spi_host, powerdown_byte_cmd);
    const uint32_t cmd_powerdown = spi_create_command((spi_command_t){
        .len        = 0,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_powerdown);
    spi_wait_for_ready(&spi_host);
    spi_set_command(&spi_host, cmd_dummy_write);
    spi_wait_for_ready(&spi_host);

    printf("%d Bytes written in Flash at @0x%08x \n", COPY_DATA_BYTES, FLASH_ADDR);

    return EXIT_SUCCESS;
}
