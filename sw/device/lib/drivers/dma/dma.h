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
* @file   dma.h
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

#include "dma_structs.h"  // Generated
#include "dma_regs.h"     // Generated


#include "core_v_mini_mcu.h" // @ToDo: Include this inside _regs.h

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/


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
    DMA_SMPH_MEMORY = 0, // Reads from memory, writes in memory
    DMA_SMPH_SLOT_1 = 1, // SLOT_1~4 are the available slots for adding semaphores.  
    DMA_SMPH_SLOT_2 = 2, // These are defined in hardware, 
    DMA_SMPH_SLOT_3 = 3, // so it should be consistent with the registers' 
    DMA_SMPH_SLOT_4 = 4, // assigned values. 
    DMA_SMPH__size,     
    DMA_SMPH__undef,// Default. DMA will not be used.
} dma_semaphore_t;  


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
 * It is possible to choose the level of safety with which the DMA operation should be configured. 
 * Not performing checks reduces the DMA usage overhead, but may result in a faulty operation, especially if
 * the configurations set to the DMA are not fixed but rather depend on the circumstance.
 * e.g. The source pointer is obtained as a result of a loop. It could happen the pointer ends up pointing outside the memory range, 
 * or that the pointer is close enough the the memory end to cause an overflow during reading. 
 */
typedef enum{
    DMA_PERFORM_CHECKS_ONLY_SANITY = 0x00, // No checks will be performed. Only sanity checks will be performed that no values are off-limits or containing errors.
    DMA_PERFORM_CHECKS_INTEGRITY   = 0x01, // Sanity AND integrity of the parameters is checked to make sure there are no inconsistencies. Not using this flag is only recommended when parameters are constant and the proper operation has been previously tested. 
} dma_perform_checks_t; 


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
 * Different possible actions that determine the end of the DMA transaction. This choice does not affect the transaction, but only the way the application is notified of its finalization. 
 * Consider that for INTR and INTR_WAIT global interrupts must be enabled with: CSR_SET_BITS(CSR_REG_MSTATUS, 0x8 ) (Or something of the sort).  
 * e.g. For SPI transmission, the application may consider the transfer has finished once the DMA has transferred all its data to the SPI buffer (Use any en event), 
 * or might prefer to wait until the SPI has finished sending all the data in its buffer (in which case POLLING could be chosen to disable DMA interrupts simply expect the SPI interrupt).
 */
typedef enum
{
    DMA_END_EVENT_POLLING,      // Interrupt for the DMA will be disabled. The application will be in charge of monitoring the end of the transaction.
    DMA_END_EVENT_INTR,         // Interrupt for the DMA will be enabled. After launching the transaction, the dma_launch function will exit. 
    DMA_END_EVENT_INTR_WAIT,    // Interrupt for the DMA will be enabled. After launching the transaction, the dma_launch function will wait in a wait_for_interrupt (wfi) state.  
    DMA_END_EVENT__size, 
} dma_end_event_t;


/**
 * Possible returns of the dma_configure() function.
 * Some of these issues or not a problem per se, yet a combination of them might be. 
 * For this reason, each error has only one high bit. This way they can be masked together
 * using the bitwise OR operator: ( DMA_CONFIG_x | DMA_CONFIG_y | DMA_CONFIG_z ).
 * The *_SRC and *_DST labels identify in which arrangements issues where encountered.
 * 
 * A flag can be unset using the bitwise AND and NOT operators: x &= ~DMA_CONFIG_*    
 */
typedef enum
{
    DMA_CONFIG_OK               = 0x00,    // DMA transfer was successfully configured. 
    DMA_CONFIG_SRC              = 0x01,    // An issue was encountered in the source arrangement. 
    DMA_CONFIG_DST              = 0x02,    // An issue was encountered in the destination arrangement.
    DMA_CONFIG_MISALIGN         = 0x04,    // An arrangement is misaligned.
    DMA_CONFIG_OVERLAP          = 0x08,    // The increment is smaller than the data type size.
    DMA_CONFIG_DISCONTINUOUS    = 0x10,    // The increment is larger than the data type size.
    DMA_CONFIG_OUTBOUNDS        = 0x20,    // The operation goes beyond the memory boundaries.
    DMA_CONFIG_INCOMPATIBLE     = 0x40,    // Different arguments result in incompatible requests.
    DMA_CONFIG_CRITICAL_ERROR   = 0x80,    // This flag determines the function will return without the DMA performing any actions.
} dma_config_flags_t;


/*
 * An environment is a region of memory defined by its start and end pointers. 
 * The sole purpose of creating environments is preventing the DMA from writing on 
 * restricted memory regions (outside the environment).
 */
typedef struct
{
    uint8_t* start;
    uint8_t* end;
} dma_env_t;

/*
 * A target is a region of memory from/to which the DMA can copy data. 
 * It is defined by its start pointer and the size of the data that can be copied. 
 * Furthermore, control parameters can be added to prevent the DMA from reading/writing outside the
 * boundaries of the target. 
 */
typedef struct
{
    dma_env_t* env;             // The environment to which this target belongs. It may be null (no checks will be performed to guarantee the write operations are not performed beyond limits). This is always null if the target is a peripheral. 
    uint8_t* ptr;              // Pointer to the start address from/to where data will be copied/pasted.
    uint32_t inc_du;            // How much the pointer will increase every time a read/write operation is done. It is a multiple of the data units. Can be left blank if the target is a peripheral.
    uint32_t size_du;           // The size (in data units) of the data to be copied. Can be left blank if the target will only be used as destination.
    dma_data_type_t type;       // The type of data to be transferred. Can be left blank if the target will only be used as destination.
    dma_semaphore_t smph;       // If the target is a peripheral, a semaphore can be set to control the data flow. 
    dma_config_flags_t flags;   // A mask with possible issues aroused from the creation of the target. 
} dma_target_t;   

/*
 * A transaction is an agreed transfer of data from one target to another. 
 * It needs a source target and a destination target. 
 * It also includes control parameters to override the targets' specific ones if needed.
 */
typedef struct
{
    dma_target_t* src;          // Target from where the data will be copied.
    dma_target_t* dst;          // Target to where the data will be copied.
    uint32_t inc_b;             // A common increment in case both targets need to use one same increment. 
    uint32_t size_b;            // The size of the transfer, in bytes (in contrast, the size stored in the targets is in data units).   
    dma_data_type_t type;       // The data type to use. One is chosen among the targets.
    dma_semaphore_t smph;       // The semaphore to use. One is chosen among the targets.
    dma_end_event_t end;        // What should happen after the transaction is launched. 
    dma_config_flags_t flags;   // A mask with possible issues aroused from the creation of the transaction.
} dma_trans_t;



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

/**
 *@brief Takes all DMA configurations to a state where no accidental transaction can be performed. 
 * It can be called anytime to reset the DMA control block.   
 */
void dma_init();


/**
 * @brief Creates an environment where targets can be added. An environment is, 
 * for instance, a RAM block, or section of memory that can be used by the DMA. 
 * Properly defining an environment can prevent the DMA from accessing restricted
 * memory regions. 
 * Targets for peripherals do not need an environment.
 * @param p_env Pointer to the dma_env_t structure where information will be allocated. The content of this pointer must be a static variable. 
 * @param p_start Pointer to the first accessible address of the environment.   
 * @param p_end Pointer to the last accessible address of the environment.
 * @return A configuration flags mask. Each individual flag can be accessed with a bitwise AND ( ret & DMA_CONFIG_* ). It is not recommended to query the result from inside
 * environment structure as an error could have appeared before the creation of the structure.
 */
dma_config_flags_t dma_create_environment( dma_env_t *p_env, uint8_t* p_start, uint8_t* p_end );


/**
 * @brief Creates a target that can be used to perform transactions. 
 * @param p_tgt Pointer to the dma_target_t structure where information will be allocated. The content of this pointer must be a static variable. 
 * @param p_ptr Pointer to the first element of the target range. 
 * @param p_inc_du Increment, in multiples of the data unit, between each read or write operation. A value of 1 will read/write all data consecutively.   
 * @param p_size_du Number of data units to be copied. Only used for reading operations (i.e. when the target is used as source).
 * @param p_type Data type to be used when reading or writing in the target range. 
 * @param p_smph Which semaphore to use to control the reading or writing rate. A value of 0 will allow writing at full-speed. 
 * @param p_env Environment to which this target belongs. A NULL pointer will assign no environment and pointer and ranges will not be checked for outbounds. 
 * @param p_check Whether integrity checks should be performed. 
 * @return A configuration flags mask. Each individual flag can be accessed with a bitwise AND ( ret & DMA_CONFIG_* ). It is not recommended to query the result from inside
 * target structure as an error could have appeared before the creation of the structure.
 */
dma_config_flags_t dma_create_target( dma_target_t *p_tgt, uint8_t* p_ptr, uint32_t p_inc_du, uint32_t p_size_du, dma_data_type_t p_type, dma_semaphore_t p_smph, dma_env_t* p_env, dma_perform_checks_t p_check );


/**
 * @brief Creates a transaction that can be loaded into the DMA.
 * @param p_trans Pointer to the dma_transaction_t structure where information will be allocated. The content of this pointer must be a static variable. 
 * @param p_src Pointer to a target structure to be used as source for the transaction. 
 * The data type and copy size of the source are used by default for the transaction. 
 * @param p_dst Pointer to a target structure to be used as destination for the transaction.
 * @param p_end The end event will determine how the dma_launch proceeds after it has effectively launched the transaction. 
 * @param p_allowRealign Whether to allow the DMA to take a smaller data type in order to counter misalignments between the selected data type and the start pointer.
 * @param p_check Whether integrity checks should be performed. 
 * @retval DMA_CONFIG_CRITICAL_ERROR if an error was detected in the transaction to be loaded.
 * @retval DMA_CONFIG_OK == 0 otherwise.    
 */
dma_config_flags_t dma_create_transaction( dma_trans_t *p_trans, dma_target_t *p_src, dma_target_t *p_dst, dma_end_event_t p_end, dma_allow_realign_t p_allowRealign, dma_perform_checks_t p_check );

/**
 * @brief The transaction configuration (that has been previously validated through the creation functions) is effectively transferred into the DMA registers. 
 * @param p_trans Pointer to the transaction struct to be loaded into the DMA. The content of this pointer must be a static variable. 
 * @return A configuration flags mask. Each individual flag can be accessed with a bitwise AND ( ret & DMA_CONFIG_* ). It is not recommended to query the result from inside
 * target structure as an error could have appeared before the creation of the structure.
 */
dma_config_flags_t dma_load_transaction( dma_trans_t* p_trans );



/**
 * @brief Launches the loaded transaction. 
 * @param p_trans A pointer to the desired transaction. This is only used to double check that the loaded transaction is the desired one. 
 *                  This check can be avoided by passing a NULL pointer.
 * @retval DMA_CONFIG_CRITICAL_ERROR if the passed pointer does not correspond itself with the loaded transaction (i.e. it is likely the transaction to be loaded is not the desired one).
 * @retval DMA_CONFIG_OK == 0 otherwise.
 */
dma_config_flags_t dma_launch( dma_trans_t* p_trans );


/**
 * @brief Read from the done register of the DMA.
 * @return Whether the DMA is working or not. It starts returning 0 as soon as the dma_launch function has returned. 
 * @retval 0 - DMA is working.   
 * @retval 1 - DMA has finished the transmission. DMA is idle. 
 */
uint32_t dma_is_done();


/**
* @brief DMA interrupt handler.   
* `dma.c` provides a weak definition of this symbol, which can be overridden
* at link-time by providing an additional non-weak definition.
*/
void dma_intr_handler();


/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/

#endif /* _DMA_DRIVER_H */
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/