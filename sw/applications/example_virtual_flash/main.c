
#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "rv_timer.h"
#include "rv_timer_regs.h"
#include "soc_ctrl.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "spi_host.h"
#include "spi_host_regs.h"
#include "dma.h"
#include "fast_intr_ctrl.h"
#include "gpio.h"
#include "fast_intr_ctrl_regs.h"
#include "x-heep.h"

#define REVERT_24b_ADDR(addr) ((((uint32_t)addr & 0xff0000) >> 16) | ((uint32_t)addr & 0xff00) | (((uint32_t)addr & 0xff) << 16))
#define FLASH_ADDR 0x00000000
#define FLASH_SIZE 64 * 1024 * 1024
#define FLASH_CLK_MAX_HZ (133 * 1000 * 1000)


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

// Interrupt controller variables
plic_result_t plic_res;
uint32_t intr_num;

//volatile int8_t timer_flag;
volatile int8_t spi_intr_flag;

spi_host_t spi_host_flash;

void dma_intr_handler_trans_done(){
    PRINTF("#\n\r");
}

void fic_irq_spi_flash(){
    // Disable SPI interrupts
    spi_enable_evt_intr(&spi_host_flash, false);
    spi_enable_rxwm_intr(&spi_host_flash, false);
    spi_intr_flag = 1;
    PRINTF("@\n\r");
}


void write_to_flash(spi_host_t *SPI, uint16_t *data, uint32_t byte_count, uint32_t addr)
{
    uint32_t write_to_mem = 0x02;
    spi_write_word(SPI, write_to_mem);
    uint32_t cmd_write_to_mem = spi_create_command((spi_command_t){
        .len       = 0,
        .csaat     = true,
        .speed     = kSpiSpeedStandard,
        .direction = kSpiDirTxOnly
    });
    spi_set_command(SPI, cmd_write_to_mem);
    spi_wait_for_ready(SPI);

    uint32_t addr_cmd = __builtin_bswap32(addr);
    spi_write_word(SPI, addr_cmd);
    uint32_t cmd_address = spi_create_command((spi_command_t){
        .len       = 3,
        .csaat     = true,
        .speed     = kSpiSpeedStandard,
        .direction = kSpiDirTxOnly
    });
    spi_set_command(SPI, cmd_address);
    spi_wait_for_ready(SPI);

    uint32_t *fifo_ptr_tx = SPI->base_addr.base + SPI_HOST_TXDATA_REG_OFFSET;

    // -- DMA CONFIGURATION --
    dma_init(NULL);

    dma_target_t tgt_src = {
        .ptr = data,
        .inc_du = 1,
        .size_du = 64, 
        .type = DMA_DATA_TYPE_HALF_WORD,
        .trig = DMA_TRIG_MEMORY, 
    };
    dma_target_t tgt_dst = {
        .ptr = fifo_ptr_tx,
        .inc_du = 0,
        .size_du = 0,
        .type = DMA_DATA_TYPE_HALF_WORD,
        .trig = DMA_TRIG_SLOT_SPI_FLASH_TX,
    };
    dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .end = DMA_TRANS_END_INTR,
    };

    dma_config_flags_t res;
    
    spi_intr_flag = 0;

    res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("trans: %u\n\r", res );
    res = dma_load_transaction(&trans);
    PRINTF("load: %u\n\r", res );
    res = dma_launch(&trans);

    // Wait for the first data to arrive to the TX FIFO before enabling interrupt
    spi_wait_for_tx_not_empty(SPI);
    // Enable event interrupt
    spi_enable_evt_intr(SPI, true);
    // Enable TX empty interrupt
    spi_enable_txempty_intr(SPI, true);

    const uint32_t cmd_write_tx = spi_create_command((spi_command_t){
        .len       = byte_count - 1,
        .csaat     = false,
        .speed     = kSpiSpeedStandard,
        .direction = kSpiDirTxOnly
    });
    spi_set_command(SPI, cmd_write_tx);
    spi_wait_for_ready(SPI);

   // Wait for SPI interrupt
   while(spi_intr_flag == 0) {
        wait_for_interrupt();
   }

    PRINTF("%d words written to flash.\n\n\r", byte_count/4);
}

int main(int argc, char *argv[])
{
    // Get current Frequency
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    soc_ctrl_select_spi_host(&soc_ctrl);

    uint32_t core_clk = soc_ctrl_get_frequency(&soc_ctrl);

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    uint32_t mask = 1 << 21;
    CSR_SET_BITS(CSR_REG_MIE, mask);
    spi_intr_flag = 0;
    // Set mie.MEIE bit to one to enable timer interrupt
    mask = 1 << 7;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    spi_host_flash.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    spi_set_enable(&spi_host_flash, true);
    spi_output_enable(&spi_host_flash, true);

    uint16_t clk_div = 0;
    if (FLASH_CLK_MAX_HZ < core_clk / 2)
    {
        clk_div = (core_clk / (FLASH_CLK_MAX_HZ)-2) / 2; // The value is truncated
        if (core_clk / (2 + 2 * clk_div) > FLASH_CLK_MAX_HZ)
            clk_div += 1; // Adjust if the truncation was not 0
    }

    // SPI Configuration
    // Configure chip 0 (flash memory)
    const uint32_t chip_cfg_flash = spi_create_configopts((spi_configopts_t){
        .clkdiv = clk_div,
        .csnidle = 0xF,
        .csntrail = 0xF,
        .csnlead = 0xF,
        .fullcyc = false,
        .cpha = 0,
        .cpol = 0});
    spi_set_configopts(&spi_host_flash, 0, chip_cfg_flash);
    spi_set_csid(&spi_host_flash, 0);

    // To set the number of dummy cycles we have to send command 0x11 and then a 1B value
    const uint32_t reset_cmd = 0x11;
    spi_write_word(&spi_host_flash, reset_cmd);
    const uint32_t cmd_reset = spi_create_command((spi_command_t){
        .len        = 0,
        .csaat      = true,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi_host_flash, cmd_reset);
    spi_wait_for_ready(&spi_host_flash);

    const uint32_t set_dummy_cycle = 0x07;
    spi_write_word(&spi_host_flash, set_dummy_cycle);
    const uint32_t cmd_set_dummy = spi_create_command((spi_command_t){
        .len        = 0,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });

    spi_set_command(&spi_host_flash, cmd_set_dummy);
    spi_wait_for_ready(&spi_host_flash);

    uint32_t results[32];
    for(uint32_t i = 0; i < 32; i++){
        results[i] = i;
    }

    write_to_flash(&spi_host_flash, results, sizeof(*results) * 32, FLASH_ADDR);

    PRINTF("Success.\n\r");

    return EXIT_SUCCESS;
}
