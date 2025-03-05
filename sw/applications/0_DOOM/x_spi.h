#ifndef X_SPI_H
#define X_SPI_H
#include <stdint.h>

#define WAD_START_ADDRESS 1024*1024

// Add your code here
void X_init_spi();
void X_spi_read(uint32_t address, uint32_t *data, uint32_t len);


#endif // X_SPI_H
