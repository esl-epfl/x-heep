/**
 * @file main.c
 * @brief Simple spi write example
 *
 * Simple example that read 8 words from memory and checks
 * that the data is correct.
 *
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "soc_ctrl.h"
#include "spi_host.h"
#include "fast_intr_ctrl.h"
#include "fast_intr_ctrl_regs.h"
#include "x-heep.h"
#include "w25q128jw.h"



#define FLASH_ADDR 0x00008500 // 256B data alignment
uint32_t flash_original[8] = {0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef};
uint32_t flash_retreived[8] = {};

int main(int argc, char *argv[]) {
    printf("BSP write test\n\r");
    
    #ifndef USE_SPI_FLASH
    printf("Currently only support for USE_SPI_FLASH\n\r");
    return EXIT_FAILURE;
    #endif // USE_SPI_FLASH


    // Write data to flash and read it back
    if (w25q128jw_init() == 0) return EXIT_FAILURE;
    if (w25q128jw_write_standard(FLASH_ADDR, flash_original, 32) == 0) return EXIT_FAILURE;
    if (w25q128jw_read_standard(FLASH_ADDR, flash_retreived, 32) == 0) return EXIT_FAILURE;


    // Check results
    printf("ram vs flash...\n\r");
    int i;
    uint32_t errors = 0;
    uint32_t count = 0;
    for (i = 0; i < 32; i++) {
        if(((uint8_t*)flash_original)[i] != ((uint8_t*)flash_retreived)[i]) {
            printf("@%08x-@%08x : %02d\t!=\t%02d\n\r",
                &((uint8_t*)flash_original)[i], &((uint8_t*)flash_retreived)[i],
                ((uint8_t*)flash_original)[i], ((uint8_t*)flash_retreived)[i]);
            errors++;
        }
        count++;
    }

    if (errors == 0) {
        printf("success! (Data units checked: %d)\n\r", count);
    } else {
        printf("Failure, %d errors! (Out of %d)\n\r", errors, count);
        return EXIT_FAILURE;
    }
}