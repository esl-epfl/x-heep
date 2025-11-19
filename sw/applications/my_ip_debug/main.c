#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "x-heep.h"
#include "w25q128jw.h"

#define REVERT_24b_ADDR(addr) (bitfield_byteswap32(addr) >> 8)

uint32_t flash_data[256];

#define FLASH_ONLY_WORDS 32
#define FLASH_ONLY_BYTES (FLASH_ONLY_WORDS*4)

int32_t __attribute__((section(".xheep_data_flash_only"))) __attribute__ ((aligned (16))) flash_only_buffer[FLASH_ONLY_WORDS] = {
    0xABCDEF00,
    0xABCDEF01,
    0xABCDEF02,
    0xABCDEF03,
    0xABCDEF04,
    0xABCDEF05,
    0xABCDEF06,
    0xABCDEF07,
    0xABCDEF08,
    0xABCDEF09,
    0xABCDEF0A,
    0xABCDEF0B,
    0xABCDEF0C,
    0xABCDEF0D,
    0xABCDEF0E,
    0xABCDEF0F,
    0xABCDEF10,
    0xABCDEF11,
    0xABCDEF12,
    0xABCDEF13,
    0xABCDEF14,
    0xABCDEF15,
    0xABCDEF16,
    0xABCDEF17,
    0xABCDEF18,
    0xABCDEF19,
    0xABCDEF1A,
    0xABCDEF1B,
    0xABCDEF1C,
    0xABCDEF1D,
    0xABCDEF1E,
    0xABCDEF1F,
};

int32_t __attribute__ ((aligned (16))) flash_only_buffer_golden_value[FLASH_ONLY_WORDS] = {
    0xABCDEF00,
    0xABCDEF01,
    0xABCDEF02,
    0xABCDEF03,
    0xABCDEF04,
    0xABCDEF05,
    0xABCDEF06,
    0xABCDEF07,
    0xABCDEF08,
    0xABCDEF09,
    0xABCDEF0A,
    0xABCDEF0B,
    0xABCDEF0C,
    0xABCDEF0D,
    0xABCDEF0E,
    0xABCDEF0F,
    0xABCDEF10,
    0xABCDEF11,
    0xABCDEF12,
    0xABCDEF13,
    0xABCDEF14,
    0xABCDEF15,
    0xABCDEF16,
    0xABCDEF17,
    0xABCDEF18,
    0xABCDEF19,
    0xABCDEF1A,
    0xABCDEF1B,
    0xABCDEF1C,
    0xABCDEF1D,
    0xABCDEF1E,
    0xABCDEF1F,
};

uint32_t test_read_flash_only(uint32_t *test_buffer, uint32_t len);
uint32_t check_result(uint8_t *test_buffer, uint32_t len);

// Define global status variable
w25q_error_codes_t global_status;

int main(int argc, char *argv[]) {
    // uint32_t debug_tool = 0x40010000;
    // uint32_t debug_tool_cmd = ((REVERT_24b_ADDR(debug_tool & 0x00ffffff) << 8) | FC_RD);
    // printf("debut_tool_cmd: 0x%08x", debug_tool_cmd);
    

    // Pick the correct spi device based on simulation type
    spi_host_t* spi;
    spi = spi_flash;

    // Define status variable
    int32_t errors = 0;

    errors += test_read_flash_only(flash_only_buffer, FLASH_ONLY_BYTES);

    if (errors) {
        printf("F\n");
        return EXIT_FAILURE;
    }

}

uint32_t test_read_flash_only(uint32_t *test_buffer, uint32_t len) {

    printf("test_buffer: 0x%08x", test_buffer);
    // Gives the address offset how where the test_buffer is stored in the flash
    // Doesn't make any sense/supported if load test_buffer directly on_chip
    uint32_t *test_buffer_flash = heep_get_flash_address_offset(test_buffer);
    printf("test_buffer_flash: 0x%08x\n", (uint32_t)test_buffer_flash);


    // Read from flash memory at the same address
    w25q_error_codes_t status = w25q128jw_read_standard(test_buffer_flash, flash_data, len);
    if (status != FLASH_OK) exit(EXIT_FAILURE);

    // Check if what we read is correct (i.e. flash_data == test_buffer)
    uint32_t res =  check_result(flash_only_buffer_golden_value, len);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return res;
}

uint32_t test_read_dma(uint32_t *test_buffer, uint32_t len) {

    uint32_t *test_buffer_flash = test_buffer;

    // Read from flash memory at the same address
    w25q_error_codes_t status = w25q128jw_read_standard_dma(test_buffer_flash, flash_data, len, 0, 0);
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
            printf("Error at position %d: expected %x, got %x\n", i, test_buffer[i], flash_data_char[i]);
            errors++;
            break;
        }
    }

    return errors;
}