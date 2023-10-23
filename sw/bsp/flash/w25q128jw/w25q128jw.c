/**
 * @file w25q128jw.c
 * @brief W25Q128JW Flash Board Support Package source file.
 *
 *
 * @author Mattia Consani
 */

#include "w25q128jw.h"


#define REVERT_24b_ADDR(addr) ((((uint32_t)(addr) & 0xff0000) >> 16) | ((uint32_t)(addr) & 0xff00) | (((uint32_t)(addr) & 0xff) << 16))

// Global variables
volatile int8_t spi_intr_flag;
spi_host_t spi;

// Prototypes of helper functions
static void power_up_flash();
static void set_QE_bit();
static void configure_spi(soc_ctrl_t soc_ctrl);
static void wait_flash();
static void reset_flash();
static void page_write(uint32_t addr, uint8_t *data, uint32_t length);


// ----------------------------------------------------------------


uint8_t w25q128jw_init() {
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    spi.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    if (get_spi_flash_mode(&soc_ctrl) == SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO) {
        return 0; // Error
    }

    // Enable interrupt on processor side
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    const uint32_t mask = 1 << 20;
    CSR_SET_BITS(CSR_REG_MIE, mask);
    spi_intr_flag = 0;

    // Select SPI host as SPI output
    soc_ctrl_select_spi_host(&soc_ctrl);
    
    spi_set_enable(&spi, true);
    spi_output_enable(&spi, true);

    // Configure SPI<->Flash connection on CSID 0
    configure_spi(soc_ctrl);
    // Set CSID
    spi_set_csid(&spi, 0);
    // Power up flash
    power_up_flash();
    // Set QE bit
    set_QE_bit(); // TO DO

    return 1;
}



uint8_t w25q128jw_read_standard(uint32_t addr, uint8_t *data, uint32_t length) {
    // Set RX watermark to length
    spi_set_rx_watermark(&spi, length);

    uint32_t read_byte_cmd = ((REVERT_24b_ADDR(addr & 0x00ffffff) << 8) | FC_RD);
    spi_write_word(&spi, read_byte_cmd);
    spi_wait_for_ready(&spi);

    const uint32_t cmd_read_1 = spi_create_command((spi_command_t){
        .len        = 3,                 // 4 Bytes
        .csaat      = true,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_read_1);
    spi_wait_for_ready(&spi);

    const uint32_t cmd_read_2 = spi_create_command((spi_command_t){
        .len        = length-1,          // len bytes
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirRxOnly      // Read only
    });
    spi_set_command(&spi, cmd_read_2);
    spi_wait_for_ready(&spi);


    // Read back
    spi_wait_for_rx_watermark(&spi);

    // Read data from SPI RX FIFO
    for (int i = 0; i < length; i+4) {
        spi_read_word(&spi, &data[i]); // Writes a full word
    }

    return 1; // Success
}

uint8_t w25q128jw_write_standard(uint32_t addr, uint8_t *data, uint32_t length) {
    // Taking care of misalligned start address
    if (addr % 256 != 0) {
        uint8_t tmp_len = 256 - (addr % 256);
        page_write(addr, data, tmp_len);
        addr += tmp_len;
        data += tmp_len;
        length -= tmp_len;
    }

    int flag = 1;
    while (flag) {
        if (length > 256) {
            page_write(addr, data, 256);
            addr += 256;
            data += 256;
            length -= 256;
        } else {
            page_write(addr, data, length);
            flag = 0;
        }
    }

    return 1; // Success
}



void w25q128jw_reset() {
    wait_flash();
    reset_flash();
    wait_flash();
}

void w25q128jw_reset_force() {
    reset_flash();    
    wait_flash();
}

void w25q128jw_power_down() {
    spi_write_word(&spi, FC_PD);
    spi_wait_for_ready(&spi);
    const uint32_t cmd_power_down = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_power_down);
    spi_wait_for_ready(&spi);
}








// ----------------
// HELPER FUNCTIONS
// ----------------

static void power_up_flash() {
    spi_write_word(&spi, FC_RPD);
    spi_wait_for_ready(&spi);
    const uint32_t cmd_powerup = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_powerup);
    spi_wait_for_ready(&spi);
}

static void set_QE_bit() {
    // TODO
}

static void configure_spi(soc_ctrl_t soc_ctrl) {
    // Configure SPI clock
    // SPI clk freq = 1/2 core clk freq when clk_div = 0
    // SPI_CLK = CORE_CLK/(2 + 2 * CLK_DIV) <= CLK_MAX => CLK_DIV > (CORE_CLK/CLK_MAX - 2)/2
    uint32_t core_clk = soc_ctrl_get_frequency(&soc_ctrl);
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
    spi_set_configopts(&spi, 0, chip_cfg);
}

// Checking BUSY bit status. Not checking SUS bit status.
static void wait_flash() {
    spi_set_rx_watermark(&spi,1);
    bool flash_busy = true;
    uint8_t flash_resp[4] = {0xff,0xff,0xff,0xff};

    while(flash_busy){
        uint32_t flash_cmd = FC_RSR1; // [CMD] Read status register 1
        spi_write_word(&spi, flash_cmd); // Push TX buffer
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
        spi_set_command(&spi, spi_status_cmd);
        spi_wait_for_ready(&spi);
        spi_set_command(&spi, spi_status_read_cmd);
        spi_wait_for_ready(&spi);
        spi_wait_for_rx_watermark(&spi);
        spi_read_word(&spi, &flash_resp[0]);
        if ((flash_resp[0] & 0x01) == 0) flash_busy = false;
    }
}

static void reset_flash() {
    spi_write_word(&spi, FC_ERESET);
    spi_write_word(&spi, FC_RESET);
    spi_wait_for_ready(&spi);

    const uint32_t cmd_reset_enable = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_reset_enable);
    spi_wait_for_ready(&spi);
    const uint32_t cmd_reset = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_reset);
    spi_wait_for_ready(&spi);
}

static void page_write(uint32_t addr, uint8_t *data, uint32_t length) {
    // Write enable
    spi_write_word(&spi, FC_WE);
    const uint32_t cmd_write_en = spi_create_command((spi_command_t){
        .len        = 0,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi, cmd_write_en);
    spi_wait_for_ready(&spi);

    // Write command
    const uint32_t write_byte_cmd = ((REVERT_24b_ADDR(addr & 0x00ffffff) << 8) | FC_PP);
    spi_write_word(&spi, write_byte_cmd);
    const uint32_t cmd_write = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = true,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi, cmd_write);
    spi_wait_for_ready(&spi);

    // Place data in TX FIFO
    uint32_t *data_32bit = (uint32_t *)data;
    for (int i = 0; i < length/4; i++) {
        spi_write_word(&spi, data_32bit[i]);
    }
    if (length % 4 != 0) {
        uint32_t last_word = 0;
        for (int i = length % 4; i > 0; i--) {
            last_word |= data[length - i] << (8*(i-1));
        }
        spi_write_word(&spi, last_word);
    }

    // Write
    const uint32_t cmd_write_2 = spi_create_command((spi_command_t){
        .len        = (length*4)-1,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi, cmd_write);
    spi_wait_for_ready(&spi);

    // Wait for flash to be ready again
    wait_flash();
}



// Non-weak definition of "fast interrupt controller irq for spi flash"
void fic_irq_spi_flash(void) {
    // Disable SPI interrupts
    spi_enable_evt_intr(&spi, false);
    spi_enable_rxwm_intr(&spi, false);
    spi_intr_flag = 1;
}