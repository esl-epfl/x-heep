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

#define DMA_TARGET_NO_SEMAPHORE 0


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
    DMA_DIR__size,     
    DMA_DIR__undef,// Default. DMA will not be used.
} dma_dir_t;   // juan: remove these. Now they will be semaphores defined upper in the foodchain





/**
 *  All the valid data types for the DMA transfer.
 *  Word            = 4 bytes   = 32 bits
 *  Half Word       = 2 bytes   = 16 bits
 *  Byte            = 1 byte    = 8 bits 
 */
typedef enum
{               
    DMA_DATA_TYPE_WORD              = 0,
    DMA_DATA_TYPE_HALF_WORD         = 1,
    DMA_DATA_TYPE_BYTE              = 2,
    //DMA_DATA_TYPE_BYTE_alt        = 3, // BYTE and BYTE_alt are interchangeable in hw, but we advice against the use of BYTE_alt.
                                         // By using the alternative, some functions/macros like DATA_TYPE_2_SIZE would brake. 
    DMA_DATA_TYPE__size,
    DMA_DATA_TYPE__undef, // Default. DMA will not be used.
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
    DMA_END_EVENT_POLLING   = 0x00, // Application must query the DONE register flag with dma_is_done() to know when the transfer finished. DMA interrupts are disabled.
    DMA_END_EVENT_DMA_INT   = 0x01, // The application will receive a DMA interrupt once the DMA transfer has finished (i.e. the DONE register flag is high).   
    DMA_END_EVENT_PERIP_INT = 0x02, // The application will receive a peripheral interrupt once the peripheral process has finished. DMA interrupts will be disabled.
    DMA_END_EVENT__size,
    DMA_END_EVENT__undef,       // Default. DMA will not be used. 
} dma_end_event_t; // juan: add this


/**
 * It is possible to choose the level of safety with which the DMA operation should be configured. 
 * Not performing checks reduces the DMA usage overhead, but may result in a faulty operation, especially if
 * the configurations set to the DMA are not fixed but rather depend on the circumstance.
 * e.g. The source pointer is obtained as a result of a loop. It could happen the pointer ends up pointing outside the memory range, 
 * or that the pointer is close enough the the memory end to cause an overflow during reading. 
 */
typedef enum{
    DMA_SAFETY_NO_CHECKS        = 0x00, // No checks will be performed. dma_configure() will go straight to writing registers.  
    DMA_SAFETY_SANITY_CHECKS    = 0x01, // Only sanity checks will be performed that no values are off-limits. It is a good way of not performing sanity check only during this operation. 
    DMA_SAFETY_INTEGRITY_CHECKS = 0x02, // Integrity of the parameters is checked to make sure there are no inconsistencies. Not using this flag is only recommended when parameters are constant and the proper operation has been previously tested. 
    DMA_SAFETY__size        
} dma_safety_level_t;  // juan: re-do this


/**
 * In some cases the DMA can overcome a misalignment issue if the data type is set to a smaller size.
 * This action can be performed by the dma_configure() function if allowed by user. 
 * 
 */
typedef enum
{
    DMA_DO_NOT_ALLOW_REALIGN  = 0,
    DMA_ALLOW_REALIGN         = 1,
} dma_allow_realign_t;


/**
 * Possible returns of the dma_configure() function.
 * Some of these issues or not a problem per se, yet a combination of them might be. 
 * For this reason, each error has only one high bit. This way they can be masked together
 * using the bitwise OR operator: ( DMA_CONFIGURATION_x | DMA_CONFIGURATION_y | DMA_CONFIGURATION_z ).
 * The *_SRC and *_DST labels identify in which arrangements issues where encountered.
 * 
 * A flag can be unset using the bitwise AND and NOT operators: x &= ~DMA_CONFIGURATION_*    
 */
typedef enum
{
    DMA_CONFIGURATION_OK               = 0x00,    // DMA transfer was successfully configured. 
    DMA_CONFIGURATION_SRC              = 0x01,    // An issue was encountered in the source arrangement. // juan: do I need these two? 
    DMA_CONFIGURATION_DST              = 0x02,    // An issue was encountered in the destination arrangement.
    DMA_CONFIGURATION_MISALIGN         = 0x04,    // An arrangement is misaligned.
    DMA_CONFIGURATION_OVERLAP          = 0x08,    // The increment is smaller than the data type size.
    DMA_CONFIGURATION_DISCONTINUOUS    = 0x10,    // The increment is larger than the data type size.
    DMA_CONFIGURATION_OUTBOUNDS        = 0x20,    // The operation goes beyond the memory boundries.
    DMA_CONFIGURATION__unused          = 0x40,    // 
    DMA_CONFIGURATION_CRITICAL_ERROR   = 0x80,    // This flag determines the function will return without the DMA performing any actions.
} dma_config_flags_t;



typedef uint8_t dma_trans_id_t; 





typedef struct
{
    uint32_t* start;
    uint32_t* end;
    //dma_env_characs_t characs; // juan : should we add restrictions to read/write
    //dma_end_event_t endEvents;
} dma_env_t;


typedef struct
{
    dma_env_t* env;
    uint32_t* ptr;
    uint32_t inc_du;
    uint32_t size_du;
    dma_data_type_t type;
    uint8_t semaphore;
    dma_config_flags_t flags;
} dma_target_t;   

typedef struct
{
    dma_target_t* src;
    dma_target_t* dst;
    dma_end_event_t end;
    dma_config_flags_t flags;
    dma_data_type_t type;
    uint32_t inc_b;
    uint32_t size_b;
} dma_trans_t;







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
 * @brief Read from the done register of the DMA.
 * @return Whether the DMA is working or not. 
 * @retval 0 - DMA is working.   
 * @retval 1 - DMA has finished the transmission. DMA is idle. 
 */
inline uint32_t dma_is_done();



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