/**
 * @file main.c
 * @brief Simple spi read example
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


uint32_t flash_data[8];
uint32_t flash_original[8] = {0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef};

int main(int argc, char *argv[]) {
    printf("BSP read test\n\r");

    #ifndef USE_SPI_FLASH
    printf("Currently only support for USE_SPI_FLASH\n\r");
    return EXIT_FAILURE;
    #endif // USE_SPI_FLASH

    w25q128jw_init();
    w25q128jw_read_standard(flash_original, flash_data, 32);

    printf("flash vs ram...\n\r");
    uint32_t errors = 0;
    uint32_t* ram_ptr = flash_original;
    for (int i=0; i<8; i++) {
        if(flash_data[i] != *ram_ptr) {
            printf("@%x : %x != %x\n\r", ram_ptr, flash_data[i], *ram_ptr);
            errors++;
        }
        ram_ptr++;
    }

    if (errors == 0) {
        printf("success!\n\r");
        return EXIT_SUCCESS;
    } else {
        printf("failure, %d errors!\n\r", errors);
        return EXIT_FAILURE;
    }
}

