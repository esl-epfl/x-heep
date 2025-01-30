#include <stdint.h>
#include <stdlib.h>

#include "x-heep.h"
#include "spi_host.h"
#include "spi_slave_sdk.h"

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA 1
#define PRINTF_IN_SIM 1

#if TARGET_SIM && PRINTF_IN_SIM
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define DATA_LENGTH_B 21
#define DUMMY_CYCLES  32

uint8_t  source_data_B[DATA_LENGTH_B] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };
uint8_t write_data_B[DATA_LENGTH_B];
uint8_t read_data_B[DATA_LENGTH_B];

static spi_host_t* host; 

int main(){

    host = spi_host1;
    spi_return_flags_e flags;

    if( spi_host_init(host)!= SPI_FLAG_SUCCESS) {
        PRINTF("Failure to initialize\n\r");
        return EXIT_FAILURE;
    }

    if( spi_slave_write(host, write_data_B, source_data_B, DATA_LENGTH_B) != SPI_FLAG_SUCCESS) {
        PRINTF("Failure to write\n\r");
        return EXIT_FAILURE;
    }

    for(int i=0; i < DATA_LENGTH_B; i++){
        if(source_data_B[i] != write_data_B[i]) return EXIT_FAILURE;
        write_data_B[i] = i+1;
    }

    if( spi_slave_read(host, write_data_B, read_data_B, DATA_LENGTH_B, DUMMY_CYCLES) != SPI_FLAG_SUCCESS) {
        PRINTF("Failure to read\n\r");
        return EXIT_FAILURE;
    }

    for(int i=0; i < DATA_LENGTH_B; i++){
        if(source_data_B[i] != write_data_B[i]) return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}





