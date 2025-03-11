#include "x_spi.h"

#include <stdlib.h>

#include "x-heep.h"
#include "spi_sdk.h"

#include "fast_intr_ctrl.h"
#include "csr.h"
#include "csr_registers.h"

//new version using sdk to read flash

// =========================== VARS & DEFS ==================================
// Flash w25q128jw SPI commands
#define FC_RD      0x03 /** Read Data */

#define START_ADDRESS 0
#define READ_LEN 4                      // Amount words to read
#define FLASH_MAX_FREQ (133*1000*1000)  // Device max spi frequency
#define FIC_FLASH_MEIE 21               // SPI Flash fast interrupt bit enable
#define CSR_INTR_EN    0x08             // CPU Global interrupt enable


spi_t spi_flash; 

// ====================== PROTOTYPES ======================
bool flash_read(spi_t* spi, uint32_t addr, uint32_t* dest_buff, uint32_t len); 

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

    uint32_t rxbuffer[READ_LEN] = {0};

    if (!flash_read(&spi_flash, START_ADDRESS, rxbuffer, 4*READ_LEN)) return EXIT_FAILURE;

    for (int i = 0; i < READ_LEN; i++)
    {
        PRINTF("0x%08X: %08X\n", START_ADDRESS+4*i, rxbuffer[i]);
    }
}

bool flash_read(spi_t* spi, uint32_t addr, uint32_t* dest_buff, uint32_t len) {
    // Transaction segments
    spi_segment_t segments[2] = { SPI_SEG_TX(4), SPI_SEG_RX(len) };

    // TX SPI command to send to flash for read
    // Flash uses Big Endian, CPU Little Endian, hence swap bytes
    uint32_t read_byte_cmd = ((bitfield_byteswap32(addr & 0x00ffffff)) | FC_RD);

    PRINTF("Blocking Reading %4i Bytes at 0x%08X\n", len, addr);

    // Start transaction
    spi_codes_e error = spi_execute(spi, segments, 2, &read_byte_cmd, dest_buff);
    if (error) {
        PRINTF("FAILED! Error Code: %i\n", error);
        return false;
    }
    return true;
}


//old version 
/*

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