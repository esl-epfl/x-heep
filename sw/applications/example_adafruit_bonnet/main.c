#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "filtered_array.h"
#include "spi_host_regs.h"

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
#define SHIFTED_ESL_WITH_BUTTONS 1
#define STATIC_ESL_LOGO 0
#define CHANGING_COLOR 0
#define FILL_SCREEN_PIXEL_BY_PIXEL 0

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



void display_init();



int main(int argc, char *argv[]) {

    buttonsInit();
    display_init();

    uint16_t x = 0;
    uint16_t y = 0;

    uint16_t color = 0xA000;

    ST7789_test_fill_screen(0x8000);

#if SHIFTED_ESL_WITH_BUTTONS
    int shift_x =8;
    int shift_y =8;

    int a = 2;

    while(1)
    {
        readButtons();
        for(uint8_t i = 0; i<BUTTON_COUNT; i++)
        {
            if(button_posedge[i])
            {
                PRINTF("Button %s pressed\n", ButtonNames[i]);
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
        
        ST7789_test_fill_picture_with_shift(&filtered_array,shift_y,shift_x);
        //PRINTF("LOOP FINISHED\n");
    }
#endif    

#if STATIC_ESL_LOGO
    while(1)
    {
        ST7789_fill_picture(&filtered_array);
        PRINTF("LOOP FINISHED\n");
        ST7789_milli_delay(500);
    }
#endif

#if CHANGING_COLOR
    while(1)
    {
        ST7789_test_fill_screen(color);
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

        while (x<240)
        {
            PRINTF("x: %d\n", x);

            while (y<240)
            {
                PRINTF("y: %d\n", y);
                ST7789_test_write_pixel(x, y, color);
                ST7789_milli_delay(1);
                y++;
            }
            y=30;
            x++;
        }
    }
#endif    
}

void display_init()
{
    PRINTF("DISPLAY INIT\n");

    ST7789_gpio_init();

    PRINTF("DISPLAY SPI INIT\n");
    

    ST7789_spi_init();
    ST7789_display_init();

    ST7789_test_fill_screen(0x8000);

}