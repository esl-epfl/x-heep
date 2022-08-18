// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "soc_ctrl.h"
#include "spi.h"


int main(int argc, char *argv[])
{
    spi_host_t spi_host;
    spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);


    // Select SPI host as SPI output
    soc_ctrl_select_spi_host(&soc_ctrl);

    spi_set_enable(&spi_host, true);

    // Configure chip 0 (Max 50 MHz core SPI clock --> Max 25 MHz SCK) and select it.
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

    // Set RX watermark to 1 word
    spi_set_rx_watermark(&spi_host, 1);

    volatile uint32_t flash_data = 0x049237ab;
    uint32_t* data_dst = &flash_data;

    // Power up flash
    const uint8_t powerup_byte_cmd = 0xab;
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

    const uint8_t read_byte_cmd = 0x03;
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
    // // Load command FIFO with address (3 Bytes at single speed)
    // const uint32_t cmd_addr = spi_create_command((spi_command_t){
    //     .len        = 2,
    //     .csaat      = true,
    //     .speed      = kSpiSpeedStandard,
    //     .direction  = kSpiDirTxOnly
    // });
    // spi_set_command(&spi_host, cmd_addr);
    // spi_wait_for_ready(&spi_host);
    // Load command FIFO with read cycles
    const uint32_t cmd_read_rx = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirRxOnly
    });
    spi_set_command(&spi_host, cmd_read_rx);
    spi_wait_for_ready(&spi_host);
    // Wait transaction is finished
    spi_wait_for_rx_watermark(&spi_host);
    // Read data from SPI RX FIFO
    spi_read_word(&spi_host, data_dst);

    // // Fill TX FIFO with TX data (Regular old read command + 3B address)
    // printf("Write %x to flash.\n", 0x049237ab);
    // const uint8_t cmd = 0x03;
    // spi_write_word(&spi, ((0 & 0xFF) << 16) | (0 & 0xFF00) | cmd);
    // // Wait for readiness to process commands
    // spi_wait_for_ready(&spi_host);
    // // Load command FIFO with command (1 Byte at single speed)
    // const uint32_t cmd_cmd = spi_create_command((spi_command_t){
    //     .len        = 0,
    //     .csaat      = true,
    //     .speed      = kSpiSpeedStandard,
    //     .direction  = kSpiDirTxOnly
    // });
    // spi_set_command(&spi_host, cmd_cmd);
    // spi_wait_for_ready(&spi_host);
    // // Load command FIFO with address (3 Bytes at single speed)
    // const uint32_t cmd_addr = spi_create_command((spi_command_t){
    //     .len        = 2,
    //     .csaat      = true,
    //     .speed      = kSpiSpeedStandard,
    //     .direction  = kSpiDirTxOnly
    // });
    // spi_set_command(&spi_host, cmd_addr);
    // spi_wait_for_ready(&spi_host);
    // // Load command FIFO with read cycles (always 256 cycles)
    // const uint32_t cmd_read = spi_create_command((spi_command_t){
    //     .len        = 3,
    //     .csaat      = false,
    //     .speed      = kSpiSpeedStandard,
    //     .direction  = kSpiDirRxOnly
    // });
    // spi_set_command(&spi_host, cmd_read);
    // spi_wait_for_ready(&spi_host);
    // spi_wait_for_rx_watermark(&spi_host);
    // spi_read_word(&spi_host, data_dst);
    // // // Read in 32-byte chunks
    // // for (int k = 0; k < 256/32; ++k) {
    // //     spi_wait_for_rx_watermark(&spi_host);
    // //     spi_read_chunk_32B(&spi_host, code_dst + 8*k + 64*i);
    // // }

    printf("Read %x from flash.\n", *data_dst);

    /* write something to stdout */
    printf("hello world!\n");
    return EXIT_SUCCESS;
}
