/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************
**
** project  : X-HEEP
** filename : spi_sdk.h
** version  : 1
** date     : 18/04/24
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
* @file   spi_sdk.h
* @date   18/04/24
* @brief  The Serial Peripheral Interface (SPI) SDK to set up and use the
* SPI peripheral
*/

#ifndef _SDK_SPI_H_
#define _SDK_SPI_H_

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include <stdint.h>

#include "mmio.h"

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

#define SPI_MAX_IDX 3
#define SPI_IDX_INVALID(idx)    idx > SPI_MAX_IDX

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/**
* SPI Peripheral IDX
*/
typedef enum {
    SPI_IDX_FLASH       = 0,
    SPI_IDX_HOST        = 1,
    SPI_IDX_HOST_2      = 2
} spi_idx_e;

typedef enum {
    SPI_CODE_OK         = 0,
    SPI_CODE_IDX_INVAL  = 1,
    SPI_CODE_INIT       = 2,
    SPI_CODE_NOT_INIT   = 4
} spi_codes_e;

/**
 * Initialization parameters for SPI.
 */
// typedef struct {
//     spi_idx_e spi_idx;
// } spi_t;

typedef struct {

} spi_slave_t;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

spi_codes_e spi_init(spi_idx_e idx);
spi_codes_e spi_deinit(spi_idx_e idx);
spi_codes_e spi_config_slave();
spi_codes_e spi_transmit();
spi_codes_e spi_receive();
spi_codes_e spi_transceive();
spi_codes_e spi_exec(void* commands, uint8_t len);
void spi_write(uint32_t* src_buffer, uint32_t len);
void spi_write_bytes(uint8_t* src_buffer, uint32_t len);
void spi_read(uint32_t* dest_buffer, uint32_t len);

/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/


#ifdef __cplusplus
}
#endif

#endif // _SDK_SPI_H_

/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
