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
 *
 * @param host The SPI host instance
 * @param write_addr uint8_t Address in the slave's memory where the data will be written. 
 * @param read_ptr uint8_t pointer to memory where the data is read from. 
 * @param length_B Length (in bytes) of the data to be copied.
 * @return SPI_FLAG_SUCCESS always
*/
spi_flags_e spi_slave_write(spi_host_t* host, uint8_t* write_addr, uint8_t* read_ptr, uint16_t length_B) {

    uint32_t length_W       = length_B >> 2;    // The length in bytes is converted to length in 32-bit words
    uint8_t remaining_bytes = length_B % 4;     // The bytes that do not fit in full words are managed separately
    uint32_t length_W_tx    = remaining_bytes ? length_W +1 : length_W;  // We will request the SPI to write an extra word if there are remaining bytes

    // Write the wrap length: How many words will be sent
    uint32_t wrap_length_W_cmds =   (WRITE_SPI_SLAVE_REG_1)                 // Move to the lowest byte
                                    | ((length_W_tx & 0xFF) << 8)           // Convert Length in bytes to length in words and move to the second lowest byte
                                    | (WRITE_SPI_SLAVE_REG_2 << 16)         // Move to the second highest byte
                                    | (((length_W_tx >> 8) & 0xFF) << 24);  // Convert Length in bytes to length in words and move to the highest byte

    // Send the wrap length (how many words are going to be sent)
    spi_write_word(host, wrap_length_W_cmds);                   // The value is added to the buffer
    spi_wait_for_ready(host);                                   // Blockingly wait until the SPI host is free
    send_command_to_spi_host(host, 4, true, SPI_DIR_TX_ONLY);   // Send the command. One full word. 

    // Send the command to instruct the slave that it will be writing in memory
    spi_write_byte(host, WRITE_SPI_SLAVE_CMD);                  // The value is added to the buffer 
    spi_wait_for_ready(host);                                   // Blockingly wait until the SPI host is free
    send_command_to_spi_host(host, 1, true, SPI_DIR_TX_ONLY);   // Send the command. One full word. 

    ///write the address in memory where the data needs to be written to
    spi_write_word(host, REVERT_ENDIANNESS((uint32_t)write_addr));    // The value is added to the buffer
    spi_wait_for_ready(host);                                   // Blockingly wait until the SPI host is free
    send_command_to_spi_host(host, 4, true, SPI_DIR_TX_ONLY);   // Send the command. One full word. 

 
    /*
     * Place read_ptr in TX FIFO
     * We fill the FIFO of the SPI host and then flush every 72 words (the depth of the fifo).
    */
    uint16_t words_in_fifo = 0;
    for (uint16_t i = 0; i < length_W; i++) {
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
        uint32_t last_word = ((uint32_t*)read_ptr)[length_W] & mask;
        spi_wait_for_tx_not_full(host);
        spi_write_word(host,  REVERT_ENDIANNESS( last_word ) );
    }

    // SPI host cannot send individual bytes, so we will send the words available at the fifo and if there are any remaining bytes we add one extra full word. 
    send_command_to_spi_host(host, (words_in_fifo+ (uint16_t)(remaining_bytes != 0))*4 , false, SPI_DIR_TX_ONLY);
    spi_wait_for_tx_empty(host);
    return SPI_FLAG_SUCCESS;
}

/**
 * @brief Command the SPI slave to write in memory
 *
 * @param host The SPI host instance
 * @param read_address uint8_t Address in the slave's memory where the data will be read from. 
 * @param write_ptr uint8_t pointer to memory where the data is written to. 
 * @param length_B Length (in bytes) of the data to be copied.
 * @return SPI_FLAG_SUCCESS always
*/
spi_flags_e spi_slave_read(spi_host_t* host, uint8_t* read_address, uint8_t* write_ptr, uint16_t length_B, uint8_t dummy_cycles){

    uint32_t length_W           = length_B >> 2;
    uint8_t remaining_bytes     = length_B % 4;
    uint32_t length_W_rx        = remaining_bytes ? length_W +1 : length_W; 
    uint32_t wrap_length_W_cmds =   (WRITE_SPI_SLAVE_REG_1)                     // Move to the lowest byte
                                    | ((length_W_rx & 0xFF) << 8)           // Convert Length in bytes to length in words and move to the second lowest byte
                                    | (WRITE_SPI_SLAVE_REG_2 << 16)             // Move to the second highest byte
                                    | (((length_W_rx >> 8) & 0xFF) << 24);        // Convert Length in bytes to length in words and move to the highest byte


    spi_write_word(host, wrap_length_W_cmds);
    spi_wait_for_ready(host);
    send_command_to_spi_host(host, 4, true, SPI_DIR_TX_ONLY);
 
    spi_write_byte(host, READ_SPI_SLAVE_CMD);
    spi_wait_for_ready(host);
    send_command_to_spi_host(host, 1, true, SPI_DIR_TX_ONLY);

    //write address
    spi_write_word(host, REVERT_ENDIANNESS((uint32_t)read_address));
    spi_wait_for_ready(host); 
    send_command_to_spi_host(host, 4, true, SPI_DIR_TX_ONLY);

    if(dummy_cycles) send_command_to_spi_host(host, dummy_cycles, true, SPI_DIR_DUMMY);

    send_command_to_spi_host(host, length_W_rx*4, false, SPI_DIR_RX_ONLY);

    /*
     * Set RX watermark to length_B. The watermark is in words.
     * If the length_B is not a multiple of 4, the RX watermark is set to length_B/4+1
     * to take into account the extra bytes.
     * If the length_B is higher then the RX FIFO depth, the RX watermark is set to
     * RX FIFO depth. In this case the there_is_data_to_read is not set to 0, so the loop will
     * continue until all the write_ptr is read.
    */
    uint16_t remaining_W = length_W_rx;
    uint16_t to_read_W = 0;
    uint16_t i_start = 0;
    uint16_t length_B_original = length_B;
    uint32_t data_32bit;
    
    while (remaining_W) {

        to_read_W = (remaining_W >= SPI_HOST_PARAM_RX_DEPTH>>2) ? SPI_HOST_PARAM_RX_DEPTH >> 2 : remaining_W;
        spi_set_rx_watermark(host, to_read_W);
        spi_wait_for_rx_watermark(host);
        // Read write_ptr from SPI Host RX FIFO
        // If there are remaining bytes (# bytes to read is not multiple of 4), do not read the last word
        for (uint16_t i = length_W_rx - remaining_W; i < to_read_W - (remaining_bytes!=0); i++) {
            spi_read_word(host, &data_32bit); // Writes a full word
            ((uint32_t *)write_ptr)[i] = REVERT_ENDIANNESS(data_32bit);
        }
        remaining_W -= to_read_W;
    }

    if( remaining_bytes ){
        // // Always treat the last word with extra care in case we were not copying a full 32-bit word. 
        spi_read_word(host, &data_32bit);

        data_32bit = REVERT_ENDIANNESS(data_32bit);
        uint32_t mask = (1 << (8 * remaining_bytes)) - 1; // Only keep the remaining bytes with a mask
        data_32bit = data_32bit & mask;
        ((uint32_t*)write_ptr)[length_W_rx-1] &= ~mask;
        ((uint32_t*)write_ptr)[length_W_rx-1] |= data_32bit;    
    }

    return SPI_FLAG_SUCCESS; // Success
}


void send_command_to_spi_host(spi_host_t* host, uint32_t length_B, bool csaat, uint8_t direction){
    if(direction != SPI_DIR_DUMMY){
        length_B--; //The SPI HOST IP uses length_B-1 = amount of bytes to read and write. But also length_B = amount of dummy cycles
    }
    const uint32_t send_cmd_W = spi_create_command((spi_command_t){
        .len        = length_B,                     
        .csaat      = csaat,                    // Command not finished e.g. CS remains low after transaction
        .speed      = SPI_SPEED_STANDARD,       // Single speed
        .direction  = direction
    });
    spi_set_command(host, send_cmd_W);
    spi_wait_for_ready(host);
}

