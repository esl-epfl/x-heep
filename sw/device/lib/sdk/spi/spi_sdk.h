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
    SPI_CODE_SLAVE_FREQ_INVAL   = 0x0020,
    SPI_CODE_NOT_IDLE           = 0x0040,
    SPI_CODE_SLAVE_NOT_INIT     = 0x0080,
    SPI_CODE_SEGMENT_INVAL      = 0x0100
} spi_codes_e;

typedef enum {
    SPI_DATA_MODE_0 = 0,     // cpol = 0, cpha = 0
    SPI_DATA_MODE_1 = 1,     // cpol = 0, cpha = 1
    SPI_DATA_MODE_2 = 2,     // cpol = 1, cpha = 0
    SPI_DATA_MODE_3 = 3      // cpol = 1, cpha = 1
} spi_datamode_e;

typedef enum {
    SPI_MODE_DUMMY   = 0,
    SPI_MODE_RX_STD  = 1,
    SPI_MODE_TX_STD  = 2,
    SPI_MODE_BIDIR   = 3,
    // 4 is same than DUMMY
    SPI_MODE_RX_DUAL = 5,
    SPI_MODE_TX_DUAL = 6,
    // 7 will be unvalidated by HAL
    // 8 is same than DUMMY
    SPI_MODE_RX_QUAD = 9,
    SPI_MODE_TX_QUAD = 10
    // everything > 10 will be unvalidated by HAL
} spi_mode_e;

typedef struct {
    uint8_t        csid       : 2;
    spi_datamode_e data_mode  : 2;
    bool           full_cycle : 1;
    uint8_t        csn_lead   : 4;
    uint8_t        csn_trail  : 4;
    uint8_t        csn_idle   : 4;
    uint32_t       freq       : 32;
} spi_slave_t;

typedef struct {
    uint32_t   len        : 24;
    spi_mode_e mode       : 4;
} spi_segment_t;

typedef struct {
    const spi_segment_t* segments;
    uint8_t              seglen;
    const void*          txbuffer;
    uint32_t             txlen;
    void*                rxbuffer;
    uint32_t             rxlen;
} spi_transaction_t;

typedef struct {
    const spi_idx_e idx;
    bool init;
    spi_slave_t slave; // TODO: Find a way of having multiple slaves...
} spi_t;

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

spi_t spi_init(spi_idx_e idx, spi_slave_t slave);

spi_codes_e spi_deinit(spi_t* spi);

spi_codes_e spi_reset(spi_t* spi);

spi_codes_e spi_get_slave(spi_t* spi, uint8_t csid, spi_slave_t* slave);

spi_codes_e spi_transmit(spi_t* spi, const uint32_t* src_buffer, uint32_t len);

spi_codes_e spi_receive(spi_t* spi, uint32_t* dest_buffer, uint32_t len);

spi_codes_e spi_transceive(spi_t* spi, const uint32_t* src_buffer, uint32_t* dest_buffer, uint32_t len);

spi_codes_e spi_execute(spi_t* spi, const spi_transaction_t* transaction);

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
