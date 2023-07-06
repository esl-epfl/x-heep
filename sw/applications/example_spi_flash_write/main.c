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
#include "x-heep.h"


#ifdef TARGET_PYNQ_Z2
    #define USE_SPI_FLASH
#endif
/* Test Configurations */
#define TEST_CIRCULAR
#define TEST_MEM_2_SPI
#define TEST_SPI_2_MEM

// These defines are used only to easily change the data types. 
// If not needed, the proper way is using the dma_data_type_t enum. 
#define TEST_DATA_TYPE  DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_8BIT_WORD

#if TEST_DATA_TYPE == DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_8BIT_WORD
#define DATA_TYPE    uint8_t
#elif TEST_DATA_TYPE == DMA_DATA_TYPE_DATA_TYPE_VALUE_DMA_16BIT_WORD
#define DATA_TYPE    uint16_t
#else
#define DATA_TYPE    uint32_t
#endif


#ifdef TEST_CIRCULAR 
// WARNING: When using circular mode the amount of WORDS of each cycle needs to be 32 <= x <= 128
    #define CIRCULAR_CYCLES 4
    #define COPY_DATA_UNITS 256   
    #define COPY_DATA_PER_CYCLE ( COPY_DATA_UNITS / CIRCULAR_CYCLES )// Flash page size = 256 Bytes
#else
    #define COPY_DATA_UNITS 64
    #define COPY_DATA_PER_CYCLE COPY_DATA_UNITS
#endif //TEST_CIRCULAR 

#define REVERT_24b_ADDR(addr) ((((uint32_t)addr & 0xff0000) >> 16) | ((uint32_t)addr & 0xff00) | (((uint32_t)addr & 0xff) << 16))

#define FLASH_ADDR 0x00008500 // 256B data alignment

#define FLASH_CLK_MAX_HZ (133*1000*1000) // In Hz (133 MHz for the flash w25q128jvsim used in the EPFL Programmer)


/* Change this value to 0 to disable prints for FPGA and enable them for simulation. */
#define DEFAULT_PRINTF_BEHAVIOR 1

/* By default, printfs are activated for FPGA and disabled for simulation. */
#ifdef TARGET_PYNQ_Z2 
    #define ENABLE_PRINTF DEFAULT_PRINTF_BEHAVIOR
#else 
    #define ENABLE_PRINTF !DEFAULT_PRINTF_BEHAVIOR
#endif

#if ENABLE_PRINTF
  #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
  #define PRINTF(...)
#endif 

int8_t spi_intr_flag;
spi_host_t spi_host;
int8_t cycles;

// Reserve memory array
DATA_TYPE flash_data[COPY_DATA_UNITS] __attribute__ ((aligned (DMA_DATA_TYPE_2_SIZE(TEST_DATA_TYPE)))) = { 0 };
DATA_TYPE copy_data [COPY_DATA_UNITS] __attribute__ ((aligned (DMA_DATA_TYPE_2_SIZE(TEST_DATA_TYPE)))) = { 0 };

DATA_TYPE *fifo_ptr_tx;
DATA_TYPE *fifo_ptr_rx;

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

#ifdef TEST_CIRCULAR
void dma_intr_handler_trans_done(void)
{
    PRINTF("#");
    cycles++;
    if( cycles >= CIRCULAR_CYCLES -1 ) dma_stop_circular();
}
#else
void dma_intr_handler_trans_done(void)
{
    PRINTF("#");
}
#endif

static inline __attribute__((always_inline)) void spi_config()
{

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

int main(int argc, char *argv[])
{

#ifdef TARGET_SIM
  #pragma message("This app does not allow Flash write operations in simulation!")
    PRINTF("Flash writes are not permitted during Simulation, only on FPGA\n");
    return EXIT_SUCCESS;
#endif

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

#ifdef USE_SPI_FLASH
   if ( get_spi_flash_mode(&soc_ctrl) == SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO )
    {
        PRINTF("This application cannot work with the memory mapped SPI FLASH module - do not use the FLASH_EXEC linker script for this application\n");
        return EXIT_SUCCESS;
    }
#endif
    spi_config();
    dma_init(NULL);

    dma_config_flags_t res;

    for( DATA_TYPE i = 0; i < COPY_DATA_PER_CYCLE; i ++ )
    {
        ((DATA_TYPE*)flash_data)[i] = (DATA_TYPE) i;
    }

#ifdef TEST_MEM_2_SPI

    PRINTF("\n\n\r======================================\n\n\r");
    PRINTF(" MEM -> -> DMA -> -> SPI -> -> FLASH ");
    PRINTF("\n\n\r======================================\n\n\r");

#ifndef USE_SPI_FLASH
    const uint8_t slot = DMA_TRIG_SLOT_SPI_TX; // The DMA will wait for the SPI TX FIFO ready signal
#else
    const uint8_t slot = DMA_TRIG_SLOT_SPI_FLASH_TX; // The DMA will wait for the SPI FLASH TX FIFO ready signal
#endif //USE_SPI_FLASH

    static dma_target_t tgt1= {
        .ptr = flash_data,
        .inc_du = 1,
        .size_du = COPY_DATA_PER_CYCLE,
        .trig = DMA_TRIG_MEMORY,
        .type = TEST_DATA_TYPE,
    };

    static dma_target_t tgt2= {
        .inc_du = 0,
        .size_du = COPY_DATA_PER_CYCLE,
        .trig = slot,
        .type = TEST_DATA_TYPE,
    };

    tgt2.ptr = fifo_ptr_tx; // This is necessary because fifo_ptr_tx is not a constant, and therefore cannot be used as initializer element.

    static dma_trans_t trans = {
                            .src = &tgt1,
                            .dst = &tgt2,
                            .mode = DMA_TRANS_MODE_SINGLE,
                            .win_du = 0,
                            .end = DMA_TRANS_END_INTR,
                            };

#ifdef TEST_CIRCULAR
    trans.mode = DMA_TRANS_MODE_CIRCULAR;
    cycles = 0;
#endif // TEST_CIRCULAR

    res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("tran: %u \n\r", res);
    
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \n\r", res);

    res = dma_launch(&trans);

    spi_intr_flag = 0;
    // Wait for the first data to arrive to the TX FIFO before enabling interrupt
    spi_wait_for_tx_not_empty(&spi_host);
    // Enable event interrupt
    spi_enable_evt_intr(&spi_host, true);
    // Enable TX empty interrupt
    spi_enable_txempty_intr(&spi_host, true);

    const uint32_t cmd_write_tx = spi_create_command((spi_command_t){
        .len        = COPY_DATA_UNITS*DMA_DATA_TYPE_2_SIZE(TEST_DATA_TYPE) - 1,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host, cmd_write_tx);
    spi_wait_for_ready(&spi_host);

    // Wait for SPI interrupt
    while(spi_intr_flag == 0) {
        wait_for_interrupt();
    }
    PRINTF("triggered\n\r");

    spi_wait_4_resp();

    PRINTF("%d Bytes written in Flash at @ 0x%08x \n\r", COPY_DATA_UNITS*DMA_DATA_TYPE_2_SIZE(TEST_DATA_TYPE), FLASH_ADDR);


#endif //TEST_SPI_2_MEM

#ifdef TEST_SPI_2_MEM

    PRINTF("\n\n\r======================================\n\n\r");
    PRINTF(" MEM <- <- DMA <- <- SPI <- <- FLASH ");
    PRINTF("\n\n\r======================================\n\n\r");

#ifndef USE_SPI_FLASH
    const uint8_t slot2 = DMA_TRIG_SLOT_SPI_RX; // The DMA will wait for the SPI TX FIFO ready signal
#else
    const uint8_t slot2 = DMA_TRIG_SLOT_SPI_FLASH_RX; // The DMA will wait for the SPI FLASH TX FIFO ready signal
    #endif

    static dma_target_t tgt3= {
    .ptr = copy_data,
    .inc_du = 1,
    // Because the data type needs to be WORD the size needs to be normalized. 
    .size_du = COPY_DATA_PER_CYCLE*DMA_DATA_TYPE_2_SIZE(TEST_DATA_TYPE)/DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE_WORD),
    .trig = DMA_TRIG_MEMORY,
    .type = DMA_DATA_TYPE_WORD, // Only possible to read word-wise from SPI
    };

    static dma_target_t tgt4= {
        .inc_du = 0,
        // Because the data type needs to be WORD the size needs to be normalized. 
        .size_du = COPY_DATA_PER_CYCLE*DMA_DATA_TYPE_2_SIZE(TEST_DATA_TYPE)/DMA_DATA_TYPE_2_SIZE(DMA_DATA_TYPE_WORD),
        .trig = slot2,
        .type = DMA_DATA_TYPE_WORD,
    };
    tgt4.ptr = fifo_ptr_rx;

    static dma_trans_t trans2 = {
                            .src = &tgt4,
                            .dst = &tgt3,
                            .end = DMA_TRANS_END_INTR,
                            };

    res = dma_validate_transaction( &trans2, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
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
        .len        = COPY_DATA_UNITS*DMA_DATA_TYPE_2_SIZE(TEST_DATA_TYPE) - 1,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirRxOnly
    });
    spi_set_command(&spi_host, cmd_read_rx);
    spi_wait_for_ready(&spi_host);

    res = dma_launch(&trans2);
    
    while( ! dma_is_ready() ){
        /* wait_for_interrupt(); For small buffer sizes the interrupt arrives before going to wfi(); */
    }; 

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
    PRINTF("ram vs flash...\n\r");

    int i;
    uint32_t errors = 0;
    uint32_t count = 0;
    for (i = 0; i<COPY_DATA_UNITS*DMA_DATA_TYPE_2_SIZE(TEST_DATA_TYPE); i++) {
        if(((uint8_t*)flash_data)[i] != ((uint8_t*)copy_data)[i]) {
            PRINTF("@%08x-@%08x : %02d\t!=\t%02d\n\r" , &((uint8_t*)flash_data)[i] , &((uint8_t*)copy_data)[i], ((uint8_t*)flash_data)[i], ((uint8_t*)copy_data)[i]);
            errors++;
        }
        count++;
    }

    if (errors == 0) {
        PRINTF("success! (Data units checked: %d)\n\r", count);
    } else {
        PRINTF("Failure, %d errors! (Out of %d)\n\r", errors, count);
        return EXIT_FAILURE;
    }

#endif //TEST_MEM_2_SPI

    return EXIT_SUCCESS;
}
