#include "x_spi.h"

#include "x-heep.h"
#include "w25q128jw.h"

spi_host_t spi_flash;

//private function declarations
void X_test_read();

//public function definitions
void X_init_spi()
{
    spi_flash.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    
    w25q128jw_init(spi_flash);
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
    X_spi_read(WAD_START_ADDRESS, data, 4);
    PRINTF("Data at WAD start address: %x %x %x %x\n", data[0], data[1], data[2], data[3]);
}