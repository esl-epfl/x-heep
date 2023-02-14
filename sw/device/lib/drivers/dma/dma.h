  // juan: remove this
  // typedef struct dma {
  //   /**
  //    * The base address for the soc_ctrl hardware registers.
  //    */
  //   mmio_region_t base_addr;
  // } dma_t; 
  //juan q: how does this struct relate to the one in dma_structs.h ?


/*
                              *******************
******************************* H HEADER FILE *****************************
**                            *******************                          
**                                                                         
** project  : X-HEEP                                                       
** filename : j_dma.h                                                      
** version  : 1                                                            
** date     : 13/02/23                                                     
**                                                                         
***************************************************************************
**                                                                         
** Copyright (c) EPFL contributors.                                        
** All rights reserved.                                                    
**                                                                         
***************************************************************************
	 
VERSION HISTORY:
----------------
Version     : 1
Date        : 13/02/2023
Revised by  : Juan Sapriza
Description : Original version.

*/

/***************************************************************************/
/***************************************************************************/

/**
* @file   j_dma.h
* @date   13/02/23
* @brief  The Direct Memory Access (DMA) driver to set up and use the DMA peripheral
*
* 
* 
*/

#ifndef _DMA_DRIVER_H
#define _DMA_DRIVER_H

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include <stddef.h>
#include <stdint.h>

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

// juan: fix this
#define DMA_MEM_PTR_MAX 1000
#define DMA_MEM_SIZE_MAX 1000
#define DMA_MEM_INC_MAX 4


/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

typedef enum
{
    DMA_SPI_MODE_DISABLE    = 0,
    DMA_SPI_MODE_RX         = 1,
    DMA_SPI_MODE_TX         = 2,
    DMA_SPI_MODE_FLASH_RX   = 3,     
    DMA_SPI_MODE_FLASH_TX   = 4,     
    DMA_SPI_MODE__size     
} dma_spi_mode_t;

typedef enum
{
    DMA_DATA_TYPE_WORD              = 0,
    DMA_DATA_TYPE_HALF_WORD         = 1,
    DMA_DATA_TYPE_BYTE              = 2,
    DMA_DATA_TYPE_BYTE_deprecated   = 3,
    DMA_DATA_TYPE__size
} dma_data_type_t;


/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/


// juan q: what would this do? 
// #ifndef _TEMPLATE_C_SRC
// #endif  /* _TEMPLATE_C_SRC */

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Obtains basic information without which no operation can be done with the DMA
 */
void dma_init();

/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/

/**
 * 
 * @brief Write to the read (source) pointer register of the DMA.
 * @param p_src Any valid memory address.
 */
inline void dma_set_src( uint32_t * p_src );

/**
 * @brief Write to the write (destination) pointer register of the DMA.
 * @param p_src Any valid memory address.
 */
inline void dma_set_dst( uint32_t * p_dst );

/**
 * @brief Write to the count/start register of the DMA // juan q: review this explanation.
 * @param p_copySize_du Size (in data units) to be copied from the source to the destination pointers.
 *                      Number of data units (du) = copy size in bytes / size of the data type.
 *                      e.g. If 16 Half Words (DMA_DATA_TYPE_HALF_WORD) are to be copied then p_copySize_du = 16.   
 */                       // juan q: in bytes or in data units?
inline void dma_set_cnt_start( uint32_t * p_copySize_du);

/**
 * @brief Read from the done register of the DMA.
 * @return juan q: no idea what is returned
 */
inline int32_t dma_is_done();

/**
 * @brief Write to the source-pointer-increment register of the DMA.
 * @param p_src Any valid memory address.
 */
inline void dma_set_src_ptr_inc( uint32_t p_inc );

/**
 * @brief Write to the destination-pointer-increment register of the DMA.
 * @param p_src Any valid memory address.
 */
inline void dma_set_dst_ptr_inc( uint32_t p_inc );

/**
 * @brief Write to the SPI mode register of the DMA.
 * @param p_src A valid SPI mode:
 *              - DMA_SPI_MODE_DISABLE    : Not use the DMA. //juan: add a sdk function to do this
                - DMA_SPI_MODE_RX         : Receive from SPI. Wait for Tx FIFO. //juan q: what is this?
                - DMA_SPI_MODE_TX         : Send to SPI. Wait for Rx FIFO.
                - DMA_SPI_MODE_FLASH_RX   : Receive from SPI Flash.     
                - DMA_SPI_MODE_FLASH_TX   : Send to SPI Flash. 
 */
inline void dma_set_spi_mode( dma_spi_mode_t p_mode);

/**
 * @brief Write to the data type register of the DMA.
 * @param p_src A valid data type. // juan: specify which, they are in dma_regs.h
 */
inline void dma_set_data_type( dma_data_type_t p_type);


#endif /* _DMA_DRIVER_H */
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/