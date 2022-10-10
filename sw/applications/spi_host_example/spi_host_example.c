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
#include "soc_ctrl.h"
#include "spi_host.h"
#include "fast_intr_ctrl.h"
#include "fast_intr_ctrl_regs.h"

// Simple example to check the SPI host peripheral is working. It checks the ram and flash have the same content
#define DATA_CHUNK_ADDR 0x00008000

int8_t spi_intr_flag;
spi_host_t spi_host;

void handler_irq_fast_spi(void)
{
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kSpi_e);

    spi_intr_flag = 1;
}

uint32_t flash_data[8];

int main(int argc, char *argv[])
{
    // spi_host_t spi_host;
    spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level fast spi interrupt
    const uint32_t mask = 1 << 20;
    CSR_SET_BITS(CSR_REG_MIE, mask);
    spi_intr_flag = 0;

    // Select SPI host as SPI output
    soc_ctrl_select_spi_host(&soc_ctrl);
    // Enable SPI host device
    spi_set_enable(&spi_host, true);

    // Enable event interrupt
    spi_enable_evt_intr(&spi_host, true);
    // Enable RX watermark interrupt
    spi_enable_rxwm_intr(&spi_host, true);

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
    spi_set_rx_watermark(&spi_host, 8);

    uint32_t *flash_data_ptr = flash_data[0];

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

    volatile uint32_t data_addr = DATA_CHUNK_ADDR;

    const uint32_t read_byte_cmd = ((DATA_CHUNK_ADDR << 8) | 0x03);

    // Fill TX FIFO with TX data (read command + 3B address)
    spi_write_word(&spi_host, read_byte_cmd);
    // Wait for readiness to process commands
    spi_wait_for_ready(&spi_host);

    ////////////////////////////////////////////////////////////////

    // // Load command FIFO with read command (1 Byte at single speed)
    // const uint32_t cmd_read = spi_create_command((spi_command_t){
    //     .len        = 0,
    //     .csaat      = true,
    //     .speed      = kSpiSpeedStandard,
    //     .direction  = kSpiDirTxOnly
    // });
    // spi_set_command(&spi_host, cmd_read);
    // spi_wait_for_ready(&spi_host);
    // // Load command FIFO with read address (3 Byte at single speed)
    // const uint32_t cmd_addr = spi_create_command((spi_command_t){
    //     .len        = 2,
    //     .csaat      = true,
    //     .speed      = kSpiSpeedStandard,
    //     .direction  = kSpiDirTxOnly
    // });
    // spi_set_command(&spi_host, cmd_addr);
    // spi_wait_for_ready(&spi_host);

    // OR

    // Load command FIFO with read command (1 Byte at single speed)
    const uint32_t cmd_read = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = true,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_read);
    spi_wait_for_ready(&spi_host);

    ////////////////////////////////////////////////////////////////

    const uint32_t cmd_read_rx = spi_create_command((spi_command_t){
        .len        = 31,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirRxOnly
    });
    spi_set_command(&spi_host, cmd_read_rx);
    spi_wait_for_ready(&spi_host);

    // Wait transaction is finished (polling register)
    // spi_wait_for_rx_watermark(&spi_host);
    // or wait for SPI interrupt
    while(spi_intr_flag==0) {
        wait_for_interrupt();
    }

    // Read data from SPI RX FIFO
    for (int i=0; i<8; i++) {
        spi_read_word(&spi_host, &flash_data[i]);
    }

    printf("flash vs ram...\n");

    uint32_t errors = 0;
    uint32_t* ram_ptr = DATA_CHUNK_ADDR;
    for (int i=0; i<8; i++) {
        if(flash_data[i] != *ram_ptr) {
            printf("@%x : %x != %x\n", DATA_CHUNK_ADDR+i*4, flash_data[i], *ram_ptr);
            errors++;
        }
        ram_ptr++;
    }

    if (errors == 0) {
        printf("success!\n");
    } else {
        printf("failure, %d errors!\n", errors);
    }
    return EXIT_SUCCESS;
}
