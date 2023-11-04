/**
 * @file main.c
 * @brief Example application for the SPI host module - write to the SPI Flash
*/

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
#include "x-heep.h"

#ifdef TARGET_PYNQ_Z2
    #define USE_SPI_FLASH
#endif

#define REVERT_24b_ADDR(addr) ((((uint32_t)addr & 0xff0000) >> 16) | ((uint32_t)addr & 0xff00) | (((uint32_t)addr & 0xff) << 16))
#define FLASH_ADDR 0x00008500 // 256B data alignment
#define FLASH_CLK_MAX_HZ (133*1000*1000) // In Hz (133 MHz for the flash w25q128jvsim used in the EPFL Programmer)

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   1

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

int8_t spi_intr_flag;
spi_host_t spi_host;

// Test buffers
uint32_t flash_data_towrite[16] = {
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,
    0x89abcdef,0x679852fe,0xff8252bb,0x763b4521,0x6875adaa,0x09ac65bb,0x666ba334,
    0x44556677,0x0000ba98
};
uint32_t flash_data_toread[16] = {};

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
    // PRINTF("&");
    spi_enable_evt_intr(&spi_host, false);
    spi_enable_rxwm_intr(&spi_host, false);
    spi_intr_flag = 1;
}
#endif //USE_SPI_FLASH



static inline __attribute__((always_inline)) void spi_config() {
/*
* SPI CONFIGURATIONS
*/
#ifndef USE_SPI_FLASH
    spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
#else
    spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
#endif //USE_SPI_FLASH

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    uint32_t core_clk = soc_ctrl_get_frequency(&soc_ctrl);

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

#ifdef USE_SPI_FLASH
        // Select SPI host as SPI output
        soc_ctrl_select_spi_host(&soc_ctrl);
#endif // USE_SPI_FLASH


    CSR_SET_BITS(CSR_REG_MIE, mask);

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

    // Reset
    const uint32_t reset_cmd = 0xFFFFFFFF; // ???
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
}

static inline __attribute__((always_inline)) void spi_wait_4_resp()
{
    // Check status register status waiting for ready
    bool flash_busy = true;
    uint8_t flash_resp[4] = {0xff,0xff,0xff,0xff};
    while(flash_busy){
        uint32_t flash_cmd = 0x00000005; // [CMD] Read status register
        spi_write_word(&spi_host, flash_cmd); // Push TX buffer
        uint32_t spi_status_cmd = spi_create_command((spi_command_t){
            .len        = 0,
            .csaat      = true,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirTxOnly
        });
        uint32_t spi_status_read_cmd = spi_create_command((spi_command_t){
            .len        = 0,
            .csaat      = false,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirRxOnly
        });
        spi_set_command(&spi_host, spi_status_cmd);
        spi_wait_for_ready(&spi_host);
        spi_set_command(&spi_host, spi_status_read_cmd);
        spi_wait_for_ready(&spi_host);
        spi_wait_for_rx_watermark(&spi_host);
        spi_read_word(&spi_host, &flash_resp[0]);
        if ((flash_resp[0] & 0x01) == 0) flash_busy = false;
    }
}


int main(int argc, char *argv[]) {
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

    #ifdef USE_SPI_FLASH
    if ( get_spi_flash_mode(&soc_ctrl) == SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO )
    {
        PRINTF("This application cannot work with the memory mapped SPI FLASH module - do not use the FLASH_EXEC linker script for this application\n");
        return EXIT_SUCCESS;
    }
    #endif

    /**
     * Configure the SPI<->flash connection parameters and the SPI host.
     * Set the write enale bit.
    */
    spi_config();

    // Write command - Program Page + address
    const uint32_t write_byte_cmd = ((REVERT_24b_ADDR(FLASH_ADDR) << 8) | 0x02);
    spi_write_word(&spi_host, write_byte_cmd);
    const uint32_t cmd_write = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = true, 
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_write);
    spi_wait_for_ready(&spi_host);

    // Write test buffer to SPI TX FIFO
    for (int i = 0; i < 16; i++) {
        spi_write_word(&spi_host, flash_data_towrite[i]);
    }
    const uint32_t cmd_write1 = spi_create_command((spi_command_t){
        .len        = 16*4 - 1,
        .csaat      = false, 
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_write1);
    spi_wait_for_ready(&spi_host);

    // Wait for all the fifo to be drained
    spi_wait_for_tx_empty(&spi_host);
    #ifndef TARGET_SIM
    spi_wait_4_resp()
    #endif // TARGET_SIM

    // Read back the data
    // Fill TX FIFO with TX data (read command + 3B address)
    const uint32_t read_byte_cmd = ((REVERT_24b_ADDR(FLASH_ADDR) << 8) | 0x03);
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

    spi_set_rx_watermark(&spi_host, 16);
    const uint32_t cmd_read_rx = spi_create_command((spi_command_t){
        .len        = 16*4 - 1,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirRxOnly
    });
    spi_set_command(&spi_host, cmd_read_rx);
    spi_wait_for_ready(&spi_host);
    
    // Wait till all the data is received
    spi_wait_for_rx_watermark(&spi_host);

    // Read data from SPI RX FIFO
    for (int i=0; i<16; i++) {
        spi_read_word(&spi_host, &flash_data_toread[i]);
    }

    PRINTF("flash vs ram...\n\r");

    uint32_t errors = 0;
    uint32_t* ram_ptr = flash_data_towrite;
    for (int i=0; i<16; i++) {
        if(flash_data_toread[i] != *ram_ptr) {
            PRINTF("@%x : %x != %x\n\r", ram_ptr, flash_data_toread[i], *ram_ptr);
            errors++;
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