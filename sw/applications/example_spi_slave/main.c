#include <stdint.h>
#include <stdlib.h>

#include "x-heep.h"
#include "spi_host.h"
#include "spi_slave_sdk.h"

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA 1
#define PRINTF_IN_SIM 0

#if TARGET_SIM && PRINTF_IN_SIM
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define DATA_LENGTH_B 21
#define DUMMY_CYCLES  32

uint8_t buffer_1[DATA_LENGTH_B] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };
uint8_t buffer_2[DATA_LENGTH_B];

static spi_host_t* host; 

int main(){

    host = spi_host1;
    spi_return_flags_e flags;

    // Initialize the SPI host to transmit data to the SPI slave
    if( spi_host_init(host)!= SPI_FLAG_SUCCESS) {
        PRINTF("Failure to initialize\n\r");
        return EXIT_FAILURE;
    }

    // Use the SPI Host SDK to write into the SPI slave.
    // In the SPI Slave SDK there are the needed SPI HOST functions to control the SPI Slave. 
    // The SPI slave per se cannot be controlled from X-HEEP's SW. 
    if( spi_slave_write(host, buffer_2, buffer_1, DATA_LENGTH_B) != SPI_FLAG_SUCCESS) {
        PRINTF("Failure to write\n\r");
        return EXIT_FAILURE;
    }

    // Check that the written values match the source ones. 
    // Also increment those values +1 to use as new data.
    for(int i=0; i < DATA_LENGTH_B; i++){
        if(buffer_1[i] != buffer_2[i]) return EXIT_FAILURE;
        buffer_2[i] = i+1;
    }

    // Use the SPI Host SDK to read from the SPI slave and store the read values in buffer 1
    if( spi_slave_read(host, buffer_2, buffer_1, DATA_LENGTH_B, DUMMY_CYCLES*2) != SPI_FLAG_SUCCESS) {
        PRINTF("Failure to read\n\r");
        return EXIT_FAILURE;
    }

    // check that the new values still match
    for(int i=0; i < DATA_LENGTH_B; i++){
        if(buffer_1[i] != buffer_2[i]) return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}





