/**
 * @file main.c
 * @brief spi obi slave functionality test
 *
 *
*/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "x-heep.h"
#include "spi_host.h"

#define DIV_ROUND_UP(numerator, denominator) ((numerator + denominator - 1) / denominator)
#define LOWER_BYTE_16_BITS(bytes)(bytes & 0xFF)
#define UPPER_BYTE_16_BITS(bytes)((bytes & 0xFF00) >> 8)
#define WRITE_SPI_SLAVE_REG_2 0x20
#define WRITE_SPI_SLAVE_REG_3 0x30
#define READ_SPI_SLAVE_CMD 0xB


typedef enum {
    // Everithing went well
    SPI_SLAVE_FLAG_OK                 = 0x0000,
    // The SPI variabled passed was a null pointer
    SPI_SLAVE_FLAG_NULL_PTR           = 0x0001,
    //The SPI was not properly initalized
    SPI_SLAVE_FLAG_NOT_INIT           = 0x0002,
    //The target address is invalid
    SPI_SLAVE_FLAG_ADDRESS_INVALID    = 0x0003,
    // The CSID was out of the bounds specified inSPI_HOST_PARAM_NUM_C_S 
    SPI_SLAVE_FLAG_CSID_INVALID       = 0x0004,
    // The CMD FIFO is currently full so couldn't write command
    SPI_SLAVE_FLAG_COMMAND_FULL       = 0x0008,
    // The specified speed is not valid so couldn't write command
    SPI_SLAVE_FLAG_SPEED_INVALID      = 0x0010,
    // The TX Queue is full, thus could not write to TX register
    SPI_SLAVE_FLAG_TX_QUEUE_FULL      = 0x0020,
    // The RX Queue is empty, thus could not read from RX register
    SPI_SLAVE_FLAG_RX_QUEUE_EMPTY     = 0x0040,
    // The SPI is not ready
    SPI_SLAVE_FLAG_NOT_READY          = 0x0080,
    // The event to enable is not a valid event
    SPI_SLAVE_FLAG_EVENT_INVALID      = 0x0100,
    // The error irq to enable is not a valid error irq
    SPI_SLAVE_FLAG_ERROR_INVALID      = 0x0200 
} spi_slave_flags_e;

spi_host_t* spi; //Removed specific memory assignement


spi_slave_flags_e spi_slave_init(spi_host_t* spi_host);
void spi_host_write();
spi_slave_flags_e spi_slave_read(uint32_t addr, void* data, uint16_t length);
bool check_address_validity(uint32_t addr, void* data, uint16_t length);


int main(int argc, char *argv[])
{   
    spi_return_flags_e flags;
    flags = spi_slave_init(spi);
    if (flags != SPI_FLAG_OK){
        printf("Failure to initialize\n Error code: %s", flags);
        return EXIT_FAILURE;
    }




    return EXIT_SUCCESS;
}

void spi_host_write(){
};

spi_slave_flags_e spi_slave_read(uint32_t addr, void* data, uint16_t length){
    // ??? Is this necessary? What happens when address is invalid or data exceeds memory? What is the size of the memory?
    if (check_address_validity(addr, data, length) != true) return SPI_SLAVE_FLAG_ADDRESS_INVALID;


    uint32_t wrap_length_cmd    = (WRITE_SPI_SLAVE_REG_2)                   //Write register 2 
                                + ((LOWER_BYTE_16_BITS(length)) << 8)       //Wraplength low 
                                + (WRITE_SPI_SLAVE_REG_3 << 16)             //Write register 3
                                + (UPPER_BYTE_16_BITS(length) << 24);       //Wraplength high

    // Load command to TX FIFO
    spi_write_word(spi, wrap_length_cmd);
    spi_wait_for_ready(spi);

    // Set up segment parameters -> send commands
    const uint32_t send_cmd_byte = spi_create_command((spi_command_t){
        .len        = 1,                    // 1 Byte (The SPI SLAVE IP can only take one CMD byte at a time)
        .csaat      = true,                 // Command not finished e.g. CS remains low after transaction
        .speed      = SPI_SPEED_STANDARD,   // Single speed
        .direction  = SPI_DIR_TX_ONLY       // Write only
    });

    for(int i = 0; i < 4; i++){ //To test but it might be possible to delete this and put .len in send_cmd_byte to 4. But not sure if that works as intended
        // Load segment parameters to COMMAND register
        spi_set_command(spi, send_cmd_byte);
        spi_wait_for_ready(spi);
    }

    spi_write_byte(spi, READ_SPI_SLAVE_CMD);
    spi_wait_for_ready(spi);

    spi_set_command(spi, send_cmd_byte);
    spi_wait_for_ready(spi);

    //write address
    spi_write_word(spi, addr);
    spi_wait_for_ready(spi);

    const uint32_t send_cmd_word = spi_create_command((spi_command_t){
      .len        = 4,                     // 4 Bytes 
      .csaat      = true,                  // Command not finished e.g. CS remains low after transaction
      .speed      = SPI_SPEED_STANDARD,    // Single speed
      .direction  = SPI_DIR_TX_ONLY        // Write only
    });
    spi_set_command(spi, send_cmd_word);
    spi_wait_for_ready(spi);


    // Set up segment parameters -> read length bytes
    const uint32_t cmd_read = spi_create_command((spi_command_t){
        .len        = length-1,             // len bytes
        .csaat      = false,                // End command
        .speed      = SPI_SPEED_STANDARD,   // Single speed
        .direction  = SPI_DIR_RX_ONLY       // Read only
    });
    spi_set_command(spi, cmd_read);
    spi_wait_for_ready(spi);

    /*
     * Set RX watermark to length. The watermark is in words.
     * If the length is not a multiple of 4, the RX watermark is set to length/4+1
     * to take into account the extra bytes.
     * If the length is higher then the RX FIFO depth, the RX watermark is set to
     * RX FIFO depth. In this case the flag is not set to 0, so the loop will
     * continue until all the data is read.
    */
    int flag = 1;
    int to_read = 0;
    int i_start = 0;
    int length_original = length;
    uint32_t *data_32bit = (uint32_t *)data;
    while (flag) {
        if (length >= SPI_HOST_PARAM_RX_DEPTH) {
            spi_set_rx_watermark(spi, SPI_HOST_PARAM_RX_DEPTH>>2);
            length -= SPI_HOST_PARAM_RX_DEPTH;
            to_read += SPI_HOST_PARAM_RX_DEPTH;
        }
        else {
            spi_set_rx_watermark(spi, (length%4==0 ? length>>2 : (length>>2)+1));
            to_read += length;
            flag = 0;
        }
        // Wait till SPI RX FIFO is full (or I read all the data)
        spi_wait_for_rx_watermark(spi);
        // Read data from SPI RX FIFO
        for (int i = i_start; i < to_read>>2; i++) {
            spi_read_word(spi, &data_32bit[i]); // Writes a full word
        }
        // Update the starting index
        i_start += SPI_HOST_PARAM_RX_DEPTH>>2;
    }
    // Take into account the extra bytes (if any)
    if (length_original % 4 != 0) {
        uint32_t last_word = 0;
        spi_read_word(spi, &last_word);
        memcpy(&data_32bit[length_original>>2], &last_word, length%4);
    }

    return SPI_SLAVE_FLAG_OK; // Success
};


spi_slave_flags_e spi_slave_init(spi_host_t* spi_host) {
 
    // Set the global spi variable to the one passed as argument.
    spi = spi_host;

    // Enable SPI host device
    if(spi_set_enable(spi, true) != SPI_FLAG_OK){
        return SPI_SLAVE_FLAG_NOT_INIT;
    };

    // Enable SPI output
    if(spi_output_enable(spi, true) != SPI_FLAG_OK){
        return SPI_SLAVE_FLAG_NOT_INIT;
    };
    // Configure SPI Master<->Slave connection on CSID 0
    configure_spi();

    // Set CSID
    if(spi_set_csid(spi, 0) != SPI_FLAG_OK){
        return SPI_SLAVE_FLAG_CSID_INVALID;
    };

    return SPI_SLAVE_FLAG_OK; // Success
}

bool check_address_validity(uint32_t addr, void* data, uint16_t length){
    //TODO
    return true;
};
