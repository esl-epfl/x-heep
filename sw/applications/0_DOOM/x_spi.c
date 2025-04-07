#include "x_spi.h"

//new version using sdk to read flash

#include <stdlib.h>

#include "x-heep.h"
#include "spi_sdk.h"

#include "fast_intr_ctrl.h"
#include "csr.h"
#include "csr_registers.h"

// =========================== VARS & DEFS ==================================
// Flash w25q128jw SPI commands
#define FC_WE      0x06 /** Write Enable */
#define FC_RD      0x03 /** Read Data */ 
#define FC_PP      0x02 /** Page Program */
#define FC_SE      0x20 /** Sector Erase 4kb */
#define FC_RSR1    0x05 /** Read Status Register 1 */

#define START_ADDRESS 0
#define READ_LEN 4                      // Amount words to read
#define FLASH_MAX_FREQ (133*1000*1000)  // Device max spi frequency
#define FIC_FLASH_MEIE 21               // SPI Flash fast interrupt bit enable
#define CSR_INTR_EN    0x08             // CPU Global interrupt enable
#define PAGE_LEN       256              // Length bytes of a page

#define WAD_LENGTH  0x800000

spi_t spi_flash;
uint32_t next_loc = WAD_START_ADDRESS + WAD_LENGTH + 1024;  

// ====================== PROTOTYPES ======================
bool flash_read(spi_t* spi, uint32_t addr, uint32_t* dest_buff, uint32_t len); 
bool flash_erase_sector(spi_t* spi, uint32_t addr); 
bool flash_write_sector(spi_t* spi, uint32_t addr, uint32_t* src_buff); 
bool flash_write_enable(spi_t* spi); 
void flash_wait(spi_t* spi); 
// ========================= FUNCTIONS =========================

void X_init_spi()
{
    spi_slave_t slave = SPI_SLAVE(0, FLASH_MAX_FREQ);
    
    spi_flash = spi_init(SPI_IDX_FLASH, slave);
    if (!spi_flash.init) {
        PRINTF("\nFailed to initialize spi\n");
        return EXIT_FAILURE;
    }

    PRINTF("\nSPI initialized\n");
    
    CSR_SET_BITS(CSR_REG_MSTATUS, CSR_INTR_EN);
    // Set mie.MEIE bit to one to enable machine-level fast spi_flash interrupt
    const uint32_t mask = 1 << FIC_FLASH_MEIE;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    X_test_read();
}

void X_spi_read(uint32_t address, uint32_t *data, uint32_t len)
{
    if (!flash_read(&spi_flash, address, data, 4*len))
    {
        PRINTF("\nFailed to read flash\n");
        return EXIT_FAILURE;   
    }
}

void X_test_read()
{
    uint32_t rxbuffer[READ_LEN] = {0};
    X_spi_read(WAD_START_ADDRESS, rxbuffer, READ_LEN);
    for (int i = 0; i < READ_LEN; i++)
    {
        PRINTF("0x%08X: %08X\n", WAD_START_ADDRESS+4*i, rxbuffer[i]);
    }
}

void X_spi_erase_sector(uint32_t addr)
{
    if (!flash_erase_sector(&spi_flash, addr)) return EXIT_FAILURE;
}

//This has not been tested yet ! 
void X_spi_write(uint32_t loc, void* buffer, uint32_t size)
{
    uint8_t* buffer_ptr = (uint8_t*)buffer;
    uint32_t address = loc;
    uint32_t bytes_remaining = size;

    while (bytes_remaining > 0)
    {
        uint32_t sector_buffer[SECT_LEN] = {0};

        uint32_t chunk_size = (bytes_remaining >= SECT_LEN) ? SECT_LEN : bytes_remaining;

        memcpy(&sector_buffer, buffer_ptr, chunk_size);

        if (!flash_write_sector(&spi_flash, address, sector_buffer)) return EXIT_FAILURE;

        // Update pointers and counters
        buffer_ptr += chunk_size;
        address += SECT_LEN;
        bytes_remaining -= chunk_size;
    }

}

void X_spi_write_sector(uint32_t address, uint32_t* sect_data)
{
    if (!flash_write_sector(&spi_flash, address, sect_data)) return EXIT_FAILURE;
} 

bool flash_read(spi_t* spi, uint32_t addr, uint32_t* dest_buff, uint32_t len) {
    // Transaction segments
    spi_segment_t segments[2] = { SPI_SEG_TX(4), SPI_SEG_RX(len) };

    // TX SPI command to send to flash for read
    // Flash uses Big Endian, CPU Little Endian, hence swap bytes
    uint32_t read_byte_cmd = ((bitfield_byteswap32(addr & 0x00ffffff)) | FC_RD);

    // PRINTF("Blocking Reading %4i Bytes at 0x%08X\n", len, addr);

    // Start transaction
    spi_codes_e error = spi_execute(spi, segments, 2, &read_byte_cmd, dest_buff);  

    if (error) {
        PRINTF("FAILED! Error Code: %i\n", error);
        return false;
    }
    return true;
}

bool flash_erase_sector(spi_t* spi, uint32_t addr) {
    // Sector start address
    const uint32_t sect_start = addr & 0xfffff000;
    // Sector erase command
    // Flash uses Big Endian, CPU Little Endian, hence swap bytes
    const uint32_t cmd = ((bitfield_byteswap32(sect_start & 0x00ffffff)) | FC_SE);

    PRINTF("Blocking Erasing 4096 Bytes at 0x%08X\n", sect_start);

    // Enable write before erase
    if (!flash_write_enable(spi)) return false;

    // Start TX Only transaction
    spi_codes_e error = spi_transmit(spi, &cmd, 4);
    if (error) {
        PRINTF("FAILED! Error Code: %i\n", error);
        return false;
    }

    // Wait flash finished processing
    flash_wait(spi);

    return true;
}

bool flash_write_sector(spi_t* spi, uint32_t addr, uint32_t* src_buff) {
    // Sector start address
    uint32_t sect = addr & 0xfffff000;
    // Current page (256 bytes) address (can't write more than one page at a time...)
    uint32_t curr_addr = sect;

    for (int i = 0; i < SECT_LEN / PAGE_LEN; i++)
    {
        // Our TX Buffer
        uint32_t wbuff[PAGE_LEN/4 + 1] = {0};
        // Our segments for the SPI transaction
        spi_segment_t segments[2] = { SPI_SEG_TX(4), SPI_SEG_TX(PAGE_LEN) };

        // Flash uses Big Endian, CPU Little Endian, hence swap bytes
        wbuff[0] = ((bitfield_byteswap32(curr_addr & 0x00ffffff)) | FC_PP);
        memcpy(&wbuff[1], &src_buff[i * (PAGE_LEN/4)], PAGE_LEN);

        if (!flash_write_enable(spi)) return false;

        // Start transaction
        // Note that since segments are only TX we could have used spi_transmit
        // instead of spi_execute, but both work perfectly fine
        spi_codes_e error = spi_execute(spi, segments, 2, wbuff, NULL);
        if (error) {
            PRINTF("FAILED! Error Code: %i\n", error);
            return false;
        }
        curr_addr += PAGE_LEN;

        PRINTF("\rBlocking Written %4i/4096 Bytes at 0x%08X", (i+1) * PAGE_LEN, sect);

        // Wait flash finished processing
        flash_wait(spi);
    }

    PRINTF("\n");
    
    return true;
}

bool flash_write_enable(spi_t* spi) {
    // SPI Flash command
    const uint32_t cmd = FC_WE;
    // Start TX Only transaction
    spi_codes_e error = spi_transmit(spi, &cmd, 1);
    if (error) {
        PRINTF("FAILED! Error Code: %i\n", error);
        return false;
    }
    return true;
}

void flash_wait(spi_t* spi) {
    // Response buffer
    uint32_t resp;
    // SPI Flash command
    uint32_t cmd = FC_RSR1;
    // Here we have to use segments and execute since our transaction is composed
    // of a TX Only part and thereafter a RX Only
    spi_segment_t segments[2] = {SPI_SEG_TX(1), SPI_SEG_RX(2)};
    // Flash busy flag
    bool busy = true;
    while (busy)
    {
        spi_execute(spi, segments, 2, &cmd, &resp);
        busy = resp & 0x01;
    }
}

uint32_t X_spi_alloc_sector()
{
    uint32_t loc = next_loc; 
    next_loc += SECT_LEN; 
    return loc; 
}

//old version 

/*

#include "x-heep.h"
#include "w25q128jw.h"

spi_host_t *spi_flash_device;

//private function declarations
void X_test_read();

//public function definitions
void X_init_spi()
{
    //spi_flash_device->base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    
    w25q128jw_init(spi_flash_device);
    PRINTF("X_SPI: init flash complete\n");
    PRINTF("X_SPI: Testing read\n");
    X_test_read(); //-> This function doesn't work. But it needs to work for the program to work !!!
    PRINTF("X_SPI: Finished testing read\n");
}

void X_spi_read(uint32_t address, uint32_t *data, uint32_t len)
{
    PRINTF("X_SPI: Reading data from WAD\n");
    w25q128jw_read_standard(address, data, len);
    PRINTF("X_SPI: Finished reading data from WAD\n");
}

//private function definitions
void X_test_read()
{
    uint32_t data[4];
    //X_spi_read(WAD_START_ADDRESS, data, 4);
    X_spi_read(0, data, 4);
    PRINTF("Data at WAD start address: %x %x %x %x\n", data[0], data[1], data[2], data[3]);
}
*/
