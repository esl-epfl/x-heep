#ifndef X_SPI_H
#define X_SPI_H
#include <stdint.h>

#define WAD_START_ADDRESS 1024*1024
#define SECT_LEN          4096         // Length bytes of a sector

void X_init_spi();
void X_spi_read(uint32_t address, uint32_t *data, uint32_t len);
void X_spi_erase_sector(uint32_t addr); 
void X_spi_write(uint32_t loc, void* buffer, uint32_t size); 
void X_spi_write_sector(uint32_t address, uint32_t* sect_data); 
uint32_t X_spi_alloc_sector();  
int32_t read_finesine(uint32_t index); 

#endif // X_SPI_H
