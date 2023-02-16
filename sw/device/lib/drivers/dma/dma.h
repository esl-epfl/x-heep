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
} dma_end_event_t; 


/**
 * It is possible to choose the level of safety with which the DMA operation should be launched. 
 * Not performing checks reduces the DMA usage overhead, but may result in a faulty operation, especially if
 * the configurations set to the DMA are not fixed but rather depend on the circumstance.
 * e.g. The source pointer is obtained as a result of a loop. It could happen the pointer ends up pointing outside the memory range, 
 * or that the pointer is close enough the the memory end to cause an overflow during reading. 
 */
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


/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

// juan q jose: what would this do? 
// #ifndef _TEMPLATE_C_SRC
// #endif  /* _TEMPLATE_C_SRC */

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Does the initial set up of the DMA control block.
 */
void dma_init();

/**
 * @brief Sets the read (source) pointer of the DMA.
 * @param p_src Any valid memory address.
 */
void dma_set_src( uint32_t * p_src );

/**
 * @brief Sets the write (destination) pointer of the DMA.
 * @param p_src Any valid memory address.
 */
void dma_set_dst( uint32_t * p_dst );

/**
 * @brief Sets the copy size (source) of the DMA 
 * @param p_copySize_du Size (in data units) to be copied from the source to the destination pointers.
 *                      Number of data units (du) = copy size in bytes / size of the data type.
 *                      e.g. If 10 Half Words (DMA_DATA_TYPE_HALF_WORD) are to be copied then p_copySize_du = 10,
 *                      not the number of bytes (20).   
 */                     
void dma_set_size( uint32_t * p_copySize_du);

/**
 * @brief Write to the source-pointer-increment register of the DMA.
 * @param p_inc_du Number of data units to increment after each read.
 */
void dma_set_src_ptr_inc( uint32_t p_inc_du );

/**
 * @brief Write to the destination-pointer-increment register of the DMA.
 * @param p_inc_du Number of data units to increment after each write.
 */
void dma_set_dst_ptr_inc( uint32_t p_inc_du );

/**
 * @brief Write to the SPI mode register of the DMA.
 * @param p_src A valid SPI mode:
 *              - DMA_DIR_M2M            : Transfer Memory to Memory.
                - DMA_DIR_SPI_RX         : Receive from SPI. Wait for Tx FIFO. //juan: remove the Wait for Tx FIFO thing. This is set by the user with the end event. 
                - DMA_DIR_SPI_TX         : Send to SPI. Wait for Rx FIFO.
                - DMA_DIR_SPI_FLASH_RX   : Receive from SPI Flash.     
                - DMA_DIR_SPI_FLASH_TX   : Send to SPI Flash. 
 */
void dma_set_direction( dma_dir_t p_dir);


/**
 * @brief Write to the data type register of the DMA.
 * @param p_src A valid data type.
 *               *  DMA_DATA_TYPE_WORD      = 4 bytes   = 32 bits
                 *  DMA_DATA_TYPE_HALF_WORD = 2 bytes   = 16 bits
                 *  DMA_DATA_TYPE_BYTE      = 1 byte    = 8 bits 
 */
void dma_set_data_type( dma_data_type_t p_type);


/**
 * @brief Sets the type of event that will determine the end of the transfer.
 * @param p_event A valid type of event.
 */
void dma_set_end_event( dma_end_event_t p_event );


/**
 * @brief Gets the number of data units not yet transferred. 
 * @return Number of data units.
 */
uint32_t dma_get_cnt_du();


/**
 * @brief Read from the done register of the DMA.
 * @return Whether the DMA is working or not. 
 * @retval 0 - DMA is working.   
 * @retval 1 - DMA has finished the transmission. DMA is idle. 
 */
inline uint32_t dma_is_done();

/**
 * @brief Writes the configuration values stored in the DMA control block into their corresponding registers. 
 *        Beforehand, sanity checks are performed and errors are returned in case any condition is violated.
 *        Also, compulsory values are added when needed.
 * @param p_safetyLevel Whether to perform sanity checks (if globally enabled). More than one condition can be masked with the bitwise or operand (x|y).         
 * @param p_allowRealign Whether to allow dma_launch() to change data type and increments to avoid misalignments.  
 * @return A mask of dma_launch_t values. Individual flags can be checked using the bitwise AND operator. e.g. ( ret & DMA_LAUNCH_* ) != 0 if the DMA_LAUNCH_* flag is set in the returned value. 
 */
dma_launch_ret_t dma_launch( dma_safety_level_t p_safetyLevel, dma_allow_realign_t p_allowRealign );

/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/

// juan q jose: In BINDI/Stack/HAL/drivers_nrf/uart/nrf_drv_uart.h functions defined in .h have the _STATIC_INLINE macro.... how is that ok????


#endif /* _DMA_DRIVER_H */
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/

                                                 
// juan q jose: Consider in set operations only backing up to the dma_cb struct and then have a "flush" function to launch the operation. 
//              This avoids unnecessary communication with the DMA.
//              Could help to avoid inconsistencies like: SPI but DMA int enabled, or INC being smaller than data unit 












