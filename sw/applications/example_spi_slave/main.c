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

#define DATA_LENGTH_W 20
#define DATA_LENGTH_B 21

uint32_t source_data_W[DATA_LENGTH_W] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
uint8_t  source_data_B[DATA_LENGTH_B] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };


/** Arrays used for testing. */
uint32_t write_data_W[DATA_LENGTH_W+1] = {99, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123};
uint8_t write_data_B[DATA_LENGTH_B];

/** Arrays used for testing. */
uint32_t read_data[DATA_LENGTH_W];



static spi_host_t* host; 

int main(){

    host = spi_host1;
    spi_return_flags_e flags;

    if( spi_host_init(host)!= SPI_FLAG_SUCCESS) {
        PRINTF("Failure to initialize\n Error code: %d", flags);
        return EXIT_FAILURE;
    }


    // TEST WRITE
    if( spi_slave_write(host, write_data_B, source_data_B, DATA_LENGTH_B) != SPI_FLAG_SUCCESS) {
        PRINTF("Failure to write\n Error code: %d", flags);
        return EXIT_FAILURE;
    }

    for(int i=19; i < DATA_LENGTH_B+4; i++){
        PRINTF("%d==%d%s\n\r", source_data_B[i], write_data_B[i], source_data_B[i] == write_data_B[i] ? "" : " X");
        // if(source_data_B[i] != write_data_B[i]) return EXIT_FAILURE;
        //write_data[i] += 1;
    }



    PRINTF("ok\n");
    return EXIT_SUCCESS;
}





