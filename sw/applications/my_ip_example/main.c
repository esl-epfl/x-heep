#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "x-heep.h"

#include "my_ip.h"
#include "my_ip_structs.h"

#include "dma.h"

#define R_WORDS 32

// Generate artificial FLASH memory data
int32_t __attribute__((section(".xheep_data_flash_only"))) __attribute__ ((aligned (16))) r_data[R_WORDS] = {
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

// Copy of data stored in flash to be able to verify read operation (RAM vs. RAM required for code to run)
int32_t __attribute__ ((aligned (16))) r_golden_data[R_WORDS] = {
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
// Generate artificial space memory
uint32_t s_data[256];
// Give address to read from FLASH (r_address)
uint32_t *r_address = r_data;
// Give address to store data read from FLASH (s_address)
uint32_t *s_address = s_data;
// Give length of data to read from FLASH (length)
#define LENGTH (R_WORDS*4)

// Check function
uint32_t check_result(uint8_t *test_buffer, uint32_t len);

__attribute__((optimize("O0"))) void my_ip_run(){
    // Clean DMA
    dma_init(NULL);
    // Gives the address offset how where the test_buffer is stored in the flash
    // r_address = heep_get_flash_address_offset(r_data);
    // Load r_address
    write_register( (uint32_t)r_address,
                    MY_IP_R_ADDRESS_REG_OFFSET,
                    0xFFFFFFFF,
                    0,
                    MY_IP_START_ADDRESS
                );
    // Load s_address
    write_register( (uint32_t)s_address,
                    MY_IP_S_ADDRESS_REG_OFFSET,
                    0xFFFFFFFF,
                    0,
                    MY_IP_START_ADDRESS
                );
    // Load length
    write_register( LENGTH,
                    MY_IP_LENGTH_REG_OFFSET,
                    0xFFFFFFFF,
                    0,
                    MY_IP_START_ADDRESS
                );
    // Start read operation
    write_register( 0x1,
                    MY_IP_CONTROL_REG_OFFSET,
                    0x1,
                    MY_IP_CONTROL_START_BIT,
                    MY_IP_START_ADDRESS
                );
}


int main(void) {

    
    my_ip_run();

    // Wait for DMA transfer to complete
    while(!my_ip_is_ready());

    uint32_t res =  check_result(r_golden_data, LENGTH);
    // return res;

    printf("s_data[0] is: 0x%08x\n",s_data[0]);
    printf("s_data[1] is: 0x%08x\n",s_data[1]);

    return EXIT_SUCCESS;
}

uint32_t check_result(uint8_t *test_buffer, uint32_t len) {
    uint32_t errors = 0;
    uint8_t *flash_data_char = (uint8_t *)s_data;

    for (uint32_t i = 0; i < len; i++) {
        if (test_buffer[i] != flash_data_char[i]) {
            printf("Error at position %d: expected %x, got %x\n", i, test_buffer[i], flash_data_char[i]);
            errors++;
            break;
        }
    }

    return errors;
}


// Verify SPI read values write them down then run spi_read_example and check SPI values (also DMA after if still not the solution)