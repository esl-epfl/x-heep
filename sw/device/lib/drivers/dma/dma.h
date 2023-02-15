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

// juan ! fix this
#define DMA_MEM_PTR_MAX 1000
#define DMA_MEM_SIZE_MAX 1000
#define DMA_MEM_INC_MAX 4


// ToDo: Juan - remove this, is just a placeholder until real assert can be included
// juan q jose: can we make the assert include some variable as parameter so we can know the "wrong value" ?? 
#define make_sure_that(x)

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
    DMA_DIR_M2M            = 0, // Reads from memory, writes in memory
    DMA_DIR_SPI_RX         = 1, // Reads from SPI, writes in memory
    DMA_DIR_SPI_TX         = 2, // Reads from memory, writes in SPI
    DMA_DIR_SPI_FLASH_RX   = 3, // Reads from SPI Flash, writes in memory    
    DMA_DIR_SPI_FLASH_TX   = 4, // Reads from memory, writes in SPI Flash   
    DMA_DIR__size     
} dma_dir_t;

/**
 *  All the valid data types for the DMA transfer.
 *  Word            = 4 bytes   = 32 bits
 *  Half Word       = 2 bytes   = 16 bits
 *  Byte/Byte alt   = 1 byte    = 8 bits 
 */
typedef enum
{
    DMA_DATA_TYPE_WORD              = 0,
    DMA_DATA_TYPE_HALF_WORD         = 1,
    DMA_DATA_TYPE_BYTE              = 2,
    //DMA_DATA_TYPE_BYTE_alt        = 3, // BYTE and BYTE_alt are interchangeable in hw, but we advice against the use of BYTE_alt.
                                         // By using the alternative, some functions/macros like DATA_TYPE_2_SIZE would brake. 
    DMA_DATA_TYPE__size
} dma_data_type_t;

/**
 * Possible way of knowing a DMA transfer has finished. 
 * This also determines what the application will consider as "finished".
 * e.g. For SPI transmission, the application may consider the transfer has 
 * finished once the DMA has transferred all its data to the SPI buffer (in 
 * which case POLLING or INTERRUPT should be used), or might as well wait 
 * until the SPI has finished sending all the data in its buffer (in which 
 * case SPI should be used).
 */
typedef enum
{
    DMA_END_EVENT_POLLING,      // Default. Application must query the DONE register flag with dma_is_done() to know when the transfer finished. DMA interrupts are disabled.
    DMA_END_EVENT_INTERRUPT,    // The application will receive a DMA interrupt once the DMA transfer has finished (i.e. the DONE register flag is high).   
    DMA_END_EVENT_SPI,          // The application will receive an SPI interrupt once the SPI process has finished. DMA interrupts are disabled.
    DMA_END_EVENT__size
} dma_end_event_t; // juan: rename


typedef enum{
    DMA_SAFETY_NO_CHECKS        = 0b00000000, // No checks will be performed. dma_launch() will go straight to writing registers.  
    DMA_SAFETY_SANITY_CHECKS    = 0b00000001, // Only sanity checks will be performed that no values are off-limits. 
    DMA_SAFETY_INTEGRITY_CHECKS = 0b00000010, // Integrity of the parameters is checked to make sure there are no inconsistencies. Not using this flag is only recommended when parameters are constant and the proper operation has been previously tested. 
    DMA_SAFETY__size        
} dma_safety_level_t;


/**
 * In some cases the DMA can overcome a misalignment issue if the data type is set to a smaller size.
 * This action can be performed by the dma_launch() function if allowed by user. 
 * 
 */
typedef enum
{
    DMA_DO_NOT_ALLOW_REALIGN  = 0,
    DMA_ALLOW_REALIGN         = 1,
} dma_allow_realign_t;


/**
 * Possible returns of the dma_launch() function.
 * Some of these issues or not a problem per se, yet a combination of them might be. 
 * For this reason, each error has only one high bit. This way they can be masked together
 * using the bitwise OR operator: ( DMA_LAUNCH_x | DMA_LAUNCH_y | DMA_LAUNCH_z ).
 * The *_SRC and *_DST labels identify in which arrangements issues where encountered.
 * 
 * A flag can be unset using the bitwise AND and NOT operators: x &= ~DMA_LAUNCH_*    
 */
typedef enum
{
    DMA_LAUNCH_OK               = 0b00000000,    // DMA transfer was successfully launched. 
    DMA_LAUNCH_SRC              = 0b00000001,    // An issue was encountered in the source arrangement.
    DMA_LAUNCH_DST              = 0b00000010,    // An issue was encountered in the destination arrangement.
    DMA_LAUNCH_MISALIGN         = 0b00000100,    // An arrangement is misaligned.
    DMA_LAUNCH_OVERLAP          = 0b00001000,    // The increment is smaller than the data type size.
    DMA_LAUNCH_DISCONTINUOUS    = 0b00010000,    // The increment is larger than the data type size.
    DMA_LAUNCH_OVERFLOW         = 0b00100000,    // The operation goes beyond the memory boundries.
    DMA_LAUNCH_CRITICAL_ERROR   = 0b10000000,    // This flag determines the function will return without the DMA performing any actions.
} dma_launch_ret_t;


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
  
  /**
   * Determines which event will determine the end of the transfer. 
   */
  dma_end_event_t endEvent;

  /**
   * The value returned from the last call to dma_launch().    
   */
  dma_launch_ret_t launchResult;
} dma_cb_t; 

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

// juan q jose: Would it be smart to make all these functions inline if then I need to extern this control block?
// Id rather have the cb static'd in the source to guarantee no manipulation.
// juan q jose: consider that if someone accesses the dma_cb not rhough these functions, its data could be incorrect because it has not been sync'd with the register

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

// juan q jose: In BINDI/Stack/HAL/drivers_nrf/uart/nrf_drv_uart.h functions defined in .h have the _STATIC_INLINE macro.... how is that ok????



/**
 * @brief Gets the number of data units not yet transferred. 
 * @return Number of data units.
 */
inline uint32_t dma_get_cnt_du()
{
    uint32_t ret = mmio_region_read32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DMA_START_REG_OFFSET));
    make_sure_that( /* juan q ruben: what could be asserted in this case? */ );
    // juan: This will not be done? Count element in struct will always be not-in-sync : dma_cb.ctrl.DMA_START = ret;
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
  make_sure_that( ret == 0 || ret == 1 );
   // juan: This will not be done? Count element in struct will always be not-in-sync : dma_cb.ctrl.DONE = ret;
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

#endif /* _DMA_DRIVER_H */
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/

                                                 
// juan q jose: Consider in set operations only backing up to the dma_cb struct and then have a "flush" function to launch the operation. 
//              This avoids unnecessary communication with the DMA.
//              Could help to avoid inconsistencies like: SPI but DMA int enabled, or INC being smaller than data unit 












