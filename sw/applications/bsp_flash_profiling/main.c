/**
 * @file main.c
 * @brief Simple spi write example using BSP
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
#include "data_array.bin"

// Flash address to write to (different from the address where the buffer is stored)
#define FLASH_ADDR 0x00008500

// Len in bytes of the test buffer, 1kB
#define MAX_TEST_BUF_LEN 1024

// End buffer
uint32_t flash_data_32[MAX_TEST_BUF_LEN/4];

// Test buffer
// The test buffer is contained in data_array.bin and is 1kB long


int main(int argc, char *argv[]) {
    printf("BSP profiling\n\r");
    error_codes_t status;
    uint32_t errors = 0;
    uint32_t flag = 0;

    // Init SPI host and SPI<->Flash bridge parameters 
    status = w25q128jw_init();
    if (status != FLASH_OK) return EXIT_FAILURE;

    uint8_t *test_buffer = flash_original_32;
    uint8_t *flash_data = flash_data_32;

    printf("Start profile routine...\n\r");
    for (int i = 1; i <= MAX_TEST_BUF_LEN; i++) {
        printf("\rIteration %u ", i);

        // Write to flash memory at specific address
        status = w25q128jw_write_standard(FLASH_ADDR, test_buffer, i);
        if (status != FLASH_OK) return EXIT_FAILURE;

        // Read from flash memory at the same address
        status = w25q128jw_read_standard(FLASH_ADDR, flash_data, i);
        if (status != FLASH_OK) return EXIT_FAILURE;


        // Check if what we read is correct (i.e. flash_original == flash_data)
        for (int j=0; j < i; j++) {
            if(flash_data[j] != test_buffer[j]) {
                printf("iteration %u, index@%x : %x != %x(ref)\n\r", i, j, flash_data[j], test_buffer[j]);
                errors++;
            }
        }
        if (errors > 0) flag = 1;
        errors = 0;
    }

    // Exit status based on errors found
    if (flag == 0) {
        printf("success!\n\r");
        return EXIT_SUCCESS;
    } else {
        printf("failure!\n\r", errors);
        return EXIT_FAILURE;
    }
}