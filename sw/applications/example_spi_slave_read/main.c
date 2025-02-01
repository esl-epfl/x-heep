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

#define DUMMY_CYCLES  32

#define DATA_LENGTH_B   200
#define DATA_CHUNK_W    1
#define DATA_CHUNK_B    1
#define CHUNKS_NW       (DATA_LENGTH_B/(DATA_CHUNK_W*4)) + ((DATA_LENGTH_B%(DATA_CHUNK_W*4))!=0)
#define CHUNKS_NB       (DATA_LENGTH_B/DATA_CHUNK_B)

uint8_t buffer_src[DATA_LENGTH_B] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
    61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 
    91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 
    121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 
    151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 
    181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200
};
uint8_t buffer_dst[DATA_LENGTH_B];

int main(){
    uint16_t i;

    if( spi_host_init(spi_host1)!= SPI_FLAG_SUCCESS) return EXIT_FAILURE;

    uint8_t byte_by_byte_length_b = 10;

    for( i=0; i<byte_by_byte_length_b; i++){
        spi_slave_request_read(spi_host1,&buffer_src[i],  1, DUMMY_CYCLES );
        spi_wait_for_rx_watermark(spi_host1);
        buffer_dst[i] = spi_copy_byte(spi_host1, i%4 );
    }

    for(int i=0; i < byte_by_byte_length_b; i++){
        if(buffer_src[i] != buffer_dst[i]) return EXIT_FAILURE;
    }
    PRINTF("B/B OK\n\r");


    uint8_t word_by_word_length_w = 10;
    uint8_t start_w = byte_by_byte_length_b/4;

    for( i=0; i<word_by_word_length_w; i++){
        spi_slave_request_read(spi_host1,&((uint32_t*)buffer_src)[start_w + i],  4, DUMMY_CYCLES );
        spi_wait_for_rx_watermark(spi_host1);
        ((uint32_t*)buffer_dst)[ start_w + i ] = spi_copy_word(spi_host1);
    }

    for(int i=start_w; i < start_w+word_by_word_length_w*4; i++){
        if(buffer_src[i] != buffer_dst[i]) return EXIT_FAILURE;
    }
    PRINTF("W/W OK\n\r");

    uint8_t chunk_w     = 5;
    uint8_t chunks_n    = 5;
    start_w               += word_by_word_length_w;

    for( i=0; i<chunks_n; i++){
        spi_slave_request_read(spi_host1,&((uint32_t*)buffer_src)[start_w + i*chunk_w],  chunk_w*4, DUMMY_CYCLES );
        spi_wait_for_rx_watermark(spi_host1);
        spi_copy_words( spi_host1, &((uint32_t*)buffer_dst)[start_w + i*chunk_w],  chunk_w );
    }

    for( i=start_w*4; i < start_w*4 + chunks_n*chunk_w*4; i++){
        if(buffer_src[i] != buffer_dst[i]) return EXIT_FAILURE;
    }
    PRINTF("C/C OK\n\r");

    return EXIT_SUCCESS;
}





