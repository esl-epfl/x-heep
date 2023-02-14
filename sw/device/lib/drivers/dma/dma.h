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

// juan: include real assert after Jose
//#include <assert.h>

#include "dma_structs.h"  // Generated
#include "dma_regs.h"     // Generated


#include "../../base/mmio.h"    // @ToDo: Not make this relative. 
#include "../../runtime/core_v_mini_mcu.h" // @ToDo: Include this inside _regs.h

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

// juan: fix this
#define DMA_MEM_PTR_MAX 1000
#define DMA_MEM_SIZE_MAX 1000
#define DMA_MEM_INC_MAX 4


// ToDo: Juan - remove this, is just a placeholder until real assert can be included
#define assert_me_please(x)

// source: https://stackoverflow.com/questions/600293/how-to-check-if-a-number-is-a-power-of-2
#define IS_POWER_OF_2(x) ( x && ( ( x & -x ) == x ) )

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/**
 * All the valid SPI modes for the DMA-SPI operation.
 */
typedef enum
{
    DMA_SPI_MODE_DISABLE    = 0,
    DMA_SPI_MODE_RX         = 1,
    DMA_SPI_MODE_TX         = 2,
    DMA_SPI_MODE_FLASH_RX   = 3,     
    DMA_SPI_MODE_FLASH_TX   = 4,     
    DMA_SPI_MODE__size     
} dma_spi_mode_t;

/**
 *  All the valid data types for the DMA transfer.
 */
typedef enum
{
    DMA_DATA_TYPE_WORD              = 0,
    DMA_DATA_TYPE_HALF_WORD         = 1,
    DMA_DATA_TYPE_BYTE              = 2,
    DMA_DATA_TYPE_BYTE_alt          = 3,
    DMA_DATA_TYPE__size
} dma_data_type_t;

/**
 *  Control Block (CB) of the DMA peripheral. 
 * Has variables and constant necessary/useful for its control. 
 */
typedef struct 
{
  /**
   * Control variables for the DMA peripheral 
  */
  dma    ctrl; 
  /**
    * The base address for the soc_ctrl hardware registers.
   */
  mmio_region_t baseAdd; 
} dma_cb_t; 

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

// juan q jose: Would it be smart to make all these functions inline if then I need to extern this control block?
// Id rather have the cb static'd in the source to guarantee no manipulation.
// Otherwise, it can be a way of giving control to the user... but hmmmm....
extern dma_cb_t dma_cb;

// juan q jose: what would this do? 
// #ifndef _TEMPLATE_C_SRC
// #endif  /* _TEMPLATE_C_SRC */

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Obtains basic information for the operation of the DMA. 
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
inline void dma_set_src( uint32_t * p_src ) 
{                                           
  assert_me_please( p_src < DMA_MEM_PTR_MAX );        
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_PTR_IN_REG_OFFSET), p_src);
} // @ToDo: Rename DMA_PTR_IN_*  ->  DMA_SRC_PTR_* 


/**
 * @brief Write to the write (destination) pointer register of the DMA.
 * @param p_src Any valid memory address.
 */
inline void dma_set_dst( uint32_t * p_dst )
{
  assert_me_please( p_dst < DMA_MEM_PTR_MAX );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_PTR_OUT_REG_OFFSET), p_dst);
} // @ToDo: Rename DMA_PTR_OUT_*  ->  DMA_DST_PTR_*


/**
 * @brief Write to the count/start register of the DMA // juan q: review this explanation.
 * @param p_copySize_du Size (in data units) to be copied from the source to the destination pointers.
 *                      Number of data units (du) = copy size in bytes / size of the data type.
 *                      e.g. If 16 Half Words (DMA_DATA_TYPE_HALF_WORD) are to be copied then p_copySize_du = 16.   
 */                       // juan q: in bytes or in data units?
inline void dma_set_cnt_start( uint32_t * p_copySize_du)
{
  assert_me_please( p_copySize < DMA_MEM_SIZE_MAX );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DMA_START_REG_OFFSET), p_copySize_du);
} // @ToDo: Rename DMA_DMA_START_*  ->  DMA_CNT_START_*
// juan q ruben: Does the count keep the amount left to transmit? So if cnt == 0, done == 1 ? 


/**
 * @brief Gets the number of data units not yet transferred. 
 * @return Number of data units.
 */
inline uint32_t dma_get_cnt_du()
{
    uint32_t ret = mmio_region_read32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DMA_START_REG_OFFSET));
    assert_me_please( /* juan q ruben: what could be asserted in this case? */ );
    return ret;
}


/**
 * @brief Read from the done register of the DMA.
 * @return Whether the DMA is working or not. 
 * @retval 0 - DMA is working.   
 * @retval 1 - DMA has finished the transmission. DMA is idle. 
 */
inline uint32_t dma_is_done()
{
  uint32_t ret = mmio_region_read32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DONE_REG_OFFSET));
  assert_me_please( ret == 0 || ret == 1 );
  return ret;
  /* In case a return wants to be forced in case of an error, there are 2 alternatives: 
   *    1) Consider any value != 0 to be a valid 1 using a LOGIC AND: 
   *            return ( 1 && mmio_region_read32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DONE_REG_OFFSET)));
   *    2) Consider only the LSB == 1 to be a valid 1 using a BITWISE AND. 
   *            return ( 1 &  mmio_region_read32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DONE_REG_OFFSET)));
   * */   
}   // juan q jose: What to do in the above case
    // juan q ruben: if cnt== 0 => done == 1 ?? For how long? 
    // @ToDo: Rename DONE_*  ->  IDLE_*
// @ToDo: Make register DONE a 1 bit field in hw/ip/dma/data/dma.hjson. Watch out for compatibility/sync with hardware.  


/**
 * @brief Write to the source-pointer-increment register of the DMA.
 * @param p_src Any valid memory address.
 */
inline void dma_set_src_ptr_inc( uint32_t p_inc )
{
  assert_me_please( p_inc <= DMA_MEM_INC_MAX && IS_POWER_OF_2(p_inc) );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_SRC_PTR_INC_REG_OFFSET), p_inc);
}


/**
 * @brief Write to the destination-pointer-increment register of the DMA.
 * @param p_src Any valid memory address.
 */
inline void dma_set_dst_ptr_inc( uint32_t p_inc )
{
  assert_me_please( p_inc <= DMA_MEM_INC_MAX && IS_POWER_OF_2(p_inc) );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DST_PTR_INC_REG_OFFSET), p_inc);
}


/**
 * @brief Write to the SPI mode register of the DMA.
 * @param p_src A valid SPI mode:
 *              - DMA_SPI_MODE_DISABLE    : Not use the DMA. //juan: add a sdk function to do this
                - DMA_SPI_MODE_RX         : Receive from SPI. Wait for Tx FIFO. //juan q: what is this?
                - DMA_SPI_MODE_TX         : Send to SPI. Wait for Rx FIFO.
                - DMA_SPI_MODE_FLASH_RX   : Receive from SPI Flash.     
                - DMA_SPI_MODE_FLASH_TX   : Send to SPI Flash. 
 */
inline void dma_set_spi_mode( dma_spi_mode_t p_mode)
{
  assert_me_please( p_mode < DMA_SPI_MODE__size );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_SPI_MODE_REG_OFFSET), (uint32_t) p_mode);
}


/**
 * @brief Write to the data type register of the DMA.
 * @param p_src A valid data type. // juan: specify which, they are in dma_regs.h
 */
inline void dma_set_data_type( dma_data_type_t p_type)
{
  assert_me_please( p_type < DMA_DATA_TYPE__size );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DATA_TYPE_REG_OFFSET), ( uint32_t ) p_type);
} 


#endif /* _DMA_DRIVER_H */
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/

                                                 
                                                     














