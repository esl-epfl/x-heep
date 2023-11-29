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


// End buffer
uint32_t flash_data[256];

// Test buffers:
// ----------------
uint32_t flash_original_32B[8] = {
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef
};
// Test buffer with a not integer number of words
uint32_t flash_original_30B[8] = {
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0xabcd
};
// Test buffer with a length = RX_FIFO_depth (64 words)
uint32_t flash_original_256B[64] = {
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef,
    0x76543210,0xfedcba98,0x579a6f90,0x657d5bee,0x758ee41f,0x01234567,0xfedbca98,0x89abcdef
};
// Test buffer with a length higher than RX_FIFO_depth (64 words)
uint32_t flash_original_768B[192] = {
    0x76543211, 0xfedcba99, 0x579a6f91, 0x657d5bef, 0x758ee420, 0x01234568, 0xfedbca97, 0x89abde00,
    0x76543212, 0xfedcba9a, 0x579a6f92, 0x657d5bf0, 0x758ee421, 0x01234569, 0xfedbca98, 0x89abde01,
    0x76543213, 0xfedcba9b, 0x579a6f93, 0x657d5bf1, 0x758ee422, 0x0123456a, 0xfedbca99, 0x89abde02,
    0x76543214, 0xfedcba9c, 0x579a6f94, 0x657d5bf2, 0x758ee423, 0x0123456b, 0xfedbca9a, 0x89abde03,
    0x76543215, 0xfedcba9d, 0x579a6f95, 0x657d5bf3, 0x758ee424, 0x0123456c, 0xfedbca9b, 0x89abde04,
    0x76543216, 0xfedcba9e, 0x579a6f96, 0x657d5bf4, 0x758ee425, 0x0123456d, 0xfedbca9c, 0x89abde05,
    0x76543217, 0xfedcba9f, 0x579a6f97, 0x657d5bf5, 0x758ee426, 0x0123456e, 0xfedbca9d, 0x89abde06,
    0x76543218, 0xfedcbaa0, 0x579a6f98, 0x657d5bf6, 0x758ee427, 0x0123456f, 0xfedbca9e, 0x89abde07,
    0x76543219, 0xfedcbaa1, 0x579a6f99, 0x657d5bf7, 0x758ee428, 0x01234570, 0xfedbca9f, 0x89abde08,
    0x7654321a, 0xfedcbaa2, 0x579a6f9a, 0x657d5bf8, 0x758ee429, 0x01234571, 0xfedbcaa0, 0x89abde09,
    0x7654321b, 0xfedcbaa3, 0x579a6f9b, 0x657d5bf9, 0x758ee42a, 0x01234572, 0xfedbcaa1, 0x89abde0a,
    0x7654321c, 0xfedcbaa4, 0x579a6f9c, 0x657d5bfa, 0x758ee42b, 0x01234573, 0xfedbcaa2, 0x89abde0b,
    0x7654321d, 0xfedcbaa5, 0x579a6f9d, 0x657d5bfb, 0x758ee42c, 0x01234574, 0xfedbcaa3, 0x89abde0c,
    0x7654321e, 0xfedcbaa6, 0x579a6f9e, 0x657d5bfc, 0x758ee42d, 0x01234575, 0xfedbcaa4, 0x89abde0d,
    0x7654321f, 0xfedcbaa7, 0x579a6f9f, 0x657d5bfd, 0x758ee42e, 0x01234576, 0xfedbcaa5, 0x89abde0e,
    0x76543220, 0xfedcbaa8, 0x579a6fa0, 0x657d5bfe, 0x758ee42f, 0x01234577, 0xfedbcaa6, 0x89abde0f,
    0x76543221, 0xfedcbaa9, 0x579a6fa1, 0x657d5bff, 0x758ee430, 0x01234578, 0xfadbcaa7, 0x89abde10,
    0x76543222, 0xfedcbaaa, 0x579a6fa2, 0x657d5c00, 0x758ee431, 0x01234579, 0xfadbcaa8, 0x89abde11,
    0x76543223, 0xfedcbaab, 0x579a6fa3, 0x657d5c01, 0x758ee432, 0x0123457a, 0xfadbcaa9, 0x89abde12,
    0x76543224, 0xfedcbaac, 0x579a6fa4, 0x657d5c02, 0x758ee433, 0x0123457b, 0xfadbcaaa, 0x89abde13,
    0x76543225, 0xfedcbaad, 0x579a6fa5, 0x657d5c03, 0x758ee434, 0x0123457c, 0xfadbcaab, 0x89abde14,
    0x76543226, 0xfedcbaae, 0x579a6fa6, 0x657d5c04, 0x758ee435, 0x0123457d, 0xfadbcaac, 0x89abde15,
    0x76543227, 0xfedcbaaf, 0x579a6fa7, 0x657d5c05, 0x758ee436, 0x0123457e, 0xfadbcaad, 0x89abde16,
    0x76543228, 0xfedcbab0, 0x579a6fa8, 0x657d5c06, 0x758ee437, 0x0123457f, 0xfadbcaae, 0x89abde17,
};
// ----------------

#define FLASH_ADDR 0x00008500 // Misalligned!

int main(int argc, char *argv[]) {
    printf("BSP write test\n\r");
    w25q_error_codes_t status;

    uint32_t *test_buffer = flash_original_32B;
    uint32_t len = 32;

    // Init SPI host and SPI<->Flash bridge parameters 
    status = w25q128jw_init();
    if (status != FLASH_OK) return EXIT_FAILURE;

    // Write to flash memory at specific address
    uint32_t test_zeros[8] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    status = w25q128jw_write_quad_dma(FLASH_ADDR, test_zeros, 32);
    if (status != FLASH_OK) return EXIT_FAILURE;

    status = w25q128jw_write_quad_dma(FLASH_ADDR, test_buffer, 30);
    if (status != FLASH_OK) return EXIT_FAILURE;

    // Read from flash memory at the same address
    status = w25q128jw_read_standard(FLASH_ADDR, flash_data, len);
    if (status != FLASH_OK) return EXIT_FAILURE;


    // Check if what we read is correct (i.e. flash_original == flash_data)
    printf("flash vs ram...\n\r");
    uint32_t errors = 0;
    for (int i=0; i< ((len%4==0) ? len/4 : len/4 + 1); i++) {
        printf("index@%x : %x == %x(ref)\n\r", i, flash_data[i], test_buffer[i]);
        if(flash_data[i] != test_buffer[i]) {
            printf("index@%x : %x != %x(ref)\n\r", i, flash_data[i], test_buffer[i]);
            errors++;
        }
    }

    // Exit status based on errors found
    if (errors == 0) {
        printf("success!\n\r");
        return EXIT_SUCCESS;
    } else {
        printf("failure, %d errors!\n\r", errors);
        return EXIT_FAILURE;
    }
}