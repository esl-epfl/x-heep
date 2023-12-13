/**
 * @file main.c
 * @brief Simple spi write example using BSP
 *
 * Simple example that writes a 1kB buffer to flash memory at a specific address
 * and then read it back to check if the data was written correctly.
 * By default the error checks after every operation are disabled, in order to
 * execute all four configurations of the test (standard, standard_dma, quad, quad_dma)
 * even if one fails.
 *
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "x-heep.h"
#include "w25q128jw.h"

/* By default, PRINTFs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    PRINTF(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    PRINTF(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#ifdef TARGET_PYNQ_Z2
    #define USE_SPI_FLASH
#endif


// Start buffers (the original data)
#include "buffer.h"
// End buffer (where what is read is stored)
uint32_t flash_data[256];

/*
 * Assign the test buffer to the buffer to write to flash.
 * The buffer is defined in the file buffer.h. As multiple buffers can
 * be defined, this is userful to pick the right one.
 * Also the length is specified, to test different length cases. In any case
 * length <= test_buffer length. 
*/
#define TEST_BUFFER flash_original_1024B
#define LENGTH 1024

// Test functions
w25q_error_codes_t test_read(uint32_t *test_buffer, uint32_t len);
w25q_error_codes_t test_read_dma(uint32_t *test_buffer, uint32_t len);
w25q_error_codes_t test_read_quad(uint32_t *test_buffer, uint32_t len);
w25q_error_codes_t test_read_quad_dma(uint32_t *test_buffer, uint32_t len);

// Check function
void check_result(uint32_t *test_buffer, uint32_t len);

// Global flag to keep track of errors
uint32_t global_errors = 0;
// Define global status variable
w25q_error_codes_t global_status;

int main(int argc, char *argv[]) {
    PRINTF("BSP read test\n\r");

    // Pick the correct spi device based on simulation type
    spi_host_t spi;
    #ifndef USE_SPI_FLASH
    spi.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    #else
    spi.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    #endif

    // Init SPI host and SPI<->Flash bridge parameters 
    global_status = w25q128jw_init(spi);
    if (global_status != FLASH_OK) return EXIT_FAILURE;

    // Test simple read
    PRINTF("Testing simple read...\n\r");
    global_status = test_read(TEST_BUFFER, LENGTH);
    // if (global_status != FLASH_OK) return EXIT_FAILURE;

    PRINTF("Testing simple read with DMA...\n\r");
    // Test simple read with DMA
    global_status = test_read_dma(TEST_BUFFER, LENGTH);
    // if (global_status != FLASH_OK) return EXIT_FAILURE;

    PRINTF("Testing quad read...\n\r");
    // Test quad read
    global_status = test_read_quad(TEST_BUFFER, LENGTH);
    // if (global_status != FLASH_OK) return EXIT_FAILURE;

    PRINTF("Testing quad read with DMA...\n\r");
    // Test quad read with DMA
    global_status = test_read_quad_dma(TEST_BUFFER, LENGTH);
    // if (global_status != FLASH_OK) return EXIT_FAILURE;

    PRINTF("\n--------TEST FINISHED--------\n");
    if (global_errors == 0) {
        PRINTF("All tests passed!\n\r");
        return EXIT_SUCCESS;
    } else {
        PRINTF("Some tests failed!\n\r");
        return EXIT_FAILURE;
    }
    
}

w25q_error_codes_t test_read(uint32_t *test_buffer, uint32_t len) {
    // Read from flash memory at the same address
    global_status = w25q128jw_read(test_buffer, flash_data, len);
    if (global_status != FLASH_OK) return FLASH_ERROR;

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    check_result(test_buffer, len);

    return FLASH_OK;
}

w25q_error_codes_t test_read_dma(uint32_t *test_buffer, uint32_t len) {
    // Read from flash memory at the same address
    global_status = w25q128jw_read(test_buffer, flash_data, len);
    if (global_status != FLASH_OK) return FLASH_ERROR;

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    check_result(test_buffer, len);

    return FLASH_OK;
}

w25q_error_codes_t test_read_quad(uint32_t *test_buffer, uint32_t len) {
    // Read from flash memory at the same address
    global_status = w25q128jw_read(test_buffer, flash_data, len);
    if (global_status != FLASH_OK) return FLASH_ERROR;

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    check_result(test_buffer, len);

    return FLASH_OK;
}

w25q_error_codes_t test_read_quad_dma(uint32_t *test_buffer, uint32_t len) {
    // Read from flash memory at the same address
    global_status = w25q128jw_read(test_buffer, flash_data, len);
    if (global_status != FLASH_OK) return FLASH_ERROR;

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    check_result(test_buffer, len);

    return FLASH_OK;
}

void check_result(uint32_t *test_buffer, uint32_t len) {
    uint32_t errors = 0;
    for (int i=0; i < ((len%4==0) ? len/4 : len/4 + 1); i++) {
        if (i < len/4 ) {
            if(flash_data[i] != test_buffer[i]) {
                PRINTF("index@%u : %x != %x(ref)\n\r", i, flash_data[i], test_buffer[i]);
                errors++;
            }
        } else {
            uint32_t last_bytes = 0;
            memcpy(&last_bytes, &test_buffer[i], len % 4);
            if (flash_data[i] != last_bytes) {
                PRINTF("index@%u : %x != %x(ref)\n\r", i, flash_data[i], last_bytes);
                errors++;
            }
        }
    }

    if (errors == 0) {
        PRINTF("success!\n\r");
    } else {
        PRINTF("failure, %d errors!\n\r", errors);
        global_errors += errors;
    }
}
