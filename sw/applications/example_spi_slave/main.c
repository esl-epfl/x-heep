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


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 


#define DATA_LENGTH_W 20
#define DATA_LENGTH_B 21
#define DUMMY_CYCLES  32

uint32_t source_data_W[DATA_LENGTH_W] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
uint8_t  source_data_B[DATA_LENGTH_B] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };


/** Arrays used for testing. */
uint32_t write_data_W[DATA_LENGTH_W+1] = {99, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123};
uint8_t write_data_B[DATA_LENGTH_B];

/** Arrays used for testing. */
uint32_t read_data[DATA_LENGTH_W];
uint8_t read_data_B[DATA_LENGTH_B] = {170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170};


static spi_host_t* host; 

int main(){

    host = spi_host1;
    spi_return_flags_e flags;

    if( spi_host_init(host)!= SPI_FLAG_SUCCESS) {
        PRINTF("Failure to initialize\n\r");
        return EXIT_FAILURE;
    }

    // TEST WRITE
    if( spi_slave_write(host, write_data_B, source_data_B, DATA_LENGTH_B) != SPI_FLAG_SUCCESS) {
        PRINTF("Failure to write\n\r");
        return EXIT_FAILURE;
    }

    for(int i=0; i < DATA_LENGTH_B; i++){
        // PRINTF("%d==%d%s\n\r", source_data_B[i], write_data_B[i], source_data_B[i] == write_data_B[i] ? "" : " X");
        // if(source_data_B[i] != write_data_B[i]) return EXIT_FAILURE;
        write_data_B[i] = i;
    }
    write_data_B[DATA_LENGTH_B-1]   = 255;
    read_data_B[DATA_LENGTH_B]      = 123;
    read_data_B[DATA_LENGTH_B+1]    = 124;
    read_data_B[DATA_LENGTH_B+2]    = 125;


    if( spi_slave_read(host, write_data_B, read_data_B, DATA_LENGTH_B, DUMMY_CYCLES) != SPI_FLAG_SUCCESS) {
        PRINTF("Failure to read\n\r");
        return EXIT_FAILURE;
    }

    for(int i=19; i < DATA_LENGTH_B+4; i++){
        // PRINTF(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY(write_data_B[i]) );
        // PRINTF("=");
        // PRINTF(BYTE_TO_BINARY_PATTERN,BYTE_TO_BINARY(read_data_B[i]) );
        // PRINTF("\n\r");
        PRINTF("%d=%d\n\r",write_data_B[i],read_data_B[i]);
        // if(source_data_B[i] != write_data_B[i]) return EXIT_FAILURE;
    }

    PRINTF("read");

    return EXIT_SUCCESS;
}





