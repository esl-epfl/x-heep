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
// #include "data_array.bin"
#include "rv_timer.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"

#include "power_manager.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "rv_timer_regs.h"

// Flash address to write to (different from the address where the buffer is stored)
#define FLASH_ADDR 0x00008500

// Len in bytes of the test buffer, 1kB
#define MAX_TEST_BUF_LEN 1024 // 1024 MAX POSSIBLE

// End buffer
uint32_t flash_data_32[MAX_TEST_BUF_LEN];

// Test buffer
// The test buffer is contained in data_array.bin and is 1kB long
const uint32_t flash_original_32[] = {
    0x00000000, 0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007,
    0x00000008, 0x00000009, 0x0000000A, 0x0000000B, 0x0000000C, 0x0000000D, 0x0000000E, 0x0000000F,
    0x00000010, 0x00000011, 0x00000012, 0x00000013, 0x00000014, 0x00000015, 0x00000016, 0x00000017,
    0x00000018, 0x00000019, 0x0000001A, 0x0000001B, 0x0000001C, 0x0000001D, 0x0000001E, 0x0000001F,
    0x00000020, 0x00000021, 0x00000022, 0x00000023, 0x00000024, 0x00000025, 0x00000026, 0x00000027, // 40
    0x00000028, 0x00000029, 0x0000002A, 0x0000002B, 0x0000002C, 0x0000002D, 0x0000002E, 0x0000002F,
    0x00000030, 0x00000031, 0x00000032, 0x00000033, 0x00000034, 0x00000035, 0x00000036, 0x00000037,
    0x00000038, 0x00000039, 0x0000003A, 0x0000003B, 0x0000003C, 0x0000003D, 0x0000003E, 0xABCDEF3F, // 64
    0x00000040, 0x00000041, 0x00000042, 0x00000043, 0x00000044, 0x00000045, 0x00000046, 0x00000047,
    0x00000048, 0x00000049, 0x0000004A, 0x0000004B, 0x0000004C, 0x0000004D, 0x0000004E, 0x0000004F, // 80
    0x00000050, 0x00000051, 0x00000052, 0x00000053, 0x00000054, 0x00000055, 0x00000056, 0x00000057,
    0x00000058, 0x00000059, 0x0000005A, 0x0000005B, 0x0000005C, 0x0000005D, 0x0000005E, 0x0000005F,
    0x00000060, 0x00000061, 0x00000062, 0x00000063, 0x00000064, 0x00000065, 0x00000066, 0x00000067,
    0x00000068, 0x00000069, 0x0000006A, 0x0000006B, 0x0000006C, 0x0000006D, 0x0000006E, 0x0000006F,
    0x00000070, 0x00000071, 0x00000072, 0x00000073, 0x00000074, 0x00000075, 0x00000076, 0x00000077, // 120
    0x00000078, 0x00000079, 0x0000007A, 0x0000007B, 0x0000007C, 0x0000007D, 0x0000007E, 0x0000007F,
    0x00000080, 0x00000081, 0x00000082, 0x00000083, 0x00000084, 0x00000085, 0x00000086, 0x00000087,
    0x00000088, 0x00000089, 0x0000008A, 0x0000008B, 0x0000008C, 0x0000008D, 0x0000008E, 0x0000008F,
    0x00000090, 0x00000091, 0x00000092, 0x00000093, 0x00000094, 0x00000095, 0x00000096, 0x00000097,
    0x00000098, 0x00000099, 0x0000009A, 0x0000009B, 0x0000009C, 0x0000009D, 0x0000009E, 0x0000009F, // 160
    0x000000A0, 0x000000A1, 0x000000A2, 0x000000A3, 0x000000A4, 0x000000A5, 0x000000A6, 0x000000A7,
    0x000000A8, 0x000000A9, 0x000000AA, 0x000000AB, 0x000000AC, 0x000000AD, 0x000000AE, 0x000000AF,
    0x000000B0, 0x000000B1, 0x000000B2, 0x000000B3, 0x000000B4, 0x000000B5, 0x000000B6, 0x000000B7,
    0x000000B8, 0x000000B9, 0x000000BA, 0x000000BB, 0x000000BC, 0x000000BD, 0x000000BE, 0x000000BF,
    0x000000C0, 0x000000C1, 0x000000C2, 0x000000C3, 0x000000C4, 0x000000C5, 0x000000C6, 0x000000C7, // 200
    0x000000C8, 0x000000C9, 0x000000CA, 0x000000CB, 0x000000CC, 0x000000CD, 0x000000CE, 0x000000CF,
    0x000000D0, 0x000000D1, 0x000000D2, 0x000000D3, 0x000000D4, 0x000000D5, 0x000000D6, 0x000000D7,
    0x000000D8, 0x000000D9, 0x000000DA, 0x000000DB, 0x000000DC, 0x000000DD, 0x000000DE, 0x000000DF,
    0x000000E0, 0x000000E1, 0x000000E2, 0x000000E3, 0x000000E4, 0x000000E5, 0x000000E6, 0x000000E7,
    0x000000E8, 0x000000E9, 0x000000EA, 0x000000EB, 0x000000EC, 0x000000ED, 0x000000EE, 0x000000EF, // 240
    0x000000F0, 0x000000F1, 0x000000F2, 0x000000F3, 0x000000F4, 0x000000F5, 0x000000F6, 0x000000F7,
    0x000000F8, 0x000000F9, 0x000000FA, 0x000000FB, 0x000000FC, 0x000000FD, 0x000000FE, 0x000000FF // 256
};

// Private functions
static mmio_region_t init_timer(void);
static void reset_timer(uint32_t hart_id);

// Global variables
int hart_id = 0;
rv_timer_t timer_0_1;
mmio_region_t timer_0_1_reg;


int main(int argc, char *argv[]) {
    printf("BSP profiling standard functions\n\r");
    error_codes_t status;
    uint32_t errors = 0;
    uint32_t flag = 0;
    uint64_t timer_value = 0;

    // Init SPI host and SPI<->Flash bridge parameters 
    status = w25q128jw_init();
    if (status != FLASH_OK) return EXIT_FAILURE;

    // Init timer
    timer_0_1_reg = init_timer();

    uint8_t *test_buffer = (uint8_t *)flash_original_32;
    uint8_t *flash_data = (uint8_t *)flash_data_32;

    printf("Start profile routine - standard speed...\n\r");
    for (int i = 4; i <= 1024; i = i+4) {
        // Reset timer
        reset_timer(hart_id);

        // Start timer
        rv_timer_counter_set_enabled(&timer_0_1, hart_id, kRvTimerEnabled);

        // Write to flash memory at specific address
        status = w25q128jw_write_standard(FLASH_ADDR, test_buffer, i);

        // Stop timer
        rv_timer_counter_set_enabled(&timer_0_1, hart_id, kRvTimerDisabled);

        // Check for errors during write
        if (status != FLASH_OK) return EXIT_FAILURE;

        // Read and print timer value
        rv_timer_counter_read(&timer_0_1, hart_id, &timer_value);
        printf("W%u, ", timer_value);

        // -------------------------------

        // Reset timer
        reset_timer(hart_id);

        // Start timer
        rv_timer_counter_set_enabled(&timer_0_1, hart_id, kRvTimerEnabled);

        // Read from flash memory at the same address
        status = w25q128jw_read_standard(flash_original_32, flash_data, i);
        
        // Stop timer
        rv_timer_counter_set_enabled(&timer_0_1, hart_id, kRvTimerDisabled);

        // Check for errors during read
        if (status != FLASH_OK) return EXIT_FAILURE;

        // Read and print timer value
        rv_timer_counter_read(&timer_0_1, hart_id, &timer_value);
        printf("R%u, ", timer_value);

        // -------------------------------

        // Check if what we read is correct (i.e. flash_original == flash_data)
        for (int j=0; j < i; j++) {
            if(flash_data[j] != test_buffer[j]) {
                printf("iteration %u - index@%u : %x != %x(ref)\n\r", i, j, flash_data[j], test_buffer[j]);
                errors++;
            }
        }
        if (errors > 0) flag = 1;
        errors = 0;
    }


    printf("\nStart profile routine - quad speed...\n\r");
    for (int i = 4; i <= MAX_TEST_BUF_LEN; i = i+4) {
        // Reset timer
        reset_timer(hart_id);

        // Start timer
        rv_timer_counter_set_enabled(&timer_0_1, hart_id, kRvTimerEnabled);

        // Write to flash memory at specific address
        status = w25q128jw_write_quad(FLASH_ADDR, test_buffer, i);

        // Stop timer
        rv_timer_counter_set_enabled(&timer_0_1, hart_id, kRvTimerDisabled);

        // Check for errors during write
        if (status != FLASH_OK) return EXIT_FAILURE;

        // Read and print timer value
        rv_timer_counter_read(&timer_0_1, hart_id, &timer_value);
        printf("W%u, ", timer_value);

        // -------------------------------

        // Reset timer
        reset_timer(hart_id);

        // Start timer
        rv_timer_counter_set_enabled(&timer_0_1, hart_id, kRvTimerEnabled);

        // Read from flash memory at the same address
        status = w25q128jw_read_quad(FLASH_ADDR, flash_data, i);
        
        // Stop timer
        rv_timer_counter_set_enabled(&timer_0_1, hart_id, kRvTimerDisabled);

        // Check for errors during read
        if (status != FLASH_OK) return EXIT_FAILURE;

        // Read and print timer value
        rv_timer_counter_read(&timer_0_1, hart_id, &timer_value);
        printf("R%u, ", timer_value);

        // -------------------------------

        // Check if what we read is correct (i.e. flash_original == flash_data)
        for (int j=0; j < i; j++) {
            if(flash_data[j] != test_buffer[j]) {
                printf("iteration %u - index@%x : %x != %x(ref)\n\r", i, j, flash_data[j], test_buffer[j]);
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




// -----------------
// Private functions
// -----------------

static mmio_region_t init_timer(void) {
    // Get current Frequency
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    uint32_t freq_hz = soc_ctrl_get_frequency(&soc_ctrl);
    printf("Freq: %u\n", freq_hz);

    // Initialize timer
    mmio_region_t timer_0_1_reg = mmio_region_from_addr(RV_TIMER_AO_START_ADDRESS);
    rv_timer_init(timer_0_1_reg, (rv_timer_config_t){.hart_count = 2, .comparator_count = 1}, &timer_0_1);
    /* defining timer prescale and step based on its desired freq */
    uint64_t kTickFreqHz = 1000 * 1000; // 1 MHz
    rv_timer_tick_params_t tick_params;
    rv_timer_approximate_tick_params(freq_hz, kTickFreqHz, &tick_params);
    if (tick_params.prescale==0){
        printf("Timer approximate function was not able to set a correct value prescale\n");
    }

    // Return timer memory region
    return timer_0_1_reg;
}

static void reset_timer(uint32_t hart_id) {
    mmio_region_write32(
        timer_0_1_reg,
        reg_for_hart(hart_id, RV_TIMER_TIMER_V_LOWER0_REG_OFFSET), 0x0
    );
    mmio_region_write32(
        timer_0_1_reg,
        reg_for_hart(hart_id, RV_TIMER_TIMER_V_UPPER0_REG_OFFSET), 0x0
    );
}
