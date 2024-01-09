/**
 * @file main.c
 * @brief Simple spi write example using BSP
 *
 * Simple example that writes a 1kB buffer to flash memory at a specific address
 * and then read it back to check if the data was written correctly.
 * For buffers bigger than 4kB (or even smaller buffers that are not aligned to 4kB),
 * the erase operation must be tweeked to erase all the sectors that contain the
 * buffer.
 * 
 * @note The application assume the correct functioning of the read operation.
 *
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "x-heep.h"
#include "w25q128jw.h"

/* By default, PRINTFs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   1

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
 * The flash address must also be specified, as the test writes to a specific
 * location.
*/
#define TEST_BUFFER flash_original_1024B
#define LENGTH 1024
#define FLASH_ADDR 0x00008500


// Test functions
uint32_t test_write(uint32_t *test_buffer, uint32_t len);
uint32_t test_write_dma(uint32_t *test_buffer, uint32_t len);
uint32_t test_write_quad(uint32_t *test_buffer, uint32_t len);
uint32_t test_write_quad_dma(uint32_t *test_buffer, uint32_t len);

// Check function
uint32_t check_result(uint8_t *test_buffer, uint32_t len);

// Erase memory function
void erase_memory(uint32_t addr);

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

    PRINTF("BSP write test\n");

    // Pick the correct spi device
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

    // Test simple write
    PRINTF("Testing simple write...\n");
    errors += test_write(TEST_BUFFER, LENGTH);

    // Test simple write with DMA
    PRINTF("Testing simple write with DMA...\n");
    errors += test_write_dma(TEST_BUFFER, LENGTH);

    // Test quad write
    PRINTF("Testing quad write...\n");
    errors += test_write_quad(TEST_BUFFER, LENGTH);

    // Test quad write with DMA
    PRINTF("Testing quad write with DMA...\n");
    errors += test_write_quad_dma(TEST_BUFFER, LENGTH);

    PRINTF("\n--------TEST FINISHED--------\n");
    if (errors == 0) {
        PRINTF("All tests passed!\n");
        return EXIT_SUCCESS;
    } else {
        PRINTF("Some tests failed!\n");
        return EXIT_FAILURE;
    }
}

uint32_t test_write(uint32_t *test_buffer, uint32_t len) {
    // Write to flash memory at specific address
    global_status = w25q128jw_write_standard(FLASH_ADDR, test_buffer, len);
    if (global_status != FLASH_OK) exit(EXIT_FAILURE);

    // Read from flash memory at the same address
    global_status = w25q128jw_read(FLASH_ADDR, flash_data, len);
    if (global_status != FLASH_OK) exit(EXIT_FAILURE);

    // Check if what we read is correct (i.e. flash_original == flash_data)
    int32_t result = check_result(test_buffer, len);

    // Clean memory for next test
    erase_memory(FLASH_ADDR);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return result;
}

uint32_t test_write_dma(uint32_t *test_buffer, uint32_t len) {
    // Write to flash memory at specific address
    global_status = w25q128jw_write_standard_dma(FLASH_ADDR, test_buffer, len);
    if (global_status != FLASH_OK) exit(EXIT_FAILURE);

    // Read from flash memory at the same address
    global_status = w25q128jw_read(FLASH_ADDR, flash_data, len);
    if (global_status != FLASH_OK) exit(EXIT_FAILURE);

    // Check if what we read is correct (i.e. flash_original == flash_data)
    int32_t result = check_result(test_buffer, len);

    // Clean memory for next test
    erase_memory(FLASH_ADDR);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return result;
}

uint32_t test_write_quad(uint32_t *test_buffer, uint32_t len) {
    // Write to flash memory at specific address
    global_status = w25q128jw_write_quad(FLASH_ADDR, test_buffer, len);
    if (global_status != FLASH_OK) exit(EXIT_FAILURE);

    // Read from flash memory at the same address
    global_status = w25q128jw_read(FLASH_ADDR, flash_data, len);
    if (global_status != FLASH_OK) exit(EXIT_FAILURE);

    // Check if what we read is correct (i.e. flash_original == flash_data)
    int32_t result = check_result(test_buffer, len);

    // Clean memory for next test
    erase_memory(FLASH_ADDR);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return result;
}

uint32_t test_write_quad_dma(uint32_t *test_buffer, uint32_t len) {
    // Write to flash memory at specific address
    global_status = w25q128jw_write_quad_dma(FLASH_ADDR, test_buffer, len);
    if (global_status != FLASH_OK) exit(EXIT_FAILURE);

    // Read from flash memory at the same address
    global_status = w25q128jw_read(FLASH_ADDR, flash_data, len);
    if (global_status != FLASH_OK) exit(EXIT_FAILURE);

    // Check if what we read is correct (i.e. flash_original == flash_data)
    int32_t result = check_result(test_buffer, len);

    // Clean memory for next test
    erase_memory(FLASH_ADDR);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return result;
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

// Erase the memory only if FPGA is used
void erase_memory(uint32_t addr) {
    #ifdef USE_SPI_FLASH
    w25q128jw_4k_erase(FLASH_ADDR);
    #endif
}