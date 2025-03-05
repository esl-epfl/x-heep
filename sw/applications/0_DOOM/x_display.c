
#include "x_display.h"
#include "i_video.h"
#include "ST7789_driver.h"

#define PRINTF(fmt, ...)    PRINTF(fmt, ## __VA_ARGS__)

//uint16_t ST7789_screenBuffer[ST7789_TFTHEIGHT*ST7789_TFTWIDTH];

//public function definitions

void X_Display_init(void)
{
    PRINTF("DISPLAY INIT\n");

    ST7789_gpio_init();

    PRINTF("DISPLAY SPI INIT\n");
    

    ST7789_spi_init();
    ST7789_display_init();

    ST7789_test_fill_screen(0xFFFF);
}

/* X-HEEP COMMENT to reduce size
void X_Display_Fill_ST7789_Buffer(void)
{
    // at the moment the game is hardcoded for a 320*200 screen
    // the screen buffer has uint8_t colors that need to be translated from palette
    // to RGB565
    uint16_t rgb565 = 0;
    uint8_t color;
    //fill the ST7789 Screen Buffer
    for (int i = 0; i < 320; i++)
    {
        if (i>=240)
        {
            continue;
        }
        
        for (int j = 0; j < 200; j++)
        {
            color = I_VideoBuffer[j*ST7789_TFTWIDTH+i];
            rgb565 = I_GetRGB565FromPaletteIndex(color);
            ST7789_screenBuffer[j*ST7789_TFTWIDTH+i] = rgb565;
        }
    }
}

// takes video buffer and fills the ST7789 screen buffer
// then sends it to ST7789 display

void X_Display_Draw_ScreenBuffer(void)
{
    X_Display_Fill_ST7789_Buffer();
    ST7789_fill_picture(ST7789_screenBuffer);
}
*/

//Function written to remove unnecessary arrays
//Buffer for ST7789 is completely removed
//Pixel values for the screen are converted from the palette on the fly.
void X_Display_Draw_Screen_200x200(void)
{
    ST7789_set_adress_window(0, 0, 200, 200);

    //Code from X_Display_Fill_ST7789_Buffer
    // at the moment the game is hardcoded for a 320*200 screen
    // the screen buffer has uint8_t colors that need to be translated from palette
    // to RGB565
    uint16_t rgb565 = 0;
    uint8_t color;
    //fill the ST7789 Screen Buffer
    for (int i = 0; i < 200; i++)
    {
        
        
        for (int j = 0; j < 200; j++)
        {
            color = I_VideoBuffer[i*200+j];
            rgb565 = I_GetRGB565FromPaletteIndex(color);
            ST7789_spi_write_data_2B(rgb565);
        }
    }
}
