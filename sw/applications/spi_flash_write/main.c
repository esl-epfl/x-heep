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

/* Test Configurations */
#define DEBUG
#define USE_SPI_FLASH
//#define TEST_CIRCULAR
#define TEST_MEM_2_SPI
#define TEST_SPI_2_MEM

#define CIRCULAR_CYCLES 5
#define COPY_DATA_WORDS 16 // Flash page size = 256 Bytes

#define REVERT_24b_ADDR(addr) ((((uint32_t)addr & 0xff0000) >> 16) | ((uint32_t)addr & 0xff00) | (((uint32_t)addr & 0xff) << 16))
#define FLASH_ADDR 0x00008500 // 256B data alignment
#define FLASH_CLK_MAX_HZ (133*1000*1000) // In Hz (133 MHz for the flash w25q128jvsim used in the EPFL Programmer)

// Use PRINTF instead of PRINTF to remove print by default
#ifdef DEBUG
  #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
  #define PRINTF(...)
#endif // DEBUG
#define PRINTF2(fmt, ...)    printf(fmt, ## __VA_ARGS__)

int8_t spi_intr_flag;
int8_t dma_intr_flag;
spi_host_t spi_host;
int8_t cycles;

// Reserve memory array
uint32_t flash_data[COPY_DATA_WORDS] __attribute__ ((aligned (4))) = {0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,0x679852fe,0xff8252bb,0x763b4521,0x6875adaa,0x09ac65bb,0x666ba334,0x44556677,0x0000ba98};
uint32_t copy_data[COPY_DATA_WORDS] __attribute__ ((aligned (4)))  = { 0 };

uint32_t *fifo_ptr_tx;
uint32_t *fifo_ptr_rx;

#ifndef USE_SPI_FLASH
void handler_irq_fast_spi(void)
{
    // Disable SPI interrupts
    spi_enable_evt_intr(&spi_host, false);
    spi_enable_rxwm_intr(&spi_host, false);
    // Clear fast interrupt
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kSpi_fic_e);
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
    clear_fast_interrupt(&fast_intr_ctrl, kSpiFlash_fic_e);
    spi_intr_flag = 1;
}
#endif //USE_SPI_FLASH

#ifdef TEST_CIRCULAR
void dma_intr_handler(void)
{
    PRINTF("#");
    cycles++;
    if( cycles == CIRCULAR_CYCLES ) dma_stop_circular();
}
#else
void dma_intr_handler(void)
{
    PRINTF("#");
    dma_intr_flag = 1;
}
#endif //TEST_CIRCULAR

void spi_config()
{

/*
* SPI CONFIGURATIONS
*/
#ifndef USE_SPI_FLASH
    spi_host.base_addr = mmio_region_from_addr((uintptr_t)SPI2_START_ADDRESS);
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
        // Select SPI host as SPI output
    soc_ctrl_select_spi_host(&soc_ctrl);
#endif // USE_SPI_FLASH


    CSR_SET_BITS(CSR_REG_MIE, mask);
    spi_intr_flag = 0;

    // Enable SPI host device
    spi_set_enable(&spi_host, true);
    // Enable SPI output
    spi_output_enable(&spi_host, true);

    // SPI and SPI_FLASH are the same IP so same register map
    fifo_ptr_tx = spi_host.base_addr.base + SPI_HOST_TXDATA_REG_OFFSET;
    fifo_ptr_rx = spi_host.base_addr.base + SPI_HOST_RXDATA_REG_OFFSET;

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
}


int main(int argc, char *argv[])
{

    spi_config();
    dma_init();

    dma_config_flags_t res;

#ifdef TEST_MEM_2_SPI

    PRINTF("\n\n===================================\n\n");
    PRINTF("  MEM -> -> -> DMA -> -> -> SPI  ");
    PRINTF("\n\n===================================\n\n");


#ifndef USE_SPI_FLASH
    const uint8_t slot = DMA_TRIG_SLOT_SPI_TX; // The DMA will wait for the SPI TX FIFO ready signal
#else
    const uint8_t slot = DMA_TRIG_SLOT_SPI_FLASH_TX; // The DMA will wait for the SPI FLASH TX FIFO ready signal
#endif //USE_SPI_FLASH

    static dma_target_t tgt1= {
        .ptr = flash_data,
        .inc_du = 1,
        .size_du = COPY_DATA_WORDS,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_WORD,
    };

    static dma_target_t tgt2= {
        .inc_du = 0,
        .size_du = COPY_DATA_WORDS,
        .trig = slot,
        .type = DMA_DATA_TYPE_WORD,
    };

    tgt2.ptr = fifo_ptr_tx; // This is necessary because fifo_ptr_tx is not a constant, and therefore cannot be used as initializer element.

    static dma_trans_t trans = {
                            .src = &tgt1,
                            .dst = &tgt2,
                            .mode = DMA_TRANS_MODE_SINGLE,
                            .win_b = 0,
                            .end = DMA_TRANS_END_INTR,
                            };

#ifdef TEST_CIRCULAR
    trans.mode = DMA_TRANS_MODE_CIRCULAR;
    cycles = 0;
#endif // TEST_CIRCULAR

    res = dma_create_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("tran: %u \n\r", res);
    
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \n\r", res);

    res = dma_launch(&trans);
    PRINTF("laun: %u \n\r", res);

    // Wait for the first data to arrive to the TX FIFO before enabling interrupt
    spi_wait_for_tx_not_empty(&spi_host);
    // Enable event interrupt
    spi_enable_evt_intr(&spi_host, true);
    // Enable TX empty interrupt
    spi_enable_txempty_intr(&spi_host, true);

    const uint32_t cmd_write_tx = spi_create_command((spi_command_t){
        .len        = COPY_DATA_WORDS*sizeof(*flash_data) - 1,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_write_tx);
    spi_wait_for_ready(&spi_host);

    // Wait for SPI interrupt
    PRINTF("Waiting for the SPI interrupt...\n");
    while(spi_intr_flag == 0) {
        wait_for_interrupt();
    }
    PRINTF("triggered!\n");

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

    PRINTF("%d Bytes written in Flash at @ 0x%08x \n", COPY_DATA_WORDS*sizeof(*flash_data), FLASH_ADDR);


#endif //TEST_SPI_2_MEM

#ifdef TEST_SPI_2_MEM

    PRINTF("\n\n===================================\n\n");
    PRINTF("  MEM <- <- <- DMA <- <- <- SPI  ");
    PRINTF("\n\n===================================\n\n");
    for( uint32_t iteration = 1000; iteration > 0; iteration-- )
    { 
        PRINTF("\n\n Iteration: %d \n\n", iteration);

#ifndef USE_SPI_FLASH
        const uint8_t slot2 = DMA_TRIG_SLOT_SPI_RX; // The DMA will wait for the SPI TX FIFO ready signal
#else
        const uint8_t slot2 = DMA_TRIG_SLOT_SPI_FLASH_RX; // The DMA will wait for the SPI FLASH TX FIFO ready signal
#endif

        static dma_target_t tgt3= {
        .ptr = copy_data,
        .inc_du = 1,
        .size_du = COPY_DATA_WORDS,
        .trig = DMA_TRIG_MEMORY,
        .type = DMA_DATA_TYPE_WORD,
        };

        static dma_target_t tgt4= {
            .inc_du = 0,
            .size_du = COPY_DATA_WORDS,
            .trig = slot2,
            .type = DMA_DATA_TYPE_WORD,
        };
        tgt4.ptr = fifo_ptr_rx;

        static dma_trans_t trans2 = {
                                .src = &tgt4,
                                .dst = &tgt3,
                                .end = DMA_TRANS_END_INTR,
                                };

        res = dma_create_transaction( &trans2, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
        PRINTF("tran: %u \n\r", res);
        
        res = dma_load_transaction(&trans2);
        PRINTF("load: %u \n\r", res);

        // The address bytes sent through the SPI to the Flash are in reverse order
        const int32_t read_byte_cmd = ((REVERT_24b_ADDR(FLASH_ADDR) << 8) | 0x03);

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
            .len        = COPY_DATA_WORDS*sizeof(*copy_data) - 1,
            .csaat      = false,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirRxOnly
        });
        spi_set_command(&spi_host, cmd_read_rx);
        spi_wait_for_ready(&spi_host);

        dma_intr_flag = 0;

        //PRINTF("///////////"); // This is a threshold printf. Shorter and it does not work. Longer works ok.
        //PRINTF("This is a long printf just to give some time.\n");

        /*
        volatile uint32_t pepe = 1000000;

        for( uint32_t i; i < pepe; i++ ){
            asm("nop");
        }
        */


        res = dma_launch(&trans2);
        PRINTF("laun: %u \n\r", res);
        
        PRINTF("Waiting for the DMA...\n");
        while( ! dma_is_ready() ){
            if( trans2.end == DMA_TRANS_END_INTR )
            {
                wait_for_interrupt();
            }
        }
        PRINTF("triggered!\n");

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
        PRINTF("flash vs ram...\n");

        int i;
        uint32_t errors = 0;
        uint32_t count = 0;
        for (i = 0; i<COPY_DATA_WORDS; i++) {
            if(flash_data[i] != copy_data[i]) {
                PRINTF("@%08x-@%08x : %02x\t!=\t%02x\n" , &flash_data[i] , &copy_data[i], flash_data[i], copy_data[i]);
                errors++;
            }
            count++;
        }

        if (errors == 0) {
            PRINTF("success! (Words checked: %d)\n", count);
        } else {
            PRINTF("failure, %d errors! (Out of %d)\n", errors, count);
        }
    }

#endif //TEST_MEM_2_SPI

    return EXIT_SUCCESS;
}
