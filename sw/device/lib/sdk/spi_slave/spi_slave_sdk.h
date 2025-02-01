// Copyright 2025 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: spi_slave_sdk.h
// Author: Juan Sapriza
// Description: This is not an actual SDK. The SPI slave cannot be controlled from 
// software. This "misleading file" is using the SPI Host SDK to read and write 
// to the SPI slave.


#include "spi_host.h"

#define REVERT_ENDIANNESS(x) ( \
    ((x & 0x000000FF) << 24) | \
    ((x & 0x0000FF00) << 8)  | \
    ((x & 0x00FF0000) >> 8)  | \
    ((x & 0xFF000000) >> 24)   \
)


/** Macros for SPI SLAVE hardware. */
#define WRITE_SPI_SLAVE_REG_0   0x11    // Used to specify the amount of dummy cycles
#define WRITE_SPI_SLAVE_REG_1   0x20    // To store the 8 LSBs of the wrap length
#define WRITE_SPI_SLAVE_REG_2   0x30    // To store the 8 MSBs of the wrap length
#define READ_SPI_SLAVE_CMD      0xB     // Command the SPI slave to READ from memory and send data out
#define WRITE_SPI_SLAVE_CMD     0x2     // Command the SPI slave to receive data and WRITE it to memory


/** Enum for SPI operation status flags. */
typedef enum {
    // Everithing went well
    SPI_FLAG_SUCCESS                        = 0x0000,        
    //The SPI host was not properly initalized
    SPI_HOST_FLAG_NOT_INIT                  = 0x0001,   
    //The target address is invalid
    SPI_SLAVE_FLAG_ADDRESS_INVALID          = 0x0002,
    // The CSID was out of the bounds specified in SPI_HOST_PARAM_NUM_C_S 
    SPI_HOST_FLAG_CSID_INVALID              = 0x0003,    
    //The amount of data exceeds the memory capacity of the SPI SLAVE (X-HEEP)
    SPI_SLAVE_FLAG_SIZE_OF_DATA_EXCEEDED    = 0x0004, 
} spi_flags_e;

spi_flags_e spi_host_init(spi_host_t* host);
spi_flags_e spi_slave_write(spi_host_t* host, uint8_t* write_addr, uint8_t* read_ptr, uint16_t length_B);
uint16_t spi_slave_request_read( spi_host_t* host, uint8_t* read_address, uint16_t length_B, uint8_t dummy_cycles );
void spi_copy_words( spi_host_t* host, uint32_t* write_ptr, uint16_t words);
uint32_t spi_copy_word( spi_host_t* host);
uint8_t spi_copy_byte(spi_host_t* host, uint8_t index);
void send_command_to_spi_host(spi_host_t* host, uint32_t len, bool csaat, spi_dir_e direction);