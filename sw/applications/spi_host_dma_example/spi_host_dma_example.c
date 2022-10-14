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
#define USE_SPI_FLASH

#define COPY_DATA_BYTES 256 // Flash page size = 256
#define SPI_BYTES (4 * (uint32_t)((COPY_DATA_BYTES-1) / 4 + 1)) // Only sends data when an entire word has been received

#define FLASH_ADDR 0x00008500

int8_t spi_intr_flag;
int8_t dma_intr_flag;
spi_host_t spi_host;

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

void handler_irq_fast_spi_flash(void)
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

void handler_irq_fast_dma(void)
{
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kDma_e);
    dma_intr_flag = 1;
}

// Reserve memory array
uint32_t flash_data[SPI_BYTES / 4] __attribute__ ((aligned (4))) = {0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,0x679852fe,0xff8252bb,0x763b4521,0x6875adaa,0x09ac65bb,0x666ba334,0x44556677,0x0000ba98};
uint32_t copy_data[SPI_BYTES / 4] __attribute__ ((aligned (4)))  = { 0 };

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
    // Set mie.MEIE bit to one to enable machine-level fast dma interrupt
    const uint32_t mask = 1 << 19;
    CSR_SET_BITS(CSR_REG_MIE, mask);
    spi_intr_flag = 0;

    // Select SPI host as SPI output
    soc_ctrl_select_spi_host(&soc_ctrl);
    // Enable SPI host device
    spi_set_enable(&spi_host, true);

    // SPI and SPI_HOST are the same IP so same register map
    uint32_t *fifo_ptr = spi_host.base_addr.base + SPI_HOST_DATA_REG_OFFSET;

    // -- SPI CONFIGURATION -- 
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

    /////////////// WRITE ////////////////

    const uint32_t cmd_dummy = spi_create_command((spi_command_t){
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

    printf("Reset...\n");

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

    printf("power up...\n");

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
    printf("Wait...\n");
    spi_set_command(&spi_host, cmd_dummy);
    spi_wait_for_ready(&spi_host);

    printf("Write enable...\n");

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
    printf("Wait...\n");
    spi_set_command(&spi_host, cmd_dummy);
    spi_wait_for_ready(&spi_host);

    printf("Write cmd...\n");

    // // Erase memory
    // const uint32_t write_byte_cmd = 0xD8; // ((FLASH_ADDR << 8) | 0x02);
    // spi_write_word(&spi_host, write_byte_cmd);
    // const uint32_t cmd_write = spi_create_command((spi_command_t){
    //     .len        = 3,
    //     .csaat      = false,
    //     .speed      = kSpiSpeedStandard,
    //     .direction  = kSpiDirTxOnly
    // });
    // spi_set_command(&spi_host, cmd_write);
    // spi_wait_for_ready(&spi_host); 

    // --

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
        dma_set_spi_mode(&dma, (uint32_t) 2); // The DMA will wait for the SPI tx FIFO ready signal
    #else
        dma_set_spi_mode(&dma, (uint32_t) 4); // The DMA will wait for the SPI tx FIFO ready signal
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

    // Fix bug with Bidirectional additional byte
    spi_set_command(&spi_host, cmd_dummy_read);
    spi_wait_for_ready(&spi_host);
    spi_wait_for_rx_watermark(&spi_host);
    spi_read_word(&spi_host, &flash_resp[0]);

    printf("Reading...\n");

    /////////////// READ /////////////////

    // The address bytes sent through the SPI to the Flash are in reverse order
    // Add read command in the first byte sent
    uint32_t read_byte_cmd = ((FLASH_ADDR << 8) | 0x03);

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
        .len        = SPI_BYTES - 1,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirRxOnly
    });
    spi_set_command(&spi_host, cmd_read_rx);
    spi_wait_for_ready(&spi_host);

    // -- DMA CONFIGURATION --
    dma_set_read_ptr_inc(&dma, (uint32_t) 0); // Do not increment address when reading from the SPI (Pop from FIFO)
    dma_set_write_ptr_inc(&dma, (uint32_t) 4); // Do not increment address when reading from the SPI (Pop from FIFO)
    dma_set_read_ptr(&dma, (uint32_t) fifo_ptr); // SPI RX FIFO addr
    dma_set_write_ptr(&dma, (uint32_t) copy_data); // copy data address
    // Set the correct SPI-DMA mode:
    // (0) disable
    // (1) receive from SPI (use SPI_START_ADDRESS for spi_host pointer)
    // (2) send to SPI (use SPI_START_ADDRESS for spi_host pointer)
    // (3) receive from SPI FLASH (use SPI_FLASH_START_ADDRESS for spi_host pointer)
    // (4) send to SPI FLASH (use SPI_FLASH_START_ADDRESS for spi_host pointer)
    #ifndef USE_SPI_FLASH
        dma_set_spi_mode(&dma, (uint32_t) 1); // The DMA will wait for the SPI rx FIFO valid signal
    #else
        dma_set_spi_mode(&dma, (uint32_t) 3); // The DMA will wait for the SPI rx FIFO valid signal
    #endif
    dma_intr_flag = 0;
    dma_set_cnt_start(&dma, (uint32_t) COPY_DATA_BYTES); // Size of data received by SPI

    // Wait for DMA interrupt
    printf("Waiting for the DMA interrupt...\n");
    while(dma_intr_flag == 0) {
        wait_for_interrupt();
    }
    printf("triggered!\n");

    printf("Power down flash...\n");
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
    printf("Wait...\n");
    spi_set_command(&spi_host, cmd_dummy);
    spi_wait_for_ready(&spi_host);

    // The data is already in memory -- Check results
    printf("flash vs ram...\n");

    int i;
    uint32_t errors = 0;
    uint32_t count = 0;
    uint8_t *flash_data_8b = (uint8_t *)flash_data;
    uint8_t *copy_data_8b = (uint8_t *)copy_data;
    for (i = 0; i<COPY_DATA_BYTES; i++) {
        if(flash_data_8b[i] != copy_data_8b[i]) {
            printf("@%08x-@%08x : %02x != %02x\n" , &flash_data_8b[i] , &copy_data_8b[i], flash_data_8b[i], copy_data_8b[i]);
            errors++;
        }
        count++;
    }
    // Check that the rest last bytes of the word have not been overwritten
    while(i < SPI_BYTES){
        if(copy_data_8b[i] != 0) {
            printf("Data Overwritten @%08x : %02x != 0\n" , &copy_data_8b[i], copy_data_8b[i]);
            errors++;
        }
        count++;
        i++;
    }

    if (errors == 0) {
        printf("success! (Bytes checked: %d)\n", count);
    } else {
        printf("failure, %d errors! (Out of %d)\n", errors, count);
    }
    return EXIT_SUCCESS;
}
