/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************
**
** project  : X-HEEP
** filename : w25q.c
** version  : 1
** date     : 1/11/2023
**
***************************************************************************
**
** Copyright (c) EPFL contributors.
** All rights reserved.
**
***************************************************************************
*/

/***************************************************************************/
/***************************************************************************/
/**
* @file   w25q.c
* @date   1/11/2023
* @brief  Source file of the W25Q-family flash memory driver.
*/

/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/
#include "w25q128jw.h"

/* To manage addresses. */
#include "mmio.h"

/* To manage interrupts. */
#include "fast_intr_ctrl.h"
#include "csr.h"
#include "stdasm.h"

/* To manage DMA. */
#include "dma.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

/**
 * The flash is expecting the address in Big endian format, so a swap is needed
 * in order to provide the MSB first.
*/
#define REVERT_24b_ADDR(addr) ((((uint32_t)(addr) & 0xff0000) >> 16) | ((uint32_t)(addr) & 0xff00) | (((uint32_t)(addr) & 0xff) << 16))

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Power up the flash.
*/
static void flash_power_up(void);

/**
 * @brief Set the QE bit in the flash status register.
 * 
 * @return 1 if the QE bit is set, 0 otherwise.
*/
static uint8_t set_QE_bit(void);

/**
 * @brief Configure the SPI<->Flash connection paramethers.
 * 
 * @param soc_ctrl pointer to the soc_ctrl_t structure.
*/
static void configure_spi(soc_ctrl_t soc_ctrl);

/**
 * @brief Wait for the flash to be ready.
 * 
 * It pools the BUSY bit in the flash status register.
 * It is not checking the SUS bit status.
*/
static void flash_wait(void);

/**
 * @brief Reset the flash.
*/
static void flash_reset(void);

/**
 * @brief Erase the flash and write the data.
 * 
 * It read 4k (a sector) at a time, erase it and write the data back.
 * Befor writing the data back, it modifies the buffer in order to
 * add the new data without modifing the other bytes. All the bytes
 * that are not going to be modified will be copied back as is.
 * 
 * @param addr 24-bit address to write to.
 * @param data pointer to the data buffer.
 * @param length number of bytes to write.
 * @return FLASH_OK if the write is successful, @ref error_codes otherwise.
*/
static uint8_t erase_and_write(uint32_t addr, uint8_t *data ,uint32_t length);

/**
 * @brief Wrapper for page write.
 * 
 * It performs the sanity checks and calls the page_write function with 
 * the correct speed paramether. A wrapper is necessary as it is not possible
 * to program more than a page (256 bytes) at a time. So multiple calls
 * to page_write can be needed.
 * 
 * @param addr 24-bit address to write to.
 * @param data pointer to the data buffer.
 * @param length number of bytes to write.
 * @param quad if 1, the write is performed at quad speed.
 * @param dma if 1, the write is performed using DMA.
*/
static uint8_t page_write_wrapper(uint32_t addr, uint8_t *data, uint32_t length, uint8_t quad, uint8_t dma);

/**
 * @brief Write (up to) a page to the flash.
 * 
 * @param addr 24-bit address to write to.
 * @param data pointer to the data buffer.
 * @param length number of bytes to write.
 * @param quad if 1, the write is performed at quad speed.
 * @param dma if 1, the write is performed using DMA.
*/
static void page_write(uint32_t addr, uint8_t *data, uint32_t length, uint8_t quad, uint8_t dma);

/**
 * @brief Copy length bytes from data to the SPI TX FIFO, using DMA.
 * 
 * @param data pointer to the data buffer.
 * @param length number of bytes to copy.
*/
static uint8_t dma_send_toflash(uint8_t *data, uint32_t length);

/**
 * @brief Enable flash write.
 * 
 * It sets the WEL bit in the flash status register.
 * Every action that require the WEL to be set is goig to automatically
 * clear it.
*/
static void flash_write_enable(void);

/**
 * @brief Performs sanity checks on the input parameters.
 * 
 * Checks if the address is valid, the data pointer is not NULL
 * and the length is not 0.
 * 
 * @param addr 24-bit address.
 * @param data pointer to the data buffer.
 * @param length number of bytes to read/write.
 * @return 1 if the checks are passed, 0 otherwise.
*/
static uint8_t sanity_checks(uint32_t addr, uint8_t *data, uint32_t length);

/**
 * @brief Return the minimum between two numbers.
 * 
 * The function uses signed integers in order to handle also negative numbers.
 * 
 * @param a first number.
 * @param b second number.
 * @return the minimum between a and b.
*/
static int32_t MIN(int32_t a, int32_t b) {
    return (a < b) ? a : b;
}


/****************************************************************************/
/**                                                                        **/
/*                            GLOBAL VARIABLES                              */
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Flag to signal the end of a SPI transaction.
*/
// volatile int8_t spi_intr_flag;

/**
 * @brief SPI host structure.
*/
spi_host_t spi;


/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/
uint8_t w25q128jw_init() {
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

    /*
    * To support both simulation and FPGA execution
    */
    #ifndef USE_SPI_FLASH
    spi.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    #else
    spi.base_addr = mmio_region_from_addr((uintptr_t)SPI_FLASH_START_ADDRESS);
    #endif

    /*
    * Check if memory mapped SPI is enabled. Current version of the bsp
    * does not support memory mapped SPI.
    */
    if (get_spi_flash_mode(&soc_ctrl) == SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO) {
        return FLASH_ERROR; // Error
    }

    #ifdef USE_SPI_FLASH
    // Select SPI host as SPI output
    soc_ctrl_select_spi_host(&soc_ctrl);
    #endif // USE_SPI_FLASH

    /*
     * For whatever reason this is required. If not present simulation 
     * behave very weirdly.
     * Is is needed also upon reset?
    */
    const uint32_t reset_cmd = 0xFFFFFFFF; // WTF???
    spi_write_word(&spi, reset_cmd);
    const uint32_t cmd_reset = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi, cmd_reset);
    spi_wait_for_ready(&spi);
    
    // Enable SPI host device
    spi_set_enable(&spi, true);
    // Enable SPI output
    spi_output_enable(&spi, true);
    // Configure SPI<->Flash connection on CSID 0
    configure_spi(soc_ctrl);
    // Set CSID
    spi_set_csid(&spi, 0);
    // Power up flash
    flash_power_up();
    // Set QE bit (only FPGA, simulation do not support status registers at all)
    #ifdef TARGET_PYNQ_Z2
    if (set_QE_bit() == FLASH_ERROR) return FLASH_ERROR; // Error occurred while setting QE bit
    #endif // TARGET_PYNQ_Z2

    return FLASH_OK; // Success
}

uint8_t w25q128jw_read(uint32_t addr, void *data, uint32_t length) {
    // Sanity checks
    if (sanity_checks(addr, data, length) == 0) return FLASH_ERROR;

    // Check DMA availability
    uint32_t dma_avail = dma_is_ready();

    // TODO

    return 0;
}

uint8_t w25q128jw_write(uint32_t addr, void *data, uint32_t length, uint8_t erase_before_write) {
    // Sanity checks
    if (sanity_checks(addr, data, length) == 0) return FLASH_ERROR;

    // Check DMA availability
    uint32_t dma_avail = dma_is_ready();

    // TODO
    uint8_t status = erase_and_write(addr, data, length);
    if (status == FLASH_ERROR) printf("Flash error");

    return FLASH_OK;
}

uint8_t w25q128jw_read_standard(uint32_t addr, void* data, uint32_t length) {
    // Sanity checks
    if (sanity_checks(addr, data, length) == 0) return FLASH_ERROR;

    // Address + Read command
    uint32_t read_byte_cmd = ((REVERT_24b_ADDR(addr & 0x00ffffff) << 8) | FC_RD);
    // Load command to TX FIFO
    spi_write_word(&spi, read_byte_cmd);
    spi_wait_for_ready(&spi);

    // Set up segment parameters -> send command and address
    const uint32_t cmd_read_1 = spi_create_command((spi_command_t){
        .len        = 3,                 // 4 Bytes
        .csaat      = true,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    // Load segment parameters to COMMAND register
    spi_set_command(&spi, cmd_read_1);
    spi_wait_for_ready(&spi);

    // Set up segment parameters -> read length bytes
    const uint32_t cmd_read_2 = spi_create_command((spi_command_t){
        .len        = length-1,          // len bytes
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirRxOnly      // Read only
    });
    spi_set_command(&spi, cmd_read_2);
    spi_wait_for_ready(&spi);

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
        if (length >= RX_FIFO_DEPTH) {
            spi_set_rx_watermark(&spi, RX_FIFO_DEPTH/4);
            length -= RX_FIFO_DEPTH;
            to_read += RX_FIFO_DEPTH;
        }
        else {
            spi_set_rx_watermark(&spi, (length%4==0 ? length/4 : length/4+1));
            to_read += length;
            flag = 0;
        }
        // Wait till SPI RX FIFO is full (or I read all the data)
        spi_wait_for_rx_watermark(&spi);
        // Read data from SPI RX FIFO
        for (int i = i_start; i < to_read/4; i++) {
            spi_read_word(&spi, &data_32bit[i]); // Writes a full word
        }
        // Update the starting index
        i_start += RX_FIFO_DEPTH/4;
    }
    // Take into account the extra bytes (if any)
    if (length_original%4 != 0) {
        uint32_t last_word = 0;
        spi_read_word(&spi, &last_word);
        memcpy(&data_32bit[length_original/4], &last_word, length%4);
    }

    return FLASH_OK; // Success
}

uint8_t w25q128jw_write_standard(uint32_t addr, void* data, uint32_t length) {
    // Call the wrapper with quad = 0, dma = 0
    return page_write_wrapper(addr, data, length, 0, 0);
}

uint8_t w25q128jw_read_standard_dma(uint32_t addr, void *data, uint32_t length) {
    // Sanity checks
    if (sanity_checks(addr, data, length) == 0) return FLASH_ERROR;

    /*
     * SET UP DMA
    */
    // SPI and SPI_FLASH are the same IP so same register map
    uint32_t *fifo_ptr_rx = spi.base_addr.base + SPI_HOST_RXDATA_REG_OFFSET;

    // Init DMA, the integrated DMA is used (peri == NULL)
    dma_init(NULL);

    // The DMA will wait for the SPI HOST/FLASH RX FIFO valid signal
    #ifndef USE_SPI_FLASH
        uint8_t slot = DMA_TRIG_SLOT_SPI_RX;
    #else
        uint8_t slot = DMA_TRIG_SLOT_SPI_FLASH_RX;
    #endif

    // Set up DMA source target
    static dma_target_t tgt_src = {
        .inc_du = 0, // Target is peripheral, no increment
        .type = DMA_DATA_TYPE_WORD, // Data type is word
    };
    // Size is in data units (words in this case)
    tgt_src.size_du = (length%4==0) ? length/4 : length/4+1;
    // Target is SPI RX FIFO
    tgt_src.ptr = (uint8_t*)fifo_ptr_rx;
    // Trigger to control the data flow
    tgt_src.trig = slot;

    // Set up DMA destination target
    static dma_target_t tgt_dst = {
        .inc_du = 1, // Increment by 1 data unit (word)
        .type = DMA_DATA_TYPE_WORD, // Data type is byte
        .trig = DMA_TRIG_MEMORY, // Read-write operation to memory
    };
    tgt_dst.ptr = (uint8_t*)data; // Target is the data buffer

    // Set up DMA transaction
    static dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .end = DMA_TRANS_END_POLLING,
    };

    // Validate, load and launch DMA transaction
    dma_config_flags_t res;
    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    res = dma_load_transaction(&trans);
    res = dma_launch(&trans);

    // Address + Read command
    uint32_t read_byte_cmd = ((REVERT_24b_ADDR(addr & 0x00ffffff) << 8) | FC_RD);
    // Load command to TX FIFO
    spi_write_word(&spi, read_byte_cmd);
    spi_wait_for_ready(&spi);

    // Set up segment parameters -> send command and address
    const uint32_t cmd_read_1 = spi_create_command((spi_command_t){
        .len        = 3,                 // 4 Bytes
        .csaat      = true,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    // Load segment parameters to COMMAND register
    spi_set_command(&spi, cmd_read_1);
    spi_wait_for_ready(&spi);

    // Set up segment parameters -> read length bytes
    const uint32_t cmd_read_2 = spi_create_command((spi_command_t){
        .len        = length-1,          // len bytes
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirRxOnly      // Read only
    });
    spi_set_command(&spi, cmd_read_2);
    spi_wait_for_ready(&spi);

    // Wait for DMA to finish transaction
    printf("Waiting dma...\n");
    while(!dma_is_ready());

    return FLASH_OK;
}

uint8_t w25q128jw_write_standard_dma(uint32_t addr, void *data, uint32_t length) {
    // Call the wrapper with quad = 0, dma = 1
    return page_write_wrapper(addr, data, length, 0, 1);
}

uint8_t w25q128jw_read_quad(uint32_t addr, void *data, uint32_t length) {
    // Sanity checks
    if (sanity_checks(addr, data, length) == 0) return FLASH_ERROR;

    // Send quad read command at standard speed
    uint32_t cmd_read_quadIO = FC_RDQIO;
    spi_write_word(&spi, cmd_read_quadIO);
    const uint32_t cmd_read = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = true,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_read);
    spi_wait_for_ready(&spi);

    /*
     * Send address at quad speed.
     * Last byte is Fxh (here FFh) required by W25Q128JW
    */
    uint32_t read_byte_cmd = (REVERT_24b_ADDR(addr) | (0xFF << 24));
    spi_write_word(&spi, read_byte_cmd);
    const uint32_t cmd_address = spi_create_command((spi_command_t){
        .len        = 3,                // 3 Byte
        .csaat      = true,             // Command not finished
        .speed      = kSpiSpeedQuad,    // Quad speed
        .direction  = kSpiDirTxOnly     // Write only
    });
    spi_set_command(&spi, cmd_address);
    spi_wait_for_ready(&spi);

    // Quad read requires dummy clocks
    const uint32_t dummy_clocks_cmd = spi_create_command((spi_command_t){
        #ifdef TARGET_PYNQ_Z2
        .len        = 3,               // W25Q128JW flash needs 4 dummy cycles
        #else
        .len        = 7,               // SPI flash simulation model needs 8 dummy cycles
        #endif
        .csaat      = true,            // Command not finished
        .speed      = kSpiSpeedQuad,   // Quad speed
        .direction  = kSpiDirDummy     // Dummy
    });
    spi_set_command(&spi, dummy_clocks_cmd);
    spi_wait_for_ready(&spi);

    // Read back the requested data at quad speed
    const uint32_t cmd_read_rx = spi_create_command((spi_command_t){
        .len        = length-1,        // 32 Byte
        .csaat      = false,           // End command
        .speed      = kSpiSpeedQuad,   // Quad speed
        .direction  = kSpiDirRxOnly    // Read only
    });
    spi_set_command(&spi, cmd_read_rx);
    spi_wait_for_ready(&spi);

    /* COMMAND FINISHED */

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
        if (length >= RX_FIFO_DEPTH) {
            spi_set_rx_watermark(&spi, RX_FIFO_DEPTH/4);
            length -= RX_FIFO_DEPTH;
            to_read += RX_FIFO_DEPTH;
        }
        else {
            spi_set_rx_watermark(&spi, (length%4==0 ? length/4 : length/4+1));
            to_read += length;
            flag = 0;
        }
        // Wait till SPI RX FIFO is full (or I read all the data)
        spi_wait_for_rx_watermark(&spi);
        // Read data from SPI RX FIFO
        for (int i = i_start; i < to_read/4; i++) {
            spi_read_word(&spi, &data_32bit[i]); // Writes a full word
        }
        // Update the starting index
        i_start += RX_FIFO_DEPTH/4;
    }
    // Take into account the extra bytes (if any)
    if (length_original%4 != 0) {
        uint32_t last_word = 0;
        spi_read_word(&spi, &last_word);
        memcpy(&data_32bit[length_original/4], &last_word, length%4);
    }

    return FLASH_OK; // Success
}

uint8_t w25q128jw_write_quad(uint32_t addr, void *data, uint32_t length) {
    // Call the wrapper with quad = 1, dma = 0
    return page_write_wrapper(addr, data, length, 1, 0);
}

uint8_t w25q128jw_read_quad_dma(uint32_t addr, void *data, uint32_t length) {
    // Sanity checks
    if (sanity_checks(addr, data, length) == 0) return FLASH_ERROR;

    // Send quad read command at standard speed
    uint32_t cmd_read_quadIO = FC_RDQIO;
    spi_write_word(&spi, cmd_read_quadIO);
    const uint32_t cmd_read = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = true,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_read);
    spi_wait_for_ready(&spi);

    /*
     * Send address at quad speed.
     * Last byte is Fxh (here FFh) required by W25Q128JW
    */
    uint32_t read_byte_cmd = (REVERT_24b_ADDR(addr) | (0xFF << 24));
    spi_write_word(&spi, read_byte_cmd);
    const uint32_t cmd_address = spi_create_command((spi_command_t){
        .len        = 3,                // 3 Byte
        .csaat      = true,             // Command not finished
        .speed      = kSpiSpeedQuad,    // Quad speed
        .direction  = kSpiDirTxOnly     // Write only
    });
    spi_set_command(&spi, cmd_address);
    spi_wait_for_ready(&spi);

    // Quad read requires dummy clocks
    const uint32_t dummy_clocks_cmd = spi_create_command((spi_command_t){
        #ifdef TARGET_PYNQ_Z2
        .len        = 3,               // W25Q128JW flash needs 4 dummy cycles
        #else
        .len        = 7,               // SPI flash simulation model needs 8 dummy cycles
        #endif
        .csaat      = true,            // Command not finished
        .speed      = kSpiSpeedQuad,   // Quad speed
        .direction  = kSpiDirDummy     // Dummy
    });
    spi_set_command(&spi, dummy_clocks_cmd);
    spi_wait_for_ready(&spi);

    // Read back the requested data at quad speed
    const uint32_t cmd_read_rx = spi_create_command((spi_command_t){
        .len        = length-1,        // length bytes
        .csaat      = false,           // End command
        .speed      = kSpiSpeedQuad,   // Quad speed
        .direction  = kSpiDirRxOnly    // Read only
    });
    spi_set_command(&spi, cmd_read_rx);
    spi_wait_for_ready(&spi);

    /* COMMAND FINISHED */

    /*
     * SET UP DMA
    */
    // SPI and SPI_FLASH are the same IP so same register map
    uint32_t *fifo_ptr_rx = spi.base_addr.base + SPI_HOST_RXDATA_REG_OFFSET;

    // Init DMA, the integrated DMA is used (peri == NULL)
    dma_init(NULL);

    // The DMA will wait for the SPI HOST/FLASH RX FIFO valid signal
    #ifndef USE_SPI_FLASH
        uint8_t slot = DMA_TRIG_SLOT_SPI_RX;
    #else
        uint8_t slot = DMA_TRIG_SLOT_SPI_FLASH_RX;
    #endif

    // Set up DMA source target
    static dma_target_t tgt_src = {
        .inc_du = 0, // Target is peripheral, no increment
        .type = DMA_DATA_TYPE_WORD, // Data type is byte
    };
    // Size is in data units (words in this case)
    tgt_src.size_du = (length%4==0) ? length/4 : length/4+1;
    // Target is SPI RX FIFO
    tgt_src.ptr = (uint8_t*)fifo_ptr_rx;
    // Trigger to control the data flow
    tgt_src.trig = slot;

    // Set up DMA destination target
    static dma_target_t tgt_dst = {
        .inc_du = 1, // Increment by 1 data unit (word)
        .type = DMA_DATA_TYPE_WORD, // Data type is byte
        .trig = DMA_TRIG_MEMORY, // Read-write operation to memory
    };
    tgt_dst.ptr = (uint8_t*)data; // Target is the data buffer

    // Set up DMA transaction
    static dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .end = DMA_TRANS_END_POLLING,
    };

    // Validate, load and launch DMA transaction
    dma_config_flags_t res;
    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    res = dma_load_transaction(&trans);
    res = dma_launch(&trans);

    // Wait for DMA to finish transaction
    while(!dma_is_ready());

    return FLASH_OK;
}

uint8_t w25q128jw_write_quad_dma(uint32_t addr, void *data, uint32_t length) {
    // Call the wrapper with quad = 1, dma = 1
    return page_write_wrapper(addr, data, length, 1, 1);
}

void w25q128jw_4k_erase(uint32_t addr) {
    // Sanity checks
    if (addr > 0x00ffffff || addr < 0) return FLASH_ERROR;

    // Wait any other operation to finish
    flash_wait();

    // Enable flash write in order to erase
    flash_write_enable();

    // Build and send erase command
    uint32_t erase_4k_cmd = ((REVERT_24b_ADDR(addr & 0x00ffffff) << 8) | FC_SE);
    spi_write_word(&spi, erase_4k_cmd);
    spi_wait_for_ready(&spi);
    const uint32_t cmd_erase = spi_create_command((spi_command_t){
        .len        = 3,                 // 4 Bytes
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_erase);
    spi_wait_for_ready(&spi);

    // Wait for the erase operation to be finished
    flash_wait();
}

void w25q128jw_32k_erase(uint32_t addr) {
    // Sanity checks
    if (addr > 0x00ffffff || addr < 0) return FLASH_ERROR;

    // Wait any other operation to finish
    flash_wait();

    // Enable flash write in order to erase
    flash_write_enable();

    // Build and send erase command
    uint32_t erase_32k_cmd = ((REVERT_24b_ADDR(addr & 0x00ffffff) << 8) | FC_BE32);
    spi_write_word(&spi, erase_32k_cmd);
    spi_wait_for_ready(&spi);
    const uint32_t cmd_erase = spi_create_command((spi_command_t){
        .len        = 3,                 // 4 Bytes
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_erase);
    spi_wait_for_ready(&spi);

    // Wait for the erase operation to be finished
    flash_wait();
}

void w25q128jw_64k_erase(uint32_t addr) {
    // Sanity checks
    if (addr > 0x00ffffff || addr < 0) return FLASH_ERROR;
    
    // Wait any other operation to finish
    flash_wait();

    // Enable flash write in order to erase
    flash_write_enable();

    // Build and send erase command
    uint32_t erase_64k_cmd = ((REVERT_24b_ADDR(addr & 0x00ffffff) << 8) | FC_BE64);
    spi_write_word(&spi, erase_64k_cmd);
    spi_wait_for_ready(&spi);
    const uint32_t cmd_erase = spi_create_command((spi_command_t){
        .len        = 3,                 // 4 Bytes
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_erase);
    spi_wait_for_ready(&spi);

    // Wait for the erase operation to be finished
    flash_wait();
}

void w25q128jw_chip_erase() {
    // Wait any other operation to finish
    flash_wait();

    // Enable flash write in order to erase
    flash_write_enable();

    // Build and send erase command
    spi_write_word(&spi, FC_CE);
    spi_wait_for_ready(&spi);
    const uint32_t cmd_erase = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Bytes
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_erase);
    spi_wait_for_ready(&spi);

    // Wait for the erase operation to be finished
    flash_wait();
}

void w25q128jw_reset() {
    // Wait for ongoing operation to finish (if any)
    flash_wait();

    // Build and send reset command
    flash_reset();

    // Wait for the reset operation to be finished
    flash_wait();
}

void w25q128jw_reset_force() {
    // Build and send reset command without waiting for ongoing operation
    flash_reset(); 

    // Wait for the reset operation to be finished   
    flash_wait();
}

void w25q128jw_power_down() {
    // Build and send power down command
    spi_write_word(&spi, FC_PD);
    const uint32_t cmd_power_down = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_power_down);
    spi_wait_for_ready(&spi);
}


/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

static void flash_power_up() {
    spi_write_word(&spi, FC_RPD);
    spi_wait_for_ready(&spi);
    const uint32_t cmd_powerup = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_powerup);
    spi_wait_for_ready(&spi);
}

static uint8_t set_QE_bit() {
    spi_set_rx_watermark(&spi,1);

    // Read Status Register 2
    const uint32_t reg2_read_cmd = FC_RSR2;
    spi_write_word(&spi, reg2_read_cmd);

    const uint32_t reg2_read_1 = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = true,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, reg2_read_1);
    spi_wait_for_ready(&spi);

    const uint32_t reg2_read_2 = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Standard speed
        .direction  = kSpiDirRxOnly      // Read only
    });
    spi_set_command(&spi, reg2_read_2);
    spi_wait_for_ready(&spi);
    spi_wait_for_rx_watermark(&spi);
    
    /* 
     * the partial word will be zero-padded and inserted into the RX FIFO once the segment is completed
     * The actual register is 8 bit, but the SPI host gives a full word
    */
    uint32_t reg2_data;
    spi_read_word(&spi, &reg2_data);

    // Set bit in position 1 (QE bit), leaving the others unchanged
    reg2_data |= 0x2;

    // Enable write operation
    flash_write_enable();

    // Write Status Register 2 (set QE bit)
    const uint32_t reg2_write_cmd = FC_WSR2;
    spi_write_word(&spi, reg2_write_cmd);

    const uint32_t reg2_write_1 = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = true,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, reg2_write_1);
    spi_wait_for_ready(&spi);

    // Load data to TX FIFO
    spi_write_word(&spi, reg2_data);

    // Create command segment
    const uint32_t reg2_write_2 = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Standard speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, reg2_write_2);
    spi_wait_for_ready(&spi);
    
    // Read back Status Register 2
    spi_write_word(&spi, reg2_read_cmd);
    spi_set_command(&spi, reg2_read_1);
    spi_wait_for_ready(&spi);
    spi_set_command(&spi, reg2_read_2);
    spi_wait_for_ready(&spi);
    spi_wait_for_rx_watermark(&spi);
    uint32_t reg2_data_check = 0x00;
    spi_read_word(&spi, &reg2_data_check);

    // Check if the QE bit is set
    if ((reg2_data_check & 0x2) == 0) return FLASH_ERROR;
    else return FLASH_OK;
}

static void configure_spi(soc_ctrl_t soc_ctrl) {
    // Configure SPI clock
    uint32_t core_clk = soc_ctrl_get_frequency(&soc_ctrl);
    uint16_t clk_div = 0;
    if(FLASH_CLK_MAX_HZ < core_clk/2){
        clk_div = (core_clk/(FLASH_CLK_MAX_HZ) - 2)/2; // The value is truncated
        if (core_clk/(2 + 2 * clk_div) > FLASH_CLK_MAX_HZ) clk_div += 1; // Adjust if the truncation was not 0
    }
    // SPI Configuration
    // Configure chip 0 (flash memory)
    const uint32_t chip_cfg = spi_create_configopts((spi_configopts_t){
        .clkdiv     = clk_div,
        .csnidle    = 0xF,
        .csntrail   = 0xF,
        .csnlead    = 0xF,
        .fullcyc    = false,
        .cpha       = 0,
        .cpol       = 0
    });
    spi_set_configopts(&spi, 0, chip_cfg);
}

static void flash_wait() {
    spi_set_rx_watermark(&spi,1);
    bool flash_busy = true;
    uint8_t flash_resp[4] = {0xff,0xff,0xff,0xff};

    while(flash_busy){
        uint32_t flash_cmd = FC_RSR1; // [CMD] Read status register 1
        spi_write_word(&spi, flash_cmd); // Push TX buffer
        uint32_t spi_status_cmd = spi_create_command((spi_command_t){
            .len        = 0,
            .csaat      = true,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirTxOnly
        });
        uint32_t spi_status_read_cmd = spi_create_command((spi_command_t){
            .len        = 0,
            .csaat      = false,
            .speed      = kSpiSpeedStandard,
            .direction  = kSpiDirRxOnly
        });
        spi_set_command(&spi, spi_status_cmd);
        spi_wait_for_ready(&spi);
        spi_set_command(&spi, spi_status_read_cmd);
        spi_wait_for_ready(&spi);
        spi_wait_for_rx_watermark(&spi);
        spi_read_word(&spi, &flash_resp[0]);
        if ((flash_resp[0] & 0x01) == 0) flash_busy = false;
    }
}

static void flash_reset() {
    spi_write_word(&spi, FC_ERESET);
    spi_write_word(&spi, FC_RESET);
    spi_wait_for_ready(&spi);

    const uint32_t cmd_reset_enable = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_reset_enable);
    spi_wait_for_ready(&spi);
    const uint32_t cmd_reset = spi_create_command((spi_command_t){
        .len        = 0,                 // 1 Byte
        .csaat      = false,             // End command
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    spi_set_command(&spi, cmd_reset);
    spi_wait_for_ready(&spi);
}

uint8_t erase_and_write(uint32_t addr, uint8_t *data, uint32_t length) {
    // Sanity checks
    if (sanity_checks(addr, data, length) == 0) return FLASH_ERROR;

    uint32_t remaining_length = length;
    uint32_t current_addr = addr;
    uint8_t *current_data = data;

    // Allocate a buffer to store the sector data
    uint8_t *sector_data = (uint8_t *)malloc(4096);
    if (sector_data == NULL) return FLASH_ERROR;

    while (remaining_length > 0) {
        // Start address of the sector to erase
        uint32_t sector_start_addr = current_addr & 0xfffff000;

        // Read the full sector and save it into RAM
        w25q128jw_read(sector_start_addr, sector_data, 4096);

        // Erase the sector
        w25q128jw_4k_erase(sector_start_addr);

        // Calculate the length of data to write in this sector
        uint32_t write_length = MIN(4096 - (current_addr - sector_start_addr), remaining_length);

        // Modify the data in RAM to include the new data
        memcpy(&sector_data[current_addr - sector_start_addr], current_data, write_length);

        // Write the modified data back to the flash (without erasing this time)
        w25q128jw_write(sector_start_addr, sector_data, 4096, 0);

        // Update the remaining length, address and data pointer
        remaining_length -= write_length;
        current_addr += write_length;
        current_data += write_length;
    }

    // Free the sector data buffer
    free(sector_data);

    return FLASH_OK;
}

static uint8_t page_write_wrapper(uint32_t addr, uint8_t *data, uint32_t length, uint8_t quad, uint8_t dma) {
    // Sanity checks
    if (sanity_checks(addr, data, length) == 0) return FLASH_ERROR;

    // Pointer arithmetics is not allowed on void pointers
    uint8_t *data_8bit = (uint8_t *)data;

    /*
     * Set speed and DMA flags.
     * Robust implementation: no need to perform safety checks on the flags.
    */
    uint8_t speed = quad==1 ? 1 : 0;
    uint8_t dma_flag = dma==1 ? 1 : 0;

    /*
     * Taking care of misalligned start address.
     * If the start address is not aligned to a 256 bytes boundary,
     * the first page is written with the first 256 - (addr % 256) bytes.
    */
    if (addr % 256 != 0) {
        uint8_t tmp_len = 256 - (addr % 256);
        page_write(addr, data_8bit, tmp_len, speed, dma_flag);
        addr += tmp_len;
        data_8bit += tmp_len;
        length -= tmp_len;
    }

    // I cannot program more than a page (256 Bytes) at a time.
    int flag = 1;
    while (flag) {
        if (length > 256) {
            page_write(addr, data_8bit, 256, speed, dma_flag);
            addr += 256;
            data_8bit += 256;
            length -= 256;
        } else {
            page_write(addr, data_8bit, length, speed, dma_flag);
            flag = 0;
        }
    }
    return FLASH_OK;
}

static void page_write(uint32_t addr, uint8_t *data, uint32_t length, uint8_t quad, uint8_t dma) {
    // Required every time before issuing a write command
    flash_write_enable();

    /*
     * Build and send write command (24bit address + command).
     * The command is picked based on the quad flag.
    */
    const uint32_t write_byte_cmd = ((REVERT_24b_ADDR(addr & 0x00ffffff) << 8) | (quad ? FC_PPQ : FC_PP));
    spi_write_word(&spi, write_byte_cmd);
    const uint32_t cmd_write = spi_create_command((spi_command_t){
        .len        = 3,
        .csaat      = true,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi, cmd_write);
    spi_wait_for_ready(&spi);

    // Place data in TX FIFO
    if (dma) {
        dma_send_toflash(data, length);
    } else {
        uint32_t *data_32bit = (uint32_t *)data;
        for (int i = 0; i < length/4; i++) {
            spi_write_word(&spi, data_32bit[i]);
        }
        if (length % 4 != 0) {
            uint32_t last_word = 0;
            memcpy(&last_word, &data[length - length % 4], length % 4);
            spi_write_word(&spi, last_word);
        }
    }

    /*
     * Set up segment parameters -> send data.
     * Speed is quad if quad flag is set, standard otherwise.
    */
    const uint32_t cmd_write_2 = spi_create_command((spi_command_t){
        .len        = length-1,
        .csaat      = false,
        .speed      = quad ? kSpiSpeedQuad : kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi, cmd_write_2);
    spi_wait_for_ready(&spi);

    // Wait for flash to be ready again (FPGA only)
    #ifdef TARGET_PYNQ_Z2
    flash_wait();
    #endif // TARGET_PYNQ_Z2
}

static uint8_t dma_send_toflash(uint8_t *data, uint32_t length) {
    // SPI and SPI_FLASH are the same IP so same register map
    uint32_t *fifo_ptr_tx = spi.base_addr.base + SPI_HOST_TXDATA_REG_OFFSET;

    // Init DMA, the integrated DMA is used (peri == NULL)
    dma_init(NULL);

    // The DMA will wait for the SPI HOST/FLASH TX FIFO valid signal
    #ifndef USE_SPI_FLASH
        uint8_t slot = DMA_TRIG_SLOT_SPI_TX;
    #else
        uint8_t slot = DMA_TRIG_SLOT_SPI_FLASH_TX;
    #endif

    // Set up DMA source target
    static dma_target_t tgt_src = {
        .inc_du = 1, // Increment by 1 data unit (word)
        .type = DMA_DATA_TYPE_WORD, // Data type is word
    };
    // Size is in data units (words in this case)
    tgt_src.size_du = (length%4==0) ? length/4 : length/4+1;
    // Target is data buffer
    tgt_src.ptr = data;
    // Reads from memory
    tgt_src.trig = DMA_TRIG_MEMORY;

    // Set up DMA destination target
    static dma_target_t tgt_dst = {
        .inc_du = 0, // It's a peripheral, no increment
        .type = DMA_DATA_TYPE_WORD, // Data type is word
    };
    tgt_dst.trig = slot;
    tgt_dst.ptr = (uint8_t*)fifo_ptr_tx; // Target is SPI TX FIFO

    // Set up DMA transaction
    static dma_trans_t trans = {
        .src = &tgt_src,
        .dst = &tgt_dst,
        .mode = DMA_TRANS_MODE_SINGLE,
        .win_du = 0,
        .end = DMA_TRANS_END_POLLING,
    };

    // Validate, load and launch DMA transaction
    dma_config_flags_t res;
    res = dma_validate_transaction(&trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    if (res != DMA_CONFIG_OK) return 1;
    res = dma_load_transaction(&trans);
    if (res != DMA_CONFIG_OK) return 1;
    res = dma_launch(&trans);
    if (res != DMA_CONFIG_OK) return 1;

    // Wait for DMA to finish transaction
    while(!dma_is_ready());
    return 0;
}

static void flash_write_enable() {
    spi_write_word(&spi, FC_WE);
    const uint32_t cmd_write_en = spi_create_command((spi_command_t){
        .len        = 0,
        .csaat      = false,
        .speed      = kSpiSpeedStandard,
        .direction  = kSpiDirTxOnly
    });
    spi_set_command(&spi, cmd_write_en);
    spi_wait_for_ready(&spi);
}

static uint8_t sanity_checks(uint32_t addr, uint8_t *data, uint32_t length) {
    // Check if address is out of range
    if (addr > 0x00ffffff || addr < 0) return 0;

    // Check if data pointer is NULL
    if (data == NULL) return 0;

    // Check if length is 0
    if (length <= 0) return 0;
    
    // Check if current address + length is out of range
    if (addr + length > 0x00ffffff) return 0;

    return 1; // Success
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/