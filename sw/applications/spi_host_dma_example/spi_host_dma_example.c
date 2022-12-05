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
#include "dma.h"
#include "fast_intr_ctrl.h"
#include "fast_intr_ctrl_regs.h"

// Un-comment this line to use the SPI FLASH instead of the default SPI
// #define USE_SPI_FLASH

// Type of data frome the SPI. For types different than words the SPI data is requested in separate transactions
// word(0), half-word(1), byte(2,3)
#define SPI_DATA_TYPE 0

// Number of elements to copy
#define COPY_DATA_NUM 16

#define FLASH_CLK_MAX_HZ (133*1000*1000) // In Hz (133 MHz for the flash w25q128jvsim used in the EPFL Programmer)

#define REVERT_24b_ADDR(addr) ((((uint32_t)(addr) & 0xff0000) >> 16) | ((uint32_t)(addr) & 0xff00) | (((uint32_t)(addr) & 0xff) << 16))

int8_t dma_intr_flag;
spi_host_t spi_host;

void handler_irq_fast_dma(void)
{
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kDma_fic_e);
    dma_intr_flag = 1;
}

// Reserve memory array
#if SPI_DATA_TYPE == 0
    uint32_t flash_data[COPY_DATA_NUM] __attribute__ ((aligned (4))) = {0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,0x679852fe,0xff8252bb,0x763b4521,0x6875adaa,0x09ac65bb,0x666ba334,0x44556677,0x0000ba98};
    uint32_t copy_data[COPY_DATA_NUM] __attribute__ ((aligned (4)))  = { 0 };
#elif SPI_DATA_TYPE == 1
    uint16_t flash_data[COPY_DATA_NUM] __attribute__ ((aligned (2))) = {0x7654,0xfedc,0x579a,0x657d,0x758e,0x0123,0xfedb,0x89ab,0x6798,0xff82,0x763b,0x6875,0x09ac,0x666b,0x4455,0x0000};
    uint16_t copy_data[COPY_DATA_NUM] __attribute__ ((aligned (2)))  = { 0 };
#else
    uint8_t flash_data[COPY_DATA_NUM] = {0x76,0xfe,0x57,0x65,0x75,0x01,0xfe,0x89,0x67,0xff,0x76,0x68,0x09,0x66,0x44,0x00};
    uint8_t copy_data[COPY_DATA_NUM] = { 0 };
#endif

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
    uint32_t core_clk = soc_ctrl_get_frequency(&soc_ctrl);

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level fast dma interrupt
    const uint32_t mask = 1 << 19;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    #ifdef USE_SPI_FLASH
        // Select SPI host as SPI output
        soc_ctrl_select_spi_host(&soc_ctrl);
    #endif

    // Enable SPI host device
    spi_set_enable(&spi_host, true);
    // Enable SPI output
    spi_output_enable(&spi_host, true);

    // SPI and SPI_FLASH are the same IP so same register map
    uint32_t *fifo_ptr_rx = spi_host.base_addr.base + SPI_HOST_RXDATA_REG_OFFSET;

    // -- DMA CONFIGURATION --
    dma_set_read_ptr_inc(&dma, (uint32_t) 0); // Do not increment address when reading from the SPI (Pop from FIFO)
    #if SPI_DATA_TYPE == 0
        dma_set_write_ptr_inc(&dma, (uint32_t) 4); // Do not increment address when reading from the SPI (Pop from FIFO)
    #elif SPI_DATA_TYPE == 1
        dma_set_write_ptr_inc(&dma, (uint32_t) 2); // Do not increment address when reading from the SPI (Pop from FIFO)
    #else
        dma_set_write_ptr_inc(&dma, (uint32_t) 1); // Do not increment address when reading from the SPI (Pop from FIFO)
    #endif
    dma_set_read_ptr(&dma, (uint32_t) fifo_ptr_rx); // SPI RX FIFO addr
    dma_set_write_ptr(&dma, (uint32_t) copy_data); // copy data address
    // Set the correct SPI-DMA mode:
    // (0) disable
    // (1) receive from SPI (use SPI_START_ADDRESS for spi_host pointer)
    // (2) send to SPI (use SPI_START_ADDRESS for spi_host pointer)
    // (3) receive from SPI FLASH (use SPI_FLASH_START_ADDRESS for spi_host pointer)
    // (4) send to SPI FLASH (use SPI_FLASH_START_ADDRESS for spi_host pointer)
    #ifndef USE_SPI_FLASH
        dma_set_spi_mode(&dma, (uint32_t) 1); // The DMA will wait for the SPI RX FIFO valid signal
    #else
        dma_set_spi_mode(&dma, (uint32_t) 3); // The DMA will wait for the SPI FLASH RX FIFO valid signal
    #endif
    dma_set_data_type(&dma, (uint32_t) SPI_DATA_TYPE);

    // Configure SPI clock
    // SPI clk freq = 1/2 core clk freq when clk_div = 0
    // SPI_CLK = CORE_CLK/(2 + 2 * CLK_DIV) <= CLK_MAX => CLK_DIV > (CORE_CLK/CLK_MAX - 2)/2
    uint16_t clk_div = 0;
    if(FLASH_CLK_MAX_HZ < core_clk/2){
        clk_div = (core_clk/(FLASH_CLK_MAX_HZ) - 2)/2; // The value is truncated
        if (core_clk/(2 + 2 * clk_div) > FLASH_CLK_MAX_HZ) clk_div += 1; // Adjust if the truncation was not 0
    }
    // SPI Configuration
    // Configure chip 0 (flash memory)
    const uint32_t chip_cfg = spi_create_configopts((spi_configopts_t){
        .clkdiv     = clk_div,
        .csnidle    = 0xF,
        .csntrail   = 0xF,
        .csnlead    = 0xF,
        .fullcyc    = false,
        .cpha       = 0,
        .cpol       = 0
    });
    spi_set_configopts(&spi_host, 0, chip_cfg);
    spi_set_csid(&spi_host, 0);

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

    // Load command FIFO with read command (1 Byte at single speed)
    const uint32_t cmd_read = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = true,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });

    uint32_t read_byte_cmd;
    read_byte_cmd = ((REVERT_24b_ADDR(flash_data) << 8) | 0x03); // The address bytes sent through the SPI to the Flash are in reverse order

    dma_intr_flag = 0;
    dma_set_cnt_start(&dma, (uint32_t) (COPY_DATA_NUM*sizeof(*copy_data)));

    #if SPI_DATA_TYPE == 0
        const uint32_t cmd_read_rx = spi_create_command((spi_command_t){ // Single transaction
            .len        = COPY_DATA_NUM*sizeof(*copy_data) - 1, // In bytes - 1
            .csaat      = false,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirRxOnly
        });
        spi_write_word(&spi_host, read_byte_cmd); // Fill TX FIFO with TX data (read command + 3B address)
        spi_wait_for_ready(&spi_host); // Wait for readiness to process commands
        spi_set_command(&spi_host, cmd_read); // Send read command to the external device through SPI
        spi_wait_for_ready(&spi_host);
        spi_set_command(&spi_host, cmd_read_rx); // Receive data in RX
        spi_wait_for_ready(&spi_host);
    #else
        const uint32_t cmd_read_rx = spi_create_command((spi_command_t){ // Multiple transactions of the data type
            .len        = (sizeof(*copy_data) - 1),
            .csaat      = false,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirRxOnly
        });
        for (int i = 0; i<COPY_DATA_NUM; i++) { // Multiple 16-bit transactions
            // Request the same data multiple times
            spi_write_word(&spi_host, read_byte_cmd); // Fill TX FIFO with TX data (read command + 3B address)
            spi_wait_for_ready(&spi_host); // Wait for readiness to process commands
            spi_set_command(&spi_host, cmd_read); // Send read command to the external device through SPI
            spi_wait_for_ready(&spi_host); 
            spi_set_command(&spi_host, cmd_read_rx); // Receive data in RX
            spi_wait_for_ready(&spi_host);
        }
    #endif

    // Wait for DMA interrupt
    printf("Waiting for the DMA interrupt...\n");
    while(dma_intr_flag == 0) {
        wait_for_interrupt();
    }
    printf("triggered!\n");

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

    // The data is already in memory -- Check results
    printf("flash vs ram...\n");

    uint32_t errors = 0;
    uint32_t count = 0;
    #if SPI_DATA_TYPE == 0
        for (int i = 0; i<COPY_DATA_NUM; i++) {
            if(flash_data[i] != copy_data[i]) {
                printf("@%08x-@%08x : %02x != %02x\n" , &flash_data[i] , &copy_data[i], flash_data[i], copy_data[i]);
                errors++;
            }
            count++;
        }
    #else
        for (int i = 0; i<COPY_DATA_NUM; i++) {
            if(flash_data[0] != copy_data[i]) {
                printf("@%08x-@%08x : %02x != %02x\n" , &flash_data[0] , &copy_data[i], flash_data[0], copy_data[i]);
                errors++;
            }
            count++;
        }
    #endif

    if (errors == 0) {
        printf("success! (bytes checked: %d)\n", count*sizeof(*copy_data));
    } else {
        printf("failure, %d errors! (Out of %d)\n", errors, count);
    }
    return EXIT_SUCCESS;
}
