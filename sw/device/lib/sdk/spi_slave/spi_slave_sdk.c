#include "spi_slave_sdk.h"



spi_flags_e spi_host_init(spi_host_t* host) {
 
    // Enable spi host device
    if( spi_set_enable(host, true) != SPI_FLAG_SUCCESS) return SPI_HOST_FLAG_NOT_INIT;
    if(spi_output_enable(host, true) != SPI_FLAG_SUCCESS) return SPI_HOST_FLAG_NOT_INIT;

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

spi_flags_e spi_slave_write(spi_host_t* host, uint8_t* addr, uint8_t* data, uint16_t length_B) {

    uint32_t length_W       = length_B >> 2;
    uint8_t remaining_bytes = length_B % 4;
    uint32_t length_W_tx    = remaining_bytes ? length_W +1 : length_W;  

    uint32_t wrap_length_W_cmds =   (WRITE_SPI_SLAVE_REG_1)                     // Move to the lowest byte
                                    | ((length_W_tx & 0xFF) << 8)           // Convert Length in bytes to length in words and move to the second lowest byte
                                    | (WRITE_SPI_SLAVE_REG_2 << 16)             // Move to the second highest byte
                                    | (((length_W_tx >> 8) & 0xFF) << 24);        // Convert Length in bytes to length in words and move to the highest byte

    spi_write_word(host, wrap_length_W_cmds);
    spi_wait_for_ready(host);
    send_command_to_spi_host(host, 4, true, SPI_DIR_TX_ONLY);

    spi_write_byte(host, WRITE_SPI_SLAVE_CMD);
    spi_wait_for_ready(host); 
    send_command_to_spi_host(host, 1, true, SPI_DIR_TX_ONLY);

    ///write address
    spi_write_word(host, REVERT_ENDIANNESS((uint32_t)addr));
    spi_wait_for_ready(host); 
    send_command_to_spi_host(host, 4, true, SPI_DIR_TX_ONLY);

 
    /*
     * Place data in TX FIFO
     * We fill the FIFO of the SPI host and then flush every 72 words (the depth of the fifo)
    */
    uint16_t words_in_fifo = 0;
    uint32_t *data_32bit = (uint32_t *)data;
    for (uint16_t i = 0; i < length_W; i++) {
        if( words_in_fifo == SPI_HOST_PARAM_TX_DEPTH){
            send_command_to_spi_host(host, SPI_HOST_PARAM_TX_DEPTH*4, true, SPI_DIR_TX_ONLY);
            words_in_fifo = 0;
        }
        spi_wait_for_tx_not_full(host);
        spi_write_word(host, REVERT_ENDIANNESS(data_32bit[i]));
        words_in_fifo++;
    }
    
    if ( remaining_bytes ) {
        uint32_t mask = (1 << (8 * remaining_bytes)) - 1; // Only keep the remaining bytes with a mask
        uint32_t last_word = ((uint32_t*)data)[length_W] & mask;
        spi_wait_for_tx_not_full(host);
        spi_write_word(host,  REVERT_ENDIANNESS( last_word ) );
    }

    // SPI host cannot send individual bytes, so we will send the words available at the fifo and if there are any remaining bytes we add one extra full word. 
    send_command_to_spi_host(host, (words_in_fifo+ (uint16_t)(remaining_bytes != 0))*4 , false, SPI_DIR_TX_ONLY);
    spi_wait_for_tx_empty(host);
    return SPI_FLAG_SUCCESS;
}


spi_flags_e spi_slave_read(spi_host_t* host, uint8_t* addr, uint8_t* data, uint16_t length_B, uint8_t dummy_cycles){

    // if(dummy_cycles) send_command_to_spi_host(host, dummy_cycles, true, SPI_DIR_DUMMY);
    
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
    spi_write_word(host, REVERT_ENDIANNESS((uint32_t)addr));
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
     * continue until all the data is read.
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
        // Read data from SPI Host RX FIFO
        for (uint16_t i = length_W_rx - remaining_W; i < to_read_W-1; i++) {
            spi_read_word(host, &data_32bit); // Writes a full word
            ((uint32_t *)data)[i] = REVERT_ENDIANNESS(data_32bit);
        }
        remaining_W -= to_read_W;
    }

    // // Always treat the last word with extra care in case we were not copying a full 32-bit word. 
    spi_read_word(host, &data_32bit);

    // printf("A%d\n\r",data_32bit);

    data_32bit = REVERT_ENDIANNESS(data_32bit);
    uint32_t mask = (1 << (8 * remaining_bytes)) - 1; // Only keep the remaining bytes with a mask
    data_32bit = data_32bit & mask;
    printf("B%d\n\r",data_32bit);
    printf("BC%d\n\r", ((uint32_t*)data)[length_W_rx-1]);
    ((uint32_t*)data)[length_W_rx-1] &= ~mask;
    printf("C%d\n\r", ((uint32_t*)data)[length_W_rx-1]);
    ((uint32_t*)data)[length_W_rx-1] |= data_32bit;    
    printf("D%d\n\r", ((uint32_t*)data)[length_W_rx-1]);  
    
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

