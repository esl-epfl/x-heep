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
#include "power_manager.h"
#include "x-heep.h"

#ifdef TARGET_PYNQ_Z2
    #define USE_SPI_FLASH
#endif

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

// Type of data frome the SPI. For types different than words the SPI data is requested in separate transactions
// word(0), half-word(1), byte(2,3)
#define SPI_DATA_TYPE DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_32BIT_WORD

// Number of elements to copy
#define COPY_DATA_NUM 16

#define FLASH_CLK_MAX_HZ (133*1000*1000) // In Hz (133 MHz for the flash w25q128jvsim used in the EPFL Programmer)

#define REVERT_24b_ADDR(addr) ((((uint32_t)(addr) & 0xff0000) >> 16) | ((uint32_t)(addr) & 0xff00) | (((uint32_t)(addr) & 0xff) << 16))

volatile int8_t dma_intr_flag;
int8_t core_sleep_flag;
spi_host_t spi_host;

static power_manager_t power_manager;

void dma_intr_handler_trans_done(void)
{
    PRINTF("Non-weak implementation of a DMA interrupt\n\r");
    dma_intr_flag = 1;
}

// Reserve memory array
uint32_t flash_data[COPY_DATA_NUM] __attribute__ ((aligned (4))) = {0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,0x679852fe,0xff8252bb,0x763b4521,0x6875adaa,0x09ac65bb,0x666ba334,0x44556677,0x0000ba98};
uint32_t copy_data[COPY_DATA_NUM] __attribute__ ((aligned (4)))  = { 0 };

#if SPI_DATA_TYPE == DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_32BIT_WORD
    #define DATA_TYPE uint32_t
#elif SPI_DATA_TYPE == DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_16BIT_WORD
    #define DATA_TYPE uint16_t
#else
    #define DATA_TYPE uint8_t
#endif

#define COPY_DATA_TYPE (COPY_DATA_NUM/(sizeof(uint32_t)/sizeof(DATA_TYPE)))

int main(int argc, char *argv[])
{

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    uint32_t read_byte_cmd;
    uint32_t* flash_data_lma = flash_data;
    //set MS 8 bits to 0 as the flash only uses 24b
    flash_data_lma = (uint32_t*) ((uint32_t)(flash_data_lma) & 0x00FFFFFF);


   if ( get_spi_flash_mode(&soc_ctrl) == SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO )
    {
#ifdef USE_SPI_FLASH
        PRINTF("This application cannot work with the memory mapped SPI FLASH module - do not use the FLASH_EXEC linker script for this application\n");
        return EXIT_SUCCESS;
#else
        /*
            if we are using in SIMULATION the SPIMMIO from Yosys, then the flash_original data is different
            as the compilation is done differently, so we will store there the first WORDs of code mapped at the beginning of the FLASH
        */
        uint32_t* ptr_flash = (uint32_t*)FLASH_MEM_START_ADDRESS;
        for(int i =0; i < COPY_DATA_NUM ; i++){
            flash_data[i] = ptr_flash[i];
        }
#endif
    }

    #ifndef USE_SPI_FLASH
        spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    #else
        spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    #endif

    // Setup power_manager
    mmio_region_t power_manager_reg = mmio_region_from_addr(POWER_MANAGER_START_ADDRESS);
    power_manager.base_addr = power_manager_reg;
    power_manager_counters_t power_manager_cpu_counters;
    // Init cpu_subsystem's counters
    if (power_gate_counters_init(&power_manager_cpu_counters, 300, 300, 300, 300, 300, 300, 0, 0) != kPowerManagerOk_e)
    {
        PRINTF("Error: power manager fail. Check the reset and powergate counters value\n\r");
        return EXIT_FAILURE;
    }

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

    core_sleep_flag = 0;

    // -- DMA CONFIGURATION --

    dma_init(NULL);

    #ifndef USE_SPI_FLASH
        uint8_t slot =  DMA_TRIG_SLOT_SPI_RX ; // The DMA will wait for the SPI RX FIFO valid signal
    #else
        uint8_t slot =  DMA_TRIG_SLOT_SPI_FLASH_RX ; // The DMA will wait for the SPI FLASH RX FIFO valid signal
    #endif

    static dma_target_t tgt_src = {
        .size_du = COPY_DATA_NUM,
        .inc_du = 0,
        .type = SPI_DATA_TYPE,
    };
    tgt_src.ptr = fifo_ptr_rx;
    tgt_src.trig = slot;

    static dma_target_t tgt_dst = {
        .ptr = copy_data,
        .inc_du = 1,
        .type = SPI_DATA_TYPE,
        .trig = DMA_TRIG_MEMORY,
    };

    static dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .end = DMA_TRANS_END_INTR,
    };

    dma_config_flags_t res;

    res = dma_validate_transaction(&trans ,DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY);
    PRINTF("trans: %u \n\r", res );
    res = dma_load_transaction(&trans);
    PRINTF(" load: %u \n\r", res );



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

    dma_intr_flag = 0;
    dma_launch(&trans);
    PRINTF("Launched\n\r");

    #if SPI_DATA_TYPE == DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_32BIT_WORD
        if(get_spi_flash_mode(&soc_ctrl) != SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO)
            read_byte_cmd = ((REVERT_24b_ADDR(flash_data_lma) << 8) | 0x03); // The address bytes sent through the SPI to the Flash are in reverse order
        else
            // we read the data from the FLASH address 0x0, which corresponds to FLASH_MEM_START_ADDRESS
            read_byte_cmd = ((REVERT_24b_ADDR(0x0) << 8) | 0x03); // The address bytes sent through the SPI to the Flash are in reverse order
        const uint32_t cmd_read_rx = spi_create_command((spi_command_t){ // Single transaction
            .len        = COPY_DATA_NUM*sizeof(DATA_TYPE) - 1, // In bytes - 1
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
            .len        = (sizeof(DATA_TYPE) - 1),
            .csaat      = false,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirRxOnly
        });
        DATA_TYPE* flash_ptr = (DATA_TYPE *)flash_data_lma;
        for (int i = 0; i<COPY_DATA_NUM; i++) { // Multiple 8 or 16-bit transactions, just to try a new mode, we could treat it as int32
            // Request the same data multiple times
            if(get_spi_flash_mode(&soc_ctrl) != SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO)
                read_byte_cmd = ((REVERT_24b_ADDR(&flash_ptr[i]) << 8) | 0x03); // The address bytes sent through the SPI to the Flash are in reverse order
            else
                read_byte_cmd = ((REVERT_24b_ADDR(i) << 8) | 0x03);
            spi_write_word(&spi_host, read_byte_cmd); // Fill TX FIFO with TX data (read command + 3B address)
            spi_wait_for_ready(&spi_host); // Wait for readiness to process commands
            spi_set_command(&spi_host, cmd_read); // Send read command to the external device through SPI
            spi_wait_for_ready(&spi_host);
            spi_set_command(&spi_host, cmd_read_rx); // Receive data in RX
            spi_wait_for_ready(&spi_host);
        }
    #endif

    // Power gate core and wait for fast DMA interrupt
    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if(dma_intr_flag == 0) {
        if (power_gate_core(&power_manager, kDma_pm_e, &power_manager_cpu_counters) != kPowerManagerOk_e)
        {
            PRINTF("Error: power manager fail.\n\r");
            return EXIT_FAILURE;
        }
        core_sleep_flag = 1;
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    if(core_sleep_flag == 1) PRINTF("Woke up from sleep!\n\r");

    // Wait for DMA interrupt
    if( trans.end == DMA_TRANS_END_POLLING ){
        PRINTF("Waiting for DMA DONE...\n\r");
        while( ! dma_is_ready() ){};
    } else{
        PRINTF("Waiting for the DMA interrupt...\n\r");
        while(dma_intr_flag == 0) {
            wait_for_interrupt();
        }
    }
    PRINTF("triggered!\n\r");

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
    PRINTF("flash vs ram...\n\r");

    uint32_t errors = 0;
    uint32_t count = 0;
    for (int i = 0; i<COPY_DATA_TYPE; i++) {
        if(flash_data[i] != copy_data[i]) {
            PRINTF("@%08x-@%08x : %02x != %02x\n\r" , &flash_data[i] , &copy_data[i], flash_data[i], copy_data[i]);
            errors++;
        }
        count++;
    }

    if (errors == 0) {
        PRINTF("success! (bytes checked: %d)\n\r", count*sizeof(DATA_TYPE));
    } else {
        PRINTF("failure, %d errors! (Out of %d)\n\r", errors, count);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
