/**
 * @file main.c
 * @brief Simple spi write example using BSP
 *
 * Simple example that writes a 1kB buffer to flash memory at a specific address
 * and then read it back to check if the data was written correctly.
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
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
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
uint32_t test_read(uint32_t *test_buffer, uint32_t len);
uint32_t test_read_dma(uint32_t *test_buffer, uint32_t len);
uint32_t test_read_quad(uint32_t *test_buffer, uint32_t len);
uint32_t test_read_quad_dma(uint32_t *test_buffer, uint32_t len);

// Check function
uint32_t check_result(uint8_t *test_buffer, uint32_t len);

// Define global status variable
w25q_error_codes_t global_status;

int main(int argc, char *argv[]) {
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

    if ( get_spi_flash_mode(&soc_ctrl) == SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO ) {
        PRINTF("This application cannot work with the memory mapped SPI FLASH"
            "module - do not use the FLASH_EXEC linker script for this application\n");
        return EXIT_SUCCESS;
    }

    PRINTF("BSP read test\n", LENGTH);

    // Pick the correct spi device based on simulation type
    spi_host_t spi;
    #ifndef USE_SPI_FLASH
    spi.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    #else
    spi.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    #endif

    // Define status variable
    int32_t errors = 0;

    // Init SPI host and SPI<->Flash bridge parameters
    if (w25q128jw_init(spi) != FLASH_OK) return EXIT_FAILURE;

    // Test simple read
    PRINTF("Testing simple read...\n");
    errors += test_read(TEST_BUFFER, LENGTH);

    // Test simple read with DMA
    PRINTF("Testing simple read with DMA...\n");
    errors += test_read_dma(TEST_BUFFER, LENGTH);

    // Test quad read
    PRINTF("Testing quad read...\n");
    errors += test_read_quad(TEST_BUFFER, LENGTH);

    // Test quad read with DMA
    PRINTF("Testing quad read with DMA...\n");
    errors += test_read_quad_dma(TEST_BUFFER, LENGTH);

    PRINTF("\n--------TEST FINISHED--------\n");
    if (errors == 0) {
        PRINTF("All tests passed!\n");
        return EXIT_SUCCESS;
    } else {
        PRINTF("Some tests failed!\n");
        return EXIT_FAILURE;
    }

}

uint32_t test_read(uint32_t *test_buffer, uint32_t len) {

    uint32_t *test_buffer_flash = test_buffer;

    // Read from flash memory at the same address
    w25q_error_codes_t status = w25q128jw_read_standard(test_buffer_flash, flash_data, len);
    if (status != FLASH_OK) exit(EXIT_FAILURE);

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    uint32_t res =  check_result(test_buffer, len);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return res;
}

uint32_t test_read_dma(uint32_t *test_buffer, uint32_t len) {

    uint32_t *test_buffer_flash = test_buffer;

    // Read from flash memory at the same address
    w25q_error_codes_t status = w25q128jw_read_standard_dma(test_buffer_flash, flash_data, len);
    if (status != FLASH_OK) exit(EXIT_FAILURE);

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    uint32_t res = check_result(test_buffer, len);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return res;
}

uint32_t test_read_quad(uint32_t *test_buffer, uint32_t len) {

    uint32_t *test_buffer_flash = test_buffer;

    // Read from flash memory at the same address
    w25q_error_codes_t status = w25q128jw_read_quad(test_buffer_flash, flash_data, len);
    if (status != FLASH_OK) exit(EXIT_FAILURE);

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    uint32_t res = check_result(test_buffer, len);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return res;
}

uint32_t test_read_quad_dma(uint32_t *test_buffer, uint32_t len) {

    uint32_t *test_buffer_flash = test_buffer;

    // Read from flash memory at the same address
    w25q_error_codes_t status = w25q128jw_read_quad_dma(test_buffer_flash, flash_data, len);
    if (status != FLASH_OK) exit(EXIT_FAILURE);

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    uint32_t res = check_result(test_buffer, len);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return res;
}

uint32_t check_result(uint8_t *test_buffer, uint32_t len) {
    uint32_t errors = 0;
    uint8_t *flash_data_char = (uint8_t *)flash_data;

    for (uint32_t i = 0; i < len; i++) {
        if (test_buffer[i] != flash_data_char[i]) {
            PRINTF("Error at position %d: expected %x, got %x\n", i, test_buffer[i], flash_data_char[i]);
            errors++;
        }
    }

    if (errors == 0) {
        PRINTF("success!\n");
    } else {
        PRINTF("failure, %d errors!\n", errors);
    }

    return errors;
}
