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
#define SPI_IDX_INVALID(idx) idx > SPI_MAX_IDX

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
    SPI_CODE_OK                 = 0x0000,
    SPI_CODE_BASE_ERROR         = 0x0001,
    SPI_CODE_IDX_INVAL          = 0x0002,
    SPI_CODE_INIT               = 0x0004,
    SPI_CODE_NOT_INIT           = 0x0008,
    SPI_CODE_SLAVE_CSID_INVAL   = 0x0010,
    SPI_CODE_SLAVE_FREQ_INVAL   = 0x0020
} spi_codes_e;

typedef enum {
    SPI_DATA_MODE_0 = 0,     // cpol = 0, cpha = 0
    SPI_DATA_MODE_1 = 1,     // cpol = 0, cpha = 1
    SPI_DATA_MODE_2 = 2,     // cpol = 1, cpha = 0
    SPI_DATA_MODE_3 = 3      // cpol = 1, cpha = 1
} spi_datamode_e;

typedef struct {
    uint8_t        csid       : 2;
    spi_datamode_e data_mode  : 2;
    bool           full_cycle : 1;
    uint8_t        csn_lead   : 4;
    uint8_t        csn_trail  : 4;
    uint8_t        csn_idle   : 4;
    uint32_t       freq       : 32;
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

spi_codes_e spi_init(const spi_idx_e idx);
spi_codes_e spi_deinit(const spi_idx_e idx);
spi_codes_e spi_set_slave(const spi_idx_e idx, const spi_slave_t slave);
spi_codes_e spi_get_slave(const spi_idx_e idx, const uint8_t csid, spi_slave_t* slave);
spi_codes_e spi_transmit();
spi_codes_e spi_receive();
spi_codes_e spi_transceive();
spi_codes_e spi_exec(void* commands, uint8_t len);
void spi_write(uint32_t* src_buffer, uint32_t len);
void spi_write_bytes(uint8_t* src_buffer, uint32_t len);
void spi_read(uint32_t* dest_buffer, uint32_t len);
void spi_read_bytes(uint8_t* dest_buffer, uint32_t len);

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
