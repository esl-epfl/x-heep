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

/* To get TX and RX FIFO depth */
#include "spi_host_regs.h"
/* To get SPI functions */
#include "spi_host.h"

#include "x-heep.h"
#include "w25q128jw.h"
#include "dma.h"

/* By default, PRINTFs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  0
#define PRINTF_IN_SIM   1

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#if defined(TARGET_PYNQ_Z2) || defined(TARGET_ZCU104) || defined(TARGET_NEXYS_A7_100T)
    #define USE_SPI_FLASH
#endif

#ifndef TARGET_SIM
#define USE_SPI_FLASH
#endif

// Start buffers (the original data)
#include "buffer.h"
// End buffer (where what is read is stored)
uint32_t flash_data[256];

#define TEST_BUFFER_WORDS flash_original_128B
#define TEST_BUFFER_SE_HALF_WORDS test_flash_se_half_words
#define TEST_BUFFER_SE_BYTES test_flash_se_bytes
#define TEST_BUFFER_HALF_WORDS test_flash_half_words
#define TEST_BUFFER_BYTES test_flash_bytes
#define LENGTH 128

typedef enum {
    TYPE_WORD = 2,
    TYPE_HALF_WORD = 1,
    TYPE_BYTE = 0
} dma_trans_data_t;

// Test functions
uint32_t test_read_dma(uint32_t *test_buffer, uint32_t len, dma_trans_data_t dma_data_type, uint8_t sign_extend);
uint32_t test_read_quad_dma(uint32_t *test_buffer, uint32_t len, dma_trans_data_t dma_data_type, uint8_t sign_extend);

// Check function
uint32_t check_result(uint32_t *test_buffer, uint32_t len, dma_trans_data_t dma_data_type, uint32_t sign_extend);

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

    // Pick the correct spi device based on simulation type
    spi_host_t* spi;
    #ifndef USE_SPI_FLASH
    spi = spi_host1;
    #else
    spi = spi_flash;
    #endif

    // Define status variable
    int32_t errors = 0;

    // Init SPI host and SPI<->Flash bridge parameters
    if (w25q128jw_init(spi) != FLASH_OK) return EXIT_FAILURE;

    // DMA transaction data type
    dma_trans_data_t dma_data_type;

    // Test simple read with DMA
    PRINTF("Testing read with DMA in SUBADDRESS mode...\n");

    dma_data_type = TYPE_WORD;
    errors += test_read_dma(TEST_BUFFER_WORDS, LENGTH, dma_data_type, 0);
    dma_data_type = TYPE_HALF_WORD;
    errors += test_read_dma(TEST_BUFFER_HALF_WORDS, LENGTH, dma_data_type, 0);
    errors += test_read_dma(TEST_BUFFER_SE_HALF_WORDS, LENGTH, dma_data_type, 1);
    dma_data_type = TYPE_BYTE;
    errors += test_read_dma(TEST_BUFFER_BYTES, LENGTH, dma_data_type, 0);
    errors += test_read_dma(TEST_BUFFER_SE_BYTES, LENGTH, dma_data_type, 1);

    // Test quad read with DMA
    dma_data_type = TYPE_WORD;
    errors += test_read_quad_dma(TEST_BUFFER_WORDS, LENGTH, dma_data_type, 0);
    dma_data_type = TYPE_HALF_WORD;
    errors += test_read_quad_dma(TEST_BUFFER_HALF_WORDS, LENGTH, dma_data_type, 0);
    errors += test_read_quad_dma(TEST_BUFFER_SE_HALF_WORDS, LENGTH, dma_data_type, 1);
    dma_data_type = TYPE_BYTE;
    errors += test_read_quad_dma(TEST_BUFFER_BYTES, LENGTH, dma_data_type, 0);
    errors += test_read_quad_dma(TEST_BUFFER_SE_BYTES, LENGTH, dma_data_type, 1);

    PRINTF("\n--------TEST FINISHED--------\n");
    if (errors == 0) {
        PRINTF("All tests passed!\n");
        return EXIT_SUCCESS;
    } else {
        PRINTF("Some tests failed!\n");
        return EXIT_FAILURE;
    }

}

uint32_t test_read_dma(uint32_t *test_buffer, uint32_t len, dma_trans_data_t dma_data_type, uint8_t sign_extend) {
    
    dma_data_type_t dma_trans_data_type;

    switch (dma_data_type) {
        case TYPE_WORD:
            dma_trans_data_type = DMA_DATA_TYPE_WORD;
            break;
        case TYPE_HALF_WORD:
            dma_trans_data_type = DMA_DATA_TYPE_HALF_WORD;
            break;
        case TYPE_BYTE:
            dma_trans_data_type = DMA_DATA_TYPE_BYTE;
            break;
        default:
            break;
    }

    dma_init(NULL);

    // The DMA will wait for the SPI HOST/FLASH RX FIFO valid signal
    #ifndef USE_SPI_FLASH
    uint8_t slot = DMA_TRIG_SLOT_SPI_RX;
    #else
    uint8_t slot = DMA_TRIG_SLOT_SPI_FLASH_RX;
    #endif

    // Set up DMA source target
    dma_target_t tgt_src = {
        .inc_d1_du = 0, // Target is peripheral, no increment
        .type = dma_trans_data_type,
    };
    // Target is SPI RX FIFO
    tgt_src.ptr = (uint8_t*) (w25q128jw_read_standard_setup((uint32_t*)(TEST_BUFFER_WORDS), flash_data, len));
    // Trigger to control the data flow
    tgt_src.trig = slot;

    // Set up DMA destination target
    dma_target_t tgt_dst = {
        .inc_d1_du = 1, // Increment by 1 data unit (word)
        .type = DMA_DATA_TYPE_WORD,
        .trig = DMA_TRIG_MEMORY, // Read-write operation to memory
    };
    tgt_dst.ptr = (uint8_t*)flash_data; // Target is the data buffer

    // Set up DMA transaction
    dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .end = DMA_TRANS_END_POLLING,
        .mode = DMA_TRANS_MODE_SUBADDRESS,
        .sign_ext = sign_extend,
    };

    // Size is in data units (words in this case)
    trans.size_d1_du = len >> (dma_data_type);
    
    // Validate, load and launch DMA transaction
    dma_config_flags_t res;
    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    res = dma_load_transaction(&trans);
    res = dma_launch(&trans);

    // Wait for DMA to finish transaction
    while(!dma_is_ready(0));

    uint32_t result = check_result(test_buffer, len, dma_data_type, sign_extend);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return result;
}

uint32_t test_read_quad_dma(uint32_t *test_buffer, uint32_t len, dma_trans_data_t dma_data_type, uint8_t sign_extend) {

    dma_data_type_t dma_trans_data_type;

    switch (dma_data_type) {
        case TYPE_WORD:
            dma_trans_data_type = DMA_DATA_TYPE_WORD;
            break;
        case TYPE_HALF_WORD:
            dma_trans_data_type = DMA_DATA_TYPE_HALF_WORD;
            break;
        case TYPE_BYTE:
            dma_trans_data_type = DMA_DATA_TYPE_BYTE;
            break;
        default:
            break;
    }

    dma_init(NULL);

    // The DMA will wait for the SPI HOST/FLASH RX FIFO valid signal
    #ifndef USE_SPI_FLASH
    uint8_t slot = DMA_TRIG_SLOT_SPI_RX;
    #else
    uint8_t slot = DMA_TRIG_SLOT_SPI_FLASH_RX;
    #endif

    // Set up DMA source target
    dma_target_t tgt_src = {
        .inc_d1_du = 0, // Target is peripheral, no increment
        .type = dma_trans_data_type,
    };
    // Target is SPI RX FIFO
    tgt_src.ptr = (uint8_t*) (w25q128jw_read_quad_setup((uint32_t*)(TEST_BUFFER_WORDS), flash_data, len));
    // Trigger to control the data flow
    tgt_src.trig = slot;

    // Set up DMA destination target
    dma_target_t tgt_dst = {
        .inc_d1_du = 1, // Increment by 1 data unit (word)
        .type = DMA_DATA_TYPE_WORD,
        .trig = DMA_TRIG_MEMORY, // Read-write operation to memory
    };
    tgt_dst.ptr = (uint8_t*)flash_data; // Target is the data buffer

    // Set up DMA transaction
    dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .end = DMA_TRANS_END_POLLING,
        .mode = DMA_TRANS_MODE_SUBADDRESS,
        .sign_ext = sign_extend,
    };

    // Size is in data units (words in this case)
    trans.size_d1_du = len >> (dma_data_type);
    
    // Validate, load and launch DMA transaction
    dma_config_flags_t res;
    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    res = dma_load_transaction(&trans);
    res = dma_launch(&trans);

    // Wait for DMA to finish transaction
    while(!dma_is_ready(0));

    uint32_t result = check_result(test_buffer, len, dma_data_type, sign_extend);

    // Reset the flash data buffer
    memset(flash_data, 0, len * sizeof(uint8_t));

    return result;
}

uint32_t check_result(uint32_t *test_buffer, uint32_t len, dma_trans_data_t dma_data_type, uint32_t sign_extend) {
    uint32_t errors = 0;

    for (uint32_t i = 0; i < len>>dma_data_type; i += 1) {
        if (test_buffer[i] != flash_data[i]) {
            PRINTF("Error in transfer %d %d at position %d: expected %x, got %x\n", dma_data_type, sign_extend, i, test_buffer[i], flash_data[i]);
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
