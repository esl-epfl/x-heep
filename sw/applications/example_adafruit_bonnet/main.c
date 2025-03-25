#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "filtered_array.h"
#include "spi_host_regs.h"
//ADDED
#include "spi_sdk.h"

#include "x-heep.h"
#include "soc_ctrl_structs.h"
#include "spi_host.h"

#include "gpio.h" 

#include "ST7789_driver.h"
#include "buttons.h"

#include "core_v_mini_mcu.h"

//Buttons are read by polling. The screen takes a while to update.
//This means that buttons need to be pressed for a while to be detected.

//WHICH TEST TO RUN
#define PRINTF_IN_FPGA  1
#define SHIFTED_ESL_WITH_BUTTONS 0
#define STATIC_ESL_LOGO 1
#define CHANGING_COLOR 0
#define FILL_SCREEN_PIXEL_BY_PIXEL 0
#define FLASH_MAX_FREQ (133*1000*1000) 

#ifdef TARGET_IS_FPGA
    #define USE_SPI_FLASH
#endif

//PIN ASSIGNMENTS

// V8  = Raspberri PI 19    = DC
// H15 = ARDUINO SPI 3      = CLK
// T12 = ARDUINO SPI 4      = MOSI
// F16 = ARDUINO SPI 5      = CS

// U8  = RASP_PI_PIN 15 = GPIO UP      
// V7  = RASP_PI_PIN 13 = GPIO DOWN
// U7  = RASP_PI_PIN 11 = GPIO LEFT
// V6  = RASP_PI_PIN 8  = GPIO RIGHT
// U13 = AR2 = GPIO A
// V13 = AR3 = GPIO B



typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    A,
    B,
    BUTTON_COUNT
} Button;

const char* ButtonNames[BUTTON_COUNT] = {
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "A",
    "B"
};


/* By default, PRINTFs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#define TARGET_PYNQ_Z2 1

#if TARGET_SIM && PRINTF_IN_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

// Change prototype to accept the SPI pointer
void display_init(spi_t* spi);

int main(int argc, char *argv[]) {
    //ADDED

    // Create our slave (flash device) with its specifications
    spi_slave_t slave = SPI_SLAVE(0, FLASH_MAX_FREQ);

    // Initialize the spi device that is CONNECTED to the flash with our slave
    //#ifdef USE_SPI_FLASH
    //spi_t spi = spi_init(SPI_IDX_FLASH, slave);
    //#else
    spi_t spi = spi_init(SPI_IDX_HOST, slave);
    //#endif

    // Check if initialization succeeded
    if (!spi.init) {
        PRINTF("\nFailed to initialize spi\n");
        return EXIT_FAILURE;
    }
    //spi_set_enable(&spi, true);  // ✅ ENABLE SPI after init


    PRINTF("\nSPI initialized\n");

    buttonsInit();
    printf("PROG RUNNING\n");
    printf("HEY\n");
    // Pass the address of spi to display_init
    display_init(&spi);

    uint16_t x = 0;
    uint16_t y = 0;

    uint16_t color = 0xA000;

   //ST7789_test_fill_screen(&spi,0x8000);

#if SHIFTED_ESL_WITH_BUTTONS
    int shift_x = 8;
    int shift_y = 8;
    int a = 2;
    printf("Entering while(1) loop...\n");
    fflush(stdout);
    
    while(1)
    {
        readButtons();
        for(uint8_t i = 0; i < BUTTON_COUNT; i++)
        {
            if(button_posedge[i])
            {
                printf("Button %s pressed\n", ButtonNames[i]);
                fflush(stdout);
                switch (i)
                {
                case UP:
                    shift_y += a;
                    break;
                case DOWN:
                    shift_y -= a;
                    break;
                case LEFT:
                    shift_x += a;
                    break;
                case RIGHT:
                    shift_x -= a;
                    break;        
                default:
                    break;
                }
            } else if (button_negedge[i])
            {
                PRINTF("Button %s released\n", ButtonNames[i]);
            }
        }
        // Pass the SPI pointer and the filtered array correctly
        ST7789_test_fill_picture_with_shift(&spi, filtered_array, shift_y, shift_x);
    }
#endif    

#if STATIC_ESL_LOGO
    while(1)
    {
        //printf("Inside loop static esl");
        ST7789_fill_picture(&spi, filtered_array);
        PRINTF("LOOP FINISHED\n");
        ST7789_milli_delay(500);
    }
#endif

#if CHANGING_COLOR
    while(1)
    {
        ST7789_test_fill_screen(&spi, color);
        PRINTF("Fill with color: %d\n", color);
        color += 0x0003;
        PRINTF("LOOP FINISHED\n");
        ST7789_milli_delay(500);
    }
#endif

#if FILL_SCREEN_PIXEL_BY_PIXEL
    while(1){
        PRINTF("RESTART with color: %d\n", color);
        color += 0x0000;
        x = 140;
        y = 30;

        while (x < 240)
        {
            PRINTF("x: %d\n", x);
            while (y < 240)
            {
                PRINTF("y: %d\n", y);
                ST7789_test_write_pixel(&spi, x, y, color);
                ST7789_milli_delay(1);
                y++;
            }
            y = 30;
            x++;
        }
    }
#endif    
}

void display_init(spi_t* spi)
{
    printf("DISPLAY INIT\n");

    // ST7789_gpio_init takes no arguments
    ST7789_gpio_init();

    printf("DISPLAY SPI INIT\n");
    
    ST7789_spi_init(spi);
    printf("ST7789_spi_init\n");
    //PROGRAM CRASHES HERE
    ST7789_display_init(spi);
    printf("ST7789_display_init\n");

    //ST7789_test_fill_screen(0x8000);
}
