// Copyright 2025 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: spi_slave_sdk.c
// Author: Juan Sapriza
// Description: This is not an actual SDK. The SPI slave cannot be controlled from 
// software. This "misleading file" is using the SPI Host SDK to read and write 
// to the SPI slave.


#include "spi_slave_sdk.h"


/*
* Initilize the SPI Host
*/
spi_flags_e spi_host_init(spi_host_t* host) {
 
    // Enable spi host device
    if( spi_set_enable(host, true) != SPI_FLAG_SUCCESS) return SPI_HOST_FLAG_NOT_INIT;
    if( spi_output_enable(host, true) != SPI_FLAG_SUCCESS) return SPI_HOST_FLAG_NOT_INIT;

    // Spi Configuration
    // Configure chip 0 (slave)
    const uint32_t chip_cfg = spi_create_configopts((spi_configopts_t){
        .clkdiv     = 0,
        .csnidle    = 0xF,
        .csntrail   = 0xF,
        .csnlead    = 0xF,
        .fullcyc    = false,
        .cpha       = 0,
        .cpol       = 0            
    });
    spi_set_configopts(host, 0, chip_cfg);;

    if(spi_set_csid(host, 0) != SPI_FLAG_SUCCESS) return SPI_HOST_FLAG_CSID_INVALID;

    return SPI_FLAG_SUCCESS; // Success
}

/**
 * @brief Command the SPI slave to write in memory
 * @param host The SPI host instance
 * @param write_addr uint8_t Address in the slave's memory where the data will be written. 
 * @param read_ptr uint8_t pointer to memory where the data is read from. 
 * @param length_B Length (in bytes) of the data to be copied.
 * @return The number of 32-bit words that were requested to write. 
*/
spi_flags_e spi_slave_write(spi_host_t* host, uint8_t* write_addr, uint8_t* read_ptr, uint16_t length_B) {

    uint32_t length_w       = length_B >> 2;    // The length in bytes is converted to length in 32-bit words
    uint8_t remaining_bytes = length_B % 4;     // The bytes that do not fit in full words are managed separately
    uint32_t length_w_tx    = remaining_bytes ? length_w +1 : length_w;  // We will request the SPI to write an extra word if there are remaining bytes

    spi_slave_send_wrap_length(host, length_w_tx);
    spi_slave_send_dir(host, SPI_SLAVE_CMD_WRITE);
    spi_slave_send_address(host,write_addr );
    
    /*
     * Place read_ptr in TX FIFO
     * We fill the FIFO of the SPI host and then flush every 72 words (the depth of the fifo).
    */
    uint16_t words_in_fifo = 0;
    for (uint16_t i = 0; i < length_w; i++) {
        if( words_in_fifo == SPI_HOST_PARAM_TX_DEPTH){
            send_command_to_spi_host(host, SPI_HOST_PARAM_TX_DEPTH*4, true, SPI_DIR_TX_ONLY);
            words_in_fifo = 0;
        }
        spi_wait_for_tx_not_full(host);
        spi_write_word(host, REVERT_ENDIANNESS( ((uint32_t *)read_ptr)[i]));
        words_in_fifo++;
    }
    
     /*
     * The remaining bytes are added to an extra word. The copied word from memory is cleaned with a mask before sending.
    */
    if ( remaining_bytes ) {
        uint32_t mask = (1 << (8 * remaining_bytes)) - 1; // Only keep the remaining bytes with a mask
        uint32_t last_word = ((uint32_t*)read_ptr)[length_w] & mask;
        spi_wait_for_tx_not_full(host);
        spi_write_word(host,  REVERT_ENDIANNESS( last_word ) );
    }

    // SPI host cannot send individual bytes, so we will send the words available at the fifo and if there are any remaining bytes we add one extra full word. 
    send_command_to_spi_host(host, (words_in_fifo+ (uint16_t)(remaining_bytes != 0))*4 , false, SPI_DIR_TX_ONLY);
    spi_wait_for_tx_empty(host);
    return SPI_FLAG_SUCCESS;
}

/**
 * @brief Command the SPI slave to read from memory. Then you need to call the spi_copy_x functions to 
 * access the copied data from the SPI Host's FIFO.
 * @param host The SPI host instance
 * @param read_address uint8_t Address in the slave's memory from where to read. 
 * @param length_B Length (in bytes) of the data to be copied.
 * @return The number of 32-bit words that were requested to read. 
*/
uint16_t spi_slave_request_read( spi_host_t* host, uint8_t* read_address, uint16_t length_B, uint8_t dummy_cycles ){
    uint8_t remaining_bytes = length_B % 4;
    uint32_t length_w       = length_B >> 2;
    length_w                = remaining_bytes ? length_w +1 : length_w; 

    spi_slave_send_dummy_cycles(host, dummy_cycles);
    spi_slave_send_wrap_length(host, length_w);
    spi_slave_send_dir(host, SPI_SLAVE_CMD_READ);
    spi_slave_send_address( host, read_address );

    send_command_to_spi_host(host, dummy_cycles, true, SPI_DIR_DUMMY);
    send_command_to_spi_host(host, length_w*4, false, SPI_DIR_RX_ONLY);

    uint16_t to_read_w = (length_w >= SPI_HOST_PARAM_RX_DEPTH>>2) ? SPI_HOST_PARAM_RX_DEPTH>>2 : length_w;
    spi_set_rx_watermark(host, to_read_w);
    return to_read_w;
}

/**
 * @brief Specify to the SPI slave how many dummy cycles to leave between receiving the read address and sending the data. 
 * @param host The SPI host instance
 * @param dummy_cycles The number of SCK cycles to leave. Min=7, Max=255. If this function is not called, the default is 32.
*/
void spi_slave_send_dummy_cycles( spi_host_t* host, uint8_t dummy_cycles ){
    uint32_t command = (WRITE_SPI_SLAVE_REG_0) | ((dummy_cycles & 0xFF) << 8);           
    spi_write_word(host, command);
    spi_wait_for_ready(host);
    send_command_to_spi_host(host, 2, true, SPI_DIR_TX_ONLY);
}

/**
 * @brief Specify to the SPI slave how many 32-bit words will be read. 
 * @param host The SPI host instance
 * @param length_w The length in words to be read. .
*/
void spi_slave_send_wrap_length( spi_host_t* host,uint16_t length_w ){
    uint32_t command = (WRITE_SPI_SLAVE_REG_1)               // Move to the lowest byte
                        | ((length_w & 0xFF) << 8)           // Convert Length in bytes to length in words and move to the second lowest byte
                        | (WRITE_SPI_SLAVE_REG_2 << 16)      // Move to the second highest byte
                        | (((length_w >> 8) & 0xFF) << 24);  // Convert Length in bytes to length in words and move to the highest byte
    spi_write_word(host, command);
    spi_wait_for_ready(host);
    send_command_to_spi_host(host, 4, true, SPI_DIR_TX_ONLY);
}

/**
 * @brief Send the order to start a read/write transaction. You later need to call spi_slave_send_address
 * @param host The SPI host instance
 * @param dir The transaction's direction: either SPI_SLAVE_CMD_READ or SPI_SLAVE_CMD_WRITE
*/
void spi_slave_send_dir( spi_host_t* host, uint8_t dir ){
    spi_write_byte(host, dir);
    spi_wait_for_ready(host);
    send_command_to_spi_host(host, 1, true, SPI_DIR_TX_ONLY);
}

/**
 * @brief Send the read/write address to the SPI slave. This command will trigger the beginning of the transaction.
 * The SPI slave will wait for the specified dummy cycles before sending the data. 
 * @param host The SPI host instance
*/
void spi_slave_send_address( spi_host_t* host, uint8_t* address ){
    spi_write_word(host, REVERT_ENDIANNESS((uint32_t)address));
    spi_wait_for_ready(host); 
    send_command_to_spi_host(host, 4, true, SPI_DIR_TX_ONLY);
}


/**
 * @brief Copy words from the SPI host Rx buffer into a pointer. 
 *
 * @param host The SPI host instance
 * @param write_ptr Pointer to uint32_t buffer where the words will be copied to. 
 * @param words Number of words to be copied.
*/
void spi_copy_words( spi_host_t* host, uint32_t* write_ptr, uint16_t words){
    uint32_t data_32bit;
    for (uint16_t i = 0; i < words; i++) {
        spi_read_word(host, &data_32bit); // Reads a full word
        write_ptr[i] = REVERT_ENDIANNESS(data_32bit);
    }
}

/**
 * @brief Read a single word from the SPI host Rx buffer. 
 * @param host The SPI host instance
 * @return The read word 
*/
uint32_t spi_copy_word( spi_host_t* host){
    uint32_t word;
    spi_read_word(host, &word); // Reads a full word
    word = REVERT_ENDIANNESS(word);
    return word;
}

/**
 * @brief Read a single byte from the SPI host Rx buffer. Note that 
 * other bytes in the element popped from the FIFO are lost!  
 * @param host The SPI host instance
 * @param index  The position of the byte inside the word. 0 = 8 less sign. bits
 * @return The read word 
*/
uint8_t spi_copy_byte(spi_host_t* host, uint8_t index){
    uint32_t word;
    uint8_t byte;
    spi_read_word(host, &word); // Writes a full word
    word = (word >> (3-index)*8) & 255; //Take only the requested byte, considering the endianness.
    return (uint8_t)word;
}

/**
 * @brief Add a command for the SPI host to send  
 * @param host The SPI host instance
 * @param length_B  The length in bytes of the command, or the length of the data to be sent. 
 * @param csaat True if the command is not finished. 
 * @param direction Directionality of the command
 * @return The read word 
*/
void send_command_to_spi_host(spi_host_t* host, uint32_t length_B, bool csaat, spi_dir_e direction){
    if(direction != SPI_DIR_DUMMY){
        length_B--; //The SPI HOST IP uses length_B-1 = amount of bytes to read and write. But also length_B = amount of dummy cycles
    }
    const uint32_t send_cmd_w = spi_create_command((spi_command_t){
        .len        = length_B,                     
        .csaat      = csaat,                    // Command not finished e.g. CS remains low after transaction
        .speed      = SPI_SPEED_STANDARD,       // Single speed
        .direction  = direction
    });
    spi_set_command(host, send_cmd_w);
    spi_wait_for_ready(host);
}

