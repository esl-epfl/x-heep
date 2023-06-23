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

#ifdef TARGET_PYNQ_Z2
    #define USE_SPI_FLASH
#endif

// Type of data frome the SPI. For types different than words the SPI data is requested in separate transactions
// word(0), half-word(1), byte(2,3)
#define SPI_DATA_TYPE DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_8BIT_WORD

// Number of elements to copy
#define COPY_DATA_NUM 16

#define FLASH_CLK_MAX_HZ (133*1000*1000) // In Hz (133 MHz for the flash w25q128jvsim used in the EPFL Programmer)

#define REVERT_24b_ADDR(addr) ((((uint32_t)(addr) & 0xff0000) >> 16) | ((uint32_t)(addr) & 0xff00) | (((uint32_t)(addr) & 0xff) << 16))

int8_t dma_intr_flag;
spi_host_t spi_host;

void dma_intr_handler_trans_done(void)
{
    printf("Non-weak implementation of a DMA interrupt\n");
    dma_intr_flag = 1;
}

// Reserve memory array
uint32_t flash_data[COPY_DATA_NUM] __attribute__ ((aligned (4))) = {0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,0x679852fe,0xff8252bb,0x763b4521,0x6875adaa,0x09ac65bb,0x666ba334,0x44556677,0x0000ba98};
uint32_t copy_data[COPY_DATA_NUM] __attribute__ ((aligned (4)))  = { 0 };

int main(int argc, char *argv[])
{
    #ifndef USE_SPI_FLASH
        spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    #else
        spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    #endif

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    uint32_t core_clk = soc_ctrl_get_frequency(&soc_ctrl);

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

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


    // DMA CONFIGURATION
    printf("---- TEST ---- \n");
    dma_init(NULL);


    #ifndef USE_SPI_FLASH
        uint8_t slot = DMA_TRIG_SLOT_SPI_RX;  // The DMA will wait for the SPI RX FIFO valid signal
    #else
        uint8_t slot = DMA_TRIG_SLOT_SPI_FLASH_RX; // The DMA will wait for the SPI FLASH RX FIFO valid signal
    #endif

    static dma_target_t tgt_src = {
        .inc_du = 0, 
        .size_du = COPY_DATA_NUM,
        .type = SPI_DATA_TYPE,
    };
    tgt_src.ptr = fifo_ptr_rx; // Necessary outside 'cause its not a const. 
    tgt_src.trig = slot;// Necessary outside 'cause its not a const. 

    static dma_target_t tgt_dst = {
        .inc_du = 1, 
        .size_du = COPY_DATA_NUM,
        .type = SPI_DATA_TYPE,
        .trig = DMA_TRIG_MEMORY,
    };
    tgt_dst.ptr = copy_data; // Necessary outside 'cause its not a const. 
    
    static dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .end = DMA_TRANS_END_INTR,
    };

    dma_config_flags_t res;

    res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    printf("Result - tgt trans: %u\n", res );
    res = dma_load_transaction(&trans);
    printf("Result - tgt load: %u\n", res );
  
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
    res = dma_launch(&trans);
    printf("launched!\n");

    #if SPI_DATA_TYPE == DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_32BIT_WORD
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
    if( trans.end == DMA_TRANS_END_POLLING ){
        while( ! dma_is_ready() ){};
    } else{
        printf("Waiting for the DMA interrupt...\n");
        while(dma_intr_flag == 0) {
            wait_for_interrupt();
        }
        printf("triggered!\n");
    }
    
    
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
    #if SPI_DATA_TYPE == DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_32BIT_WORD
        for (int i = 0; i<COPY_DATA_NUM; i++) {
            if(flash_data[i] != copy_data[i]) {
                printf("@%08x-@%08x : %02x != %02x\n" , &flash_data[i] , &copy_data[i], flash_data[i], copy_data[i]);
                errors++;
            }
            count++;
        }
    #else
        for (int i = 0; i<COPY_DATA_NUM; i++) {
            if(flash_data[i] != copy_data[i]) {
                printf("@%08x-@%08x : %02x != %02x\n" , &flash_data[i] , &copy_data[i], flash_data[i], copy_data[i]);
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

