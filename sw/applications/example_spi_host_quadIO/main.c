// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

/**
 * \brief Fast Read Quad I/O SPI Host example
 * 
 * Simple example to check the Fast Read Quad I/O SPI_host functionality.
 * It checks that the ram and flash have the same content.
 * 
 * \author Mattia Consani, EPFL
*/


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
#include "x-heep.h"


// W25Q128JW flash commands supported by Questasim flash model
// Also FFh and EDh are supported by the simulation model, but not by the phisical flash
#define W25Q128JW_CMD_RELEASE_POWERDOWN  0xab
#define W25Q128JW_CMD_POWERDOWN          0xb9
#define W25Q128JW_CMD_READ               0x03
#define W25Q128JW_CMD_READ_DUALIO        0xbb
#define W25Q128JW_CMD_READ_QUADIO        0xeb
// Not supported in Verilog flash model
#define W25Q128JW_CMD_READ_REG2          0x35
#define W25Q128JW_CMD_WRITE_REG2         0x31
#define W25Q128JW_CMD_WRITE_ENABLE       0x06


#ifdef TARGET_PYNQ_Z2
    #define USE_SPI_FLASH
#endif


#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif


#define REVERT_24b_ADDR(addr) ((((uint32_t)(addr) & 0xff0000) >> 16) | ((uint32_t)(addr) & 0xff00) | (((uint32_t)(addr) & 0xff) << 16))

#define FLASH_CLK_MAX_HZ (133*1000*1000) // In Hz (133 MHz for the flash w25q128jvsim used in the EPFL Programmer)

volatile int8_t spi_intr_flag;
spi_host_t spi_host;
uint32_t flash_data[8];

// This is the vector that the spi_host is reading from the flash
uint32_t flash_original[8] = {0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef};
// uint32_t flash_original[8] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

#ifndef USE_SPI_FLASH
void fic_irq_spi(void)
{
    // Disable SPI interrupts
    spi_enable_evt_intr(&spi_host, false);
    spi_enable_rxwm_intr(&spi_host, false);
    spi_intr_flag = 1;
}
#else
void fic_irq_spi_flash(void)
{
    // Disable SPI interrupts
    spi_enable_evt_intr(&spi_host, false);
    spi_enable_rxwm_intr(&spi_host, false);
    spi_intr_flag = 1;
}
#endif


int main(int argc, char *argv[])
{
    PRINTF("Quad I/O SPI Host example\n\r");

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);


    // [WARNING]: this part was not updated to support quad SPI
    if ( get_spi_flash_mode(&soc_ctrl) == SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO )
    {
    #ifdef USE_SPI_FLASH
        PRINTF("This application cannot work with the memory mapped SPI FLASH module - do not use the FLASH_EXEC linker script for this application\n");
        return EXIT_SUCCESS;
    #else
        PRINTF("This application is not supporting quad SPI in memory mapped mode (yet)\n");
        return EXIT_FAILURE;
        /*
            if we are using in SIMULATION the SPIMMIO from Yosys, then the flash_original data is different
            as the compilation is done differently, so we will store there the first WORDs of code mapped at the beginning of the FLASH
        */
        uint32_t* ptr_flash = (uint32_t*)FLASH_MEM_START_ADDRESS;
        for(int i =0; i < 8 ; i++){
            flash_original[i] = ptr_flash[i];
        }
        // we read the data from the FLASH address 0x0, which corresponds to FLASH_MEM_START_ADDRESS
        uint32_t read_byte_cmd_spimemio = ((REVERT_24b_ADDR(0x0) << 8) | 0x03); // The address bytes sent through the SPI to the Flash are in reverse order
    #endif
    }
    // [WARNING STOP] -----------------------------


    // spi_host_t spi_host;
    #ifndef USE_SPI_FLASH
        spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    #else
        spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    #endif

    uint32_t core_clk = soc_ctrl_get_frequency(&soc_ctrl);

    #ifdef USE_SPI_FLASH
    // Select SPI host as SPI output
    soc_ctrl_select_spi_host(&soc_ctrl);
    #endif

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

    // Enable SPI host device
    spi_set_enable(&spi_host, true);
    // Enable SPI output
    spi_output_enable(&spi_host, true);

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

    // Set RX watermark to 8 word
    spi_set_rx_watermark(&spi_host, 8);

    // ----------------COMMAND----------------
    // Power up flash
    // ----------------COMMAND----------------
    
    // Create segment 1
    const uint32_t powerup_byte_cmd = W25Q128JW_CMD_RELEASE_POWERDOWN;
    spi_write_word(&spi_host, powerup_byte_cmd);
    spi_wait_for_ready(&spi_host);

    const uint32_t cmd_powerup = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi_host, cmd_powerup);
    spi_wait_for_ready(&spi_host);
    // ----------------END COMMAND----------------

    

    // W25Q128JW requires the QE (Quad Enable) bit to be set in order to operate at quad speed
    // The Verilog flash do not model this behavior and no actions are required
    #ifdef TARGET_PYNQ_Z2

    PRINTF("FPGA target: setting QE bit...\n\r");
    spi_set_rx_watermark(&spi_host,1);

    // ----------------COMMAND----------------
    // Read Status Register 2
    // ----------------COMMAND----------------
    
    // Create segment 1
    const uint32_t reg2_read_cmd = W25Q128JW_CMD_READ_REG2;
    spi_write_word(&spi_host, reg2_read_cmd);
    spi_wait_for_ready(&spi_host);

    const uint32_t reg2_read_1 = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = true,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi_host, reg2_read_1);
    spi_wait_for_ready(&spi_host);


    // Create segment 2
    const uint32_t reg2_read_2 = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Standard speed
        .direction  = kSpiDirRxOnly      // Read only
    });
    spi_set_command(&spi_host, reg2_read_2);
    spi_wait_for_ready(&spi_host);
    spi_wait_for_rx_watermark(&spi_host);
    
    // the partial word will be zero-padded and inserted into the RX FIFO once the segment is completed
    uint32_t reg2_data; // The actual register is 8 bit, but the SPI host gives a full word
    spi_read_word(&spi_host, &reg2_data);
    // ----------------END COMMAND----------------


    // Set bit in position 1 (QE bit)
    PRINTF("before reg2_data = 0x%x\n\r", reg2_data);
    reg2_data |= 0x2;
    PRINTF("after reg2_data = 0x%x\n\r", reg2_data);


    // ----------------COMMAND----------------
    // Write Enable - WEL (Write Enable Latch) set
    // ----------------COMMAND----------------
    // Create segment 1
    const uint32_t write_enable_cmd = W25Q128JW_CMD_WRITE_ENABLE;
    spi_write_word(&spi_host, write_enable_cmd);
    const uint32_t cmd_write_en = spi_create_command((spi_command_t){
        .len        = 0,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_write_en);
    spi_wait_for_ready(&spi_host);
    // ----------------END COMMAND----------------


    // ----------------COMMAND----------------
    // Write Status Register 2
    // ----------------COMMAND----------------
    // Create segment 1
    const uint32_t reg2_write_cmd = W25Q128JW_CMD_WRITE_REG2;
    spi_write_word(&spi_host, reg2_write_cmd);
    spi_wait_for_ready(&spi_host);

    const uint32_t reg2_write_1 = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = true,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi_host, reg2_write_1);
    spi_wait_for_ready(&spi_host);


    // Create segment 2
    spi_write_word(&spi_host, reg2_data);
    spi_wait_for_ready(&spi_host);

    const uint32_t reg2_write_2 = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Standard speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi_host, reg2_write_2);
    spi_wait_for_ready(&spi_host);
    // ----------------END COMMAND----------------
    
    // Check back the register
    spi_write_word(&spi_host, reg2_read_cmd);
    spi_wait_for_ready(&spi_host);
    spi_set_command(&spi_host, reg2_read_1);
    spi_wait_for_ready(&spi_host);
    spi_set_command(&spi_host, reg2_read_2);
    spi_wait_for_ready(&spi_host);
    spi_wait_for_rx_watermark(&spi_host);
    uint32_t reg2_data_check = 0x00;
    spi_read_word(&spi_host, &reg2_data_check);
    PRINTF("reg2_data_check = 0x%x\n\r", reg2_data_check);

    #endif


   // Set RX watermark to 8 word (32 bytes)
    spi_set_rx_watermark(&spi_host,8);
    

    // ----------------COMMAND----------------
    // Fast Read Quad I/O
    // ----------------COMMAND----------------

    // Create segment 1
    uint32_t cmd_read_quadIO = W25Q128JW_CMD_READ_QUADIO;
    spi_write_word(&spi_host, cmd_read_quadIO);
    spi_wait_for_ready(&spi_host);

    const uint32_t cmd_read = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = true,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi_host, cmd_read);
    spi_wait_for_ready(&spi_host);
    PRINTF("cmd_read = 0x%x\n\r", cmd_read);


    // Create segment 2
    uint32_t read_byte_cmd = (REVERT_24b_ADDR(flash_original) | (0xFF << 24)); // Fxh (here FFh) required by W25Q128JW
    PRINTF("read_byte_cmd = %u\n\r", read_byte_cmd);
    spi_write_word(&spi_host, read_byte_cmd);
    spi_wait_for_ready(&spi_host);

    const uint32_t cmd_address = spi_create_command((spi_command_t){
        .len        = 3,                // 3 Byte
        .csaat      = true,             // Command not finished
        .speed      = kSpiSpeedQuad,    // Quad speed
        .direction  = kSpiDirTxOnly     // Write only
    });
    spi_set_command(&spi_host, cmd_address);
    spi_wait_for_ready(&spi_host);
    PRINTF("cmd_address = 0x%x\n\r", cmd_address);

    
    // Create segment 3
    const uint32_t dummy_clocks_cmd = spi_create_command((spi_command_t){
        #ifdef TARGET_PYNQ_Z2
        .len        = 3,               // W25Q128JW flash needs 4 dummy cycles
        #else
        .len        = 7,               // SPI flash simulation model needs 8 dummy cycles
        #endif
        .csaat      = true,            // Command not finished
        .speed      = kSpiSpeedQuad,   // Quad speed
        .direction  = kSpiDirDummy     // Dummy
    });
    spi_set_command(&spi_host, dummy_clocks_cmd);
    spi_wait_for_ready(&spi_host);
    PRINTF("dummy_clocks_cmd = 0x%x\n\r", dummy_clocks_cmd);


    // Create segment 4
    const uint32_t cmd_read_rx = spi_create_command((spi_command_t){
        .len        = 31,              // 32 Byte
        .csaat      = false,           // End command
        .speed      = kSpiSpeedQuad,   // Quad speed
        .direction  = kSpiDirRxOnly    // Read only
    });
    spi_set_command(&spi_host, cmd_read_rx);
    spi_wait_for_ready(&spi_host);
    PRINTF("cmd_read_rx = 0x%x\n\r", cmd_read_rx);
    // ----------------END COMMAND----------------



    // Wait transaction is finished (polling register)
    PRINTF("Waiting for SPI...\n\r");
    spi_wait_for_rx_watermark(&spi_host);

    // Read data from SPI RX FIFO
    for (int i=0; i<8; i++) {
        spi_read_word(&spi_host, &flash_data[i]);
    }

    PRINTF("flash vs ram...\n\r");

    uint32_t errors = 0;
    uint32_t* ram_ptr = flash_original;
    for (int i=0; i<8; i++) {
        if(flash_data[i] != *ram_ptr) {
            PRINTF("@%x : %x != %x(ref)\n\r", ram_ptr, flash_data[i], *ram_ptr);
            errors++;
        } else {
            PRINTF("@%x : %x == %x(ref)\n\r", ram_ptr, flash_data[i], *ram_ptr);
        }
        ram_ptr++;
    }

    if (errors == 0) {
        PRINTF("success!\n\r");
    } else {
        PRINTF("failure, %d errors!\n\r", errors);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
