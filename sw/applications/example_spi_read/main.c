/**
 * @file main.c
 * @brief Simple spi write example using BSP
 *
 * Simple example that writes a 1kB buffer to flash memory at a specific address
 * and then read it back to check if the data was written correctly.
 * 
 * @note The application assume the correct functioning of the read operation.
 *
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "x-heep.h"
#include "w25q128jw.h"


// Test functions
void test_read(uint32_t *test_buffer, uint32_t len);
void test_read_dma(uint32_t *test_buffer, uint32_t len);
void test_read_quad(uint32_t *test_buffer, uint32_t len);
void test_read_quad_dma(uint32_t *test_buffer, uint32_t len);

// Check function
void check_result(uint32_t *test_buffer, uint32_t len);


// Start buffers (the original data)
#include "buffer.bin"
// End buffer (where what is read is stored)
uint32_t flash_data[256];

// Global flag to keep track of errors
uint32_t global_errors = 0;
// Define global status variable
w25q_error_codes_t global_status;

int main(int argc, char *argv[]) {
    printf("BSP read test\n\r");

    /*
     * Assign the test buffer to the buffer to write to flash.
     * The buffer is defined in the file buffer.bin. As multiple buffers can
     * be defined, this is userful to pick the right one.
     * Also the length is specified, to test different length cases. In any case
     * length <= test_buffer length. 
    */
    uint32_t *test_buffer = flash_original_1024B;
    uint32_t len = 1024;

    // Pick the correct spi device
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
    test_read(test_buffer, len);

    // Test simple read with DMA
    test_read_dma(test_buffer, len);

    // Test quad read
    test_read_quad(test_buffer, len);

    // Test quad read with DMA
    test_read_quad_dma(test_buffer, len);

    if (global_errors == 0) {
        printf("All tests passed!\n\r");
        return EXIT_SUCCESS;
    } else {
        printf("Some tests failed!\n\r");
        return EXIT_FAILURE;
    }
    
}

void test_read(uint32_t *test_buffer, uint32_t len) {
    // Read from flash memory at the same address
    global_status = w25q128jw_read(test_buffer, flash_data, len);
    if (global_status != FLASH_OK) return EXIT_FAILURE;

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    check_result(test_buffer, len);
}

void test_read_dma(uint32_t *test_buffer, uint32_t len) {
    // Read from flash memory at the same address
    global_status = w25q128jw_read(test_buffer, flash_data, len);
    if (global_status != FLASH_OK) return EXIT_FAILURE;

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    check_result(test_buffer, len);
}

void test_read_quad(uint32_t *test_buffer, uint32_t len) {
    // Read from flash memory at the same address
    global_status = w25q128jw_read(test_buffer, flash_data, len);
    if (global_status != FLASH_OK) return EXIT_FAILURE;

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    check_result(test_buffer, len);
}

void test_read_quad_dma(uint32_t *test_buffer, uint32_t len) {
    // Read from flash memory at the same address
    global_status = w25q128jw_read(test_buffer, flash_data, len);
    if (global_status != FLASH_OK) return EXIT_FAILURE;

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    check_result(test_buffer, len);
}

void check_result(uint32_t *test_buffer, uint32_t len) {
    printf("flash vs ram...\n\r");
    uint32_t errors = 0;
    for (int i=0; i < ((len%4==0) ? len/4 : len/4 + 1); i++) {
        if (i < len/4 ) {
            if(flash_data[i] != test_buffer[i]) {
                printf("index@%u : %x != %x(ref)\n\r", i, flash_data[i], test_buffer[i]);
                errors++;
            }
        } else {
            uint32_t last_bytes = 0;
            memcpy(&last_bytes, &test_buffer[i], len % 4);
            if (flash_data[i] != last_bytes) {
                printf("index@%u : %x != %x(ref)\n\r", i, flash_data[i], last_bytes);
                errors++;
            }
        }
    }

    if (errors == 0) {
        printf("success!\n\r");
    } else {
        printf("failure, %d errors!\n\r", errors);
        global_errors += errors;
    }
}
