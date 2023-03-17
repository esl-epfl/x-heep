
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

#define REVERT_24b_ADDR(addr) ((((uint32_t)addr & 0xff0000) >> 16) | ((uint32_t)addr & 0xff00) | (((uint32_t)addr & 0xff) << 16))
#define FLASH_ADDR 0x00000000
#define FLASH_SIZE 64 * 1024 * 1024
#define FLASH_CLK_MAX_HZ (133 * 1000 * 1000)

// Interrupt controller variables
dif_plic_params_t rv_plic_params;
dif_plic_t rv_plic;
dif_plic_result_t plic_res;
dif_plic_irq_id_t intr_num;

//volatile int8_t timer_flag;
volatile int8_t spi_intr_flag;

spi_host_t spi_host_flash;

void handler_irq_fast_spi_flash(void)
{
    // Disable SPI interrupts
    spi_enable_evt_intr(&spi_host_flash, false);
    spi_enable_rxwm_intr(&spi_host_flash, false);
    // Clear fast interrupt
    fast_intr_ctrl_t fast_intr_ctrl;
    fast_intr_ctrl.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    clear_fast_interrupt(&fast_intr_ctrl, kSpiFlash_fic_e);
    spi_intr_flag = 1;
}

void write_to_flash(spi_host_t *SPI, dma_t *DMA, uint16_t *data, uint32_t byte_count, uint32_t addr)
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
    dma_set_read_ptr_inc(DMA, (uint32_t) 2);
    dma_set_write_ptr_inc(DMA, (uint32_t) 0); // Do not increment address when reading from the SPI (Pop from FIFO)
    dma_set_read_ptr(DMA, (uint32_t) data);
    dma_set_write_ptr(DMA, (uint32_t) fifo_ptr_tx);

    // Set the correct SPI-DMA mode:
    // (0) disable
    // (1) receive from SPI (use SPI_START_ADDRESS for spi_host pointer)
    // (2) send to SPI (use SPI_START_ADDRESS for spi_host pointer)
    // (3) receive from SPI FLASH (use SPI_FLASH_START_ADDRESS for spi_host pointer)
    // (4) send to SPI FLASH (use SPI_FLASH_START_ADDRESS for spi_host pointer)
    dma_set_spi_mode(DMA, (uint32_t) 4); // The DMA will wait for the SPI FLASH TX FIFO ready signal
    dma_set_data_type(DMA, (uint32_t) 1); // 1 is for 16-bits
    dma_set_cnt_start(DMA, (uint32_t)byte_count); // Size of data received by SPI

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

    printf("%d words written to flash.\n\n\r", byte_count/4);
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

    spi_host_flash.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    spi_set_enable(&spi_host_flash, true);
    spi_output_enable(&spi_host_flash, true);

    dma_t dma;
    dma.base_addr = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);

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

    write_to_flash(&spi_host_flash, &dma, results, sizeof(*results) * 32, FLASH_ADDR);

    printf("Application ended successfully.\n\r");

    return EXIT_SUCCESS;
}
