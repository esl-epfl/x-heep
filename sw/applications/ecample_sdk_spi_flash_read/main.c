/**
 * @file main.c
 * @brief Simple spi read example using SDK
 *
 * Simple example that reads ...
 *
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "x-heep.h"
#include "spi_sdk.h"
#include "soc_ctrl_structs.h"
#include "bitfield.h"

/* By default, PRINTFs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#ifdef TARGET_PYNQ_Z2
    #define USE_SPI_FLASH
#endif

int main(int argc, char *argv[]) {
    spi_slave_t slave = {
        .csid = 0,
        .csn_idle = 15,
        .csn_lead = 15,
        .csn_trail = 15,
        .data_mode = SPI_DATA_MODE_0,
        .full_cycle = false,
        .freq = soc_ctrl_peri->SYSTEM_FREQUENCY_HZ / 4
    };
    spi_t spi = spi_init(SPI_IDX_FLASH, slave);

    if (!spi.init) {
        PRINTF("Failed to initialize spi\n");
        return -1;
    }

    spi_segment_t segments[2] = {SPI_SEG_TX(4),SPI_SEG_RX(40)};
    uint32_t read_byte_cmd = ((bitfield_byteswap32(10 & 0x00ffffff)) | 0x03);

    uint32_t rxbuffer[10];

    spi_transaction_t txn = {
        .segments = segments,
        .seglen = 2,
        .txbuffer = &read_byte_cmd,
        .txlen = 1,
        .rxbuffer = rxbuffer,
        .rxlen = 10
    };

    spi_codes_e error = spi_execute(&spi, txn);
    if (!error) {
        PRINTF("Failed to execute command\n");
        PRINTF("Error Code: %i", error);
        return -1;
    }

    PRINTF("SUCCESS\n\n");

    for (int i = 0; i < 10; i++)
    {
        PRINTF("0x%032X\n", rxbuffer[i]);
    }
    return 0;
}