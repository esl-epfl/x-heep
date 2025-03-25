#include "ST7789_driver.h"
#include "soc_ctrl_structs.h"

#include "spi_host_regs.h"
#include "gpio.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define FC_RSR1    0x05 /** Read Status Register 1 */

#include "core_v_mini_mcu.h"

//#define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
    
spi_host_t *ST7789_spi_LCD;

/*
 * Private function definitions
*/
static void ST7789_configure_spi(spi_t* spi);

 /*
  * Private Function prototypes
  */

static void ST7789_configure_spi(spi_t* spi) {
    // Configure SPI clock
    uint32_t core_clk = soc_ctrl_peri->SYSTEM_FREQUENCY_HZ;
    uint16_t clk_div = 0;
    if(CLK_MAX_HZ < core_clk/2){
        clk_div = (core_clk/(CLK_MAX_HZ) - 2)/2; // The value is truncated
        if (core_clk/(2 + 2 * clk_div) > CLK_MAX_HZ) clk_div += 1; // Adjust if the truncation was not 0
    }
    //PRINTF("SPI CLK DIV: %d\n", clk_div);
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

 /*
  * Public Function prototypes
  */

// WE DON'T NEED TO SEND A POINTER
void ST7789_gpio_init()
{
    gpio_cfg_t cfg_out = {
        .pin = GPIO_SPI_DC,
        .mode = GpioModeOutPushPull
    };
    gpio_config(cfg_out);
}




uint8_t ST7789_spi_init(spi_t* spi){
    //ST7789_spi_LCD.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    // Enable SPI host device
    printf("[INIT] Calling spi_set_enable(spi, true)...\n");
    spi_return_flags_e flags = spi_set_enable(spi, true);
    if (flags != SPI_FLAG_OK) {
        printf("[ERROR] spi_set_enable failed: 0x%X\n", flags);
    }

    uint32_t ctrl = SPI_HW(spi)->CONTROL;
    printf("[DEBUG] CONTROL after enable = 0x%08X\n", ctrl);

    // Enable SPI output
    spi_output_enable(spi, true);

    // Configure SPI connection on CSID 0
    ST7789_configure_spi(spi);

    // Set CSID
    spi_set_csid(spi, 0);
    ST7789_milli_delay(100);

    return 0;
}

// PROBLEMATIC
uint8_t ST7789_display_init(spi_t* spi)
{
    printf("Hello1");
    fflush(stdout);
    gpio_write(GPIO_SPI_DC, DC_COMMAND);
    printf("Hello2");
    fflush(stdout);

    ST7789_spi_write_command(spi, ST7789_SWRESET);	// 1: Software reset, no args, w/delay: delay(150)
    //PRINTF("ST7789_SWRESET 0x01\n");
    ST7789_milli_delay(150);
    printf("Hello3");
    fflush(stdout);

    ST7789_spi_write_command(spi, ST7789_SLPOUT);	// 2: Out of sleep mode, no args, w/delay: delay(500)
    //PRINTF("ST7789_SLPOUT 0x11\n");
    ST7789_milli_delay(500);
    printf("Hello1");
    fflush(stdout);

    ST7789_spi_write_command(spi, ST7789_COLMOD);	// 3: Set color mode, 1 arg, delay: delay(10)
    //PRINTF("ST7789_COLMOD 0x3A\n");
    ST7789_milli_delay(10);
    printf("Hello2");
    fflush(stdout);
    ST7789_spi_write_data(spi, ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT);	// 65K color, 16-bit color
    //PRINTF("ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT = 0x55\n");
    ST7789_milli_delay(150);

    ST7789_milli_delay(10);
    ST7789_spi_write_command(spi, ST7789_MADCTL);	// 4: Memory access ctrl (directions), 1 arg:
    //PRINTF("ST7789_MADCTL 0x36\n");

    ST7789_milli_delay(10);
    ST7789_spi_write_data(spi, ST7789_MADCTL_RGB);	// RGB Color
    //PRINTF("ST7789_MADCTL_RGB = 0x00\n");

    ST7789_milli_delay(10);
    ST7789_spi_write_command(spi, ST7789_CASET);	// 5: Column addr set, 4 args, no delay:
    //PRINTF("ST7789_CASET 0x2A\n");
    ST7789_milli_delay(10);
    ST7789_spi_write_data(spi, ST7789_240x240_XSTART >> 8);
    ST7789_spi_write_data(spi, ST7789_240x240_XSTART);
    ST7789_spi_write_data(spi, (ST7789_TFTWIDTH + ST7789_240x240_XSTART) >> 8);
    ST7789_spi_write_data(spi, (ST7789_TFTWIDTH + ST7789_240x240_XSTART));
    printf("Allright");
    fflush(stdout);

    ST7789_milli_delay(10);
    ST7789_spi_write_command(spi, ST7789_RASET);	// 6: Row addr set, 4 args, no delay:
    //PRINTF("ST7789_RASET 0x2B\n");
    ST7789_milli_delay(10);
    ST7789_spi_write_data(spi, ST7789_240x240_YSTART >> 8);
    ST7789_spi_write_data(spi, ST7789_240x240_YSTART);
    ST7789_spi_write_data(spi, (ST7789_TFTHEIGHT + ST7789_240x240_YSTART) >> 8);
    ST7789_spi_write_data(spi, (ST7789_TFTHEIGHT + ST7789_240x240_YSTART));

    ST7789_spi_write_command(spi, ST7789_INVON);	// 5: Inversion ON (but why?) delay(10)
    //PRINTF("ST7789_INVON 0x21\n");
    ST7789_milli_delay(10);

    ST7789_spi_write_command(spi, ST7789_NORON);	// 6: Normal display on, no args, w/delay: delay(10)
    //PRINTF("ST7789_NORON 0x13\n");
    ST7789_milli_delay(10);

    ST7789_spi_write_command(spi, ST7789_DISPON);	// 7: Main screen turn on, no args, w/delay: delay(500)
    //PRINTF("ST7789_DISPON 0x29\n");
    ST7789_milli_delay(500);
    
    //PRINTF("Display Initialization Done \n");
    return 0;
}

// USELESS?
spi_host_t* ST7789_get_spi_host(spi_t* spi)
{
    return spi;
}

void ST7789_set_adress_window(spi_t* spi, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    // Set column address with driver for screen ST7789V
    ST7789_spi_write_command(&spi, ST7789_CASET);

    //(x1<<8 | x1>>8)
    //spi_write_data(x1 >> 8);

    ST7789_milli_delay(10);
    ST7789_spi_write_data_2B(spi, x1);

    //spi_write_data(x2 >> 8);
    ST7789_milli_delay(10);
    ST7789_spi_write_data_2B(spi, x2);

    // Set row address
    ST7789_milli_delay(10);
    ST7789_spi_write_command(spi, ST7789_RASET);

    //spi_write_data(y1 >> 8);
    ST7789_milli_delay(10);
    ST7789_spi_write_data_2B(spi, y1);

    //spi_write_data(y2 >> 8);
    ST7789_milli_delay(10);
    ST7789_spi_write_data_2B(spi, y2);

    // Write to RAM
    ST7789_milli_delay(10);
    ST7789_spi_write_command(spi, 0x2C);
}

// PROBLEMATIC
void ST7789_spi_write_command(spi_t* spi, uint8_t command)
{
    gpio_write(GPIO_SPI_DC, DC_COMMAND);
    spi_return_flags_e flags;

    flags =spi_write_word(spi, command);
    if (flags != SPI_FLAG_OK) {
        printf("FAILED TO WRITE WORD: 0x%X\n", flags);
        return;
    } else if (flags == SPI_FLAG_OK) {
        printf("WRITE WORD SUCCESS\n");
        
    }
    ST7789_milli_delay(10);
    printf("INSIDE SPI WRITE");
    fflush(stdout);
    //PRINTF("SPI HOST ADDRESS = %x\n", ST7789_spi_LCD.base_addr);
    //PRINTF("SPI WRITE COMMAND = %x\n", command);
    //CRASHES HERE
    
    // Set up segment parameters -> send command and address
    const uint32_t cmd = spi_create_command((spi_command_t){
        .len        = 1,                 // 4 Bytes WAS SET TO 0 BEFORE
        .csaat      = false,             // Command not finished
        .speed      = SPI_SPEED_STANDARD, // Single speed
        .direction  = SPI_DIR_TX_ONLY     // Write only
    });
    ST7789_milli_delay(500);
    // Load segment parameters to COMMAND register
    fflush(stdout);
    spi_wait_for_idle(spi); 
    printf("AFTER WAIT FOR IDLE\n");
    uint32_t control = SPI_HW(spi)->CONTROL;
    uint32_t status = SPI_HW(spi)->STATUS;
    printf("[DEBUG] CONTROL = 0x%08X, STATUS = 0x%08X\n", control, status);
    printf("[DEBUG] CMD_REG = 0x%08X\n", cmd);


    flags=spi_set_command(spi, cmd);
    
    if (flags == SPI_FLAG_NULL_PTR) {
        printf("[FAIL] spi_set_command: NULL pointer\n");
        return;
    } else if (flags & SPI_FLAG_SPEED_INVALID) {
        printf("[FAIL] spi_set_command: Invalid speed/direction\n");
        return;
    } else if (flags & SPI_FLAG_NOT_READY) {
        printf("[FAIL] spi_set_command: SPI not ready (timeout)\n");
        return;
    } else if (flags != SPI_FLAG_OK) {
        printf("[FAIL] spi_set_command: Unknown error: 0x%X\n", flags);
        return;
    } else {
        printf("[OK] spi_set_command succeeded\n");
    }
    flags=spi_wait_for_ready(spi);

    printf("AFTER WAIT FOR READY");
    fflush(stdout);
}


void ST7789_spi_write_data(spi_t* spi, uint8_t data)
{
    gpio_write(GPIO_SPI_DC, DC_DATA);

    spi_return_flags_e flags = spi_write_word(spi, data);
    if (flags != SPI_FLAG_OK) {
        printf("FAILED TO WRITE DATA: 0x%X\n", flags);
        return;
    }

    const uint32_t cmd = spi_create_command((spi_command_t){
        .len        = 1, // ✅ not 0!
        .csaat      = false,
        .speed      = SPI_SPEED_STANDARD,
        .direction  = SPI_DIR_TX_ONLY
    });

    flags = spi_set_command(spi, cmd);
    if (flags != SPI_FLAG_OK) {
        printf("FAILED TO SET COMMAND (DATA): 0x%X\n", flags);
        return;
    }

    flags = spi_wait_for_ready(spi);
    if (flags != SPI_FLAG_OK) {
        printf("DATA TRANSFER TIMED OUT: 0x%X\n", flags);
    }
}


void ST7789_spi_write_data_2B(spi_t* spi, uint16_t data)
{
    gpio_write(GPIO_SPI_DC, DC_DATA);

    // Swap endianness
    data = (data >> 8) | (data << 8);

    spi_return_flags_e flags = spi_write_word(spi, data);
    if (flags != SPI_FLAG_OK) {
        printf("FAILED TO WRITE 2B DATA: 0x%X\n", flags);
        return;
    }

    const uint32_t cmd = spi_create_command((spi_command_t){
        .len        = 1,
        .csaat      = false,
        .speed      = SPI_SPEED_STANDARD,
        .direction  = SPI_DIR_TX_ONLY
    });

    flags = spi_set_command(spi, cmd);
    if (flags != SPI_FLAG_OK) {
        printf("FAILED TO SET COMMAND (2B DATA): 0x%X\n", flags);
        return;
    }

    flags = spi_wait_for_ready(spi);
    if (flags != SPI_FLAG_OK) {
        printf("2B DATA TRANSFER TIMED OUT: 0x%X\n", flags);
    }
}


uint32_t ST7789_test_write_pixel(spi_t* spi, uint16_t x, uint16_t y, uint16_t color) {
    ST7789_set_adress_window(spi, x, y, x+1, y+1);
    ST7789_spi_write_data(spi, color >> 8);
    ST7789_spi_write_data(spi, color);
    // Consider returning a value if needed.
}

void ST7789_test_write_multi_unicolor(spi_t* spi, uint16_t color, uint32_t num)
{
    while (num > 0)
    {
        ST7789_spi_write_data_2B(spi, color);
        num--;
    }    
}

void ST7789_test_fill_screen(spi_t* spi, uint16_t color)
{
    ST7789_set_adress_window(spi, 0, 0, (uint16_t) ST7789_TFTWIDTH, (uint16_t) ST7789_TFTHEIGHT);
    ST7789_test_write_multi_unicolor(spi, color, (uint32_t) ST7789_TFTWIDTH * ST7789_TFTHEIGHT);
}

void ST7789_fill_picture(spi_t* spi, uint16_t* colors)
{
    ST7789_set_adress_window(spi, 0, 0, (uint16_t) ST7789_TFTWIDTH, (uint16_t) ST7789_TFTHEIGHT);

    int i = 0;
    for (i = 0; i < (ST7789_TFTWIDTH)/2; i++) {
        for (int repeat_row = 0; repeat_row < 2; repeat_row++) {
            for (int j = 0; j < ST7789_TFTHEIGHT/2; j++) {
                ST7789_spi_write_data_2B(spi, colors[i * ST7789_TFTWIDTH/2 + j]);
                ST7789_spi_write_data_2B(spi, colors[i * ST7789_TFTWIDTH/2 + j]);
            }
        }
    }
    //PRINTF(" i = %d\n", i);
}

void ST7789_test_fill_picture_with_shift(spi_t* spi, uint16_t* colors, uint8_t verticalShift, uint8_t horizontalShift)
{
    ST7789_set_adress_window(spi, 0, 0, (uint16_t) ST7789_TFTWIDTH, (uint16_t) ST7789_TFTHEIGHT);

    int i = 0;
    for (i = 0; i < (ST7789_TFTWIDTH)/2; i++) {
        for (int repeat_row = 0; repeat_row < 2; repeat_row++) {
            for (int j = 0; j < ST7789_TFTHEIGHT/2; j++) {
                ST7789_spi_write_data_2B(spi, colors[i * ST7789_TFTWIDTH/2 + j + horizontalShift + verticalShift * ST7789_TFTWIDTH/2]);
                ST7789_spi_write_data_2B(spi, colors[i * ST7789_TFTWIDTH/2 + j + horizontalShift + verticalShift * ST7789_TFTWIDTH/2]);
            }
        }
    }
}

void ST7789_milli_delay(int n_milli_seconds)
{
    // Converting time into cycles
    // Factor found for ZYNQ-Z2 through experimentation
    int cycles = 4 * 1000 * n_milli_seconds;
 
    for (int i = 0; i < cycles; i++) asm volatile("nop;");
}
