/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************                          
**                                                                         
** project  : X-HEEP                                                       
** filename : dma.c                                                      
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
* @file   dma.c
* @date   13/02/23
* @brief  The Direct Memory Access (DMA) driver to set up and use the DMA 
* peripheral
*/

/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "dma.h"

/* To manage addresses. */
#include "mmio.h"

/* To manage interrupts. */
#include "fast_intr_ctrl.h"
#include "fast_intr_ctrl_regs.h"
#include "hart.h"
#include "handler.h"
#include "csr.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

/**
 * Returns the size in bytes of a certain datatype, as a sizeof(type) would. 
 */
#define DMA_DATA_TYPE_2_DATA_SIZE(type) (0b00000100 >> (type) )     

// @ToDo: Remove this, is just a placeholder until real assert can be included
#define make_sure_that(x) printf( "%s@%u\n\r",x ? "Success" : "Error",__LINE__ );

/**
 * Returns the mask to enable/disable DMA interrupts.
 */
#define DMA_CSR_REG_MIE_MASK ( 1 << 19 )

/**
 * Mask to use to write on a whole register.
 */
#define DMA_MASK_WHOLE_REGISTER 0xFFFFFFFF

/**
 * Size of a register of 32 bits. 
 */
#define DMA_REGISTER_SIZE_BYTES 4

/**
 * Mask to determine if an address is multiple of 4 (Word aligned).
 */
#define DMA_WORD_ALIGN_MASK 3

/**
 * Mask to determine if an address is multiple of 2 (Half Word aligned).
 */
#define DMA_HALF_WORD_ALIGN_MASK 1

/****************************************************************************/
/**                                                                        **/
/*                        TYPEDEFS AND STRUCTURES                           */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Creates a target that can be used to perform transactions. 
 * @param p_tgt Pointer to the dma_target_t structure where configuration
 * should be allocated. The content of this pointer must be a static variable. 
 * @return A configuration flags mask. Each individual flag can be accessed with 
 * a bitwise AND ( ret & DMA_CONFIG_* ).
 */
dma_config_flags_t validateTarget( dma_target_t *p_tgt );

/**
 * @brief Gets how misaligned a pointer is, taking into account the data type 
 * size. 
 * @param p_ptr The source or destination pointer. 
 * @return How misaligned the pointer is, in bytes. 
 */
static inline uint8_t getMisalignment_b( uint8_t* p_ptr, 
                                        dma_data_type_t p_type );


/**
 * @brief Determines whether a given region will fit before the end of an 
 * environment.
 * @param p_start Pointer to the beginning of the region.
 * @param p_end Pointer to the last byte of the environment.
 * @param p_type The data type to be transferred.
 * @param p_size_du The number of data units to be transferred. Must be 
 * non-zero.
 * @param p_inc_du The size in data units of each increment. 
 * @retval 1 There is an outbound.
 * @retval 0 There is NOT an outbound.   
 */
static inline uint8_t isOutbound( uint8_t* p_start, uint8_t* p_end, 
                                uint32_t p_type, uint32_t p_size_du, 
                                uint32_t p_inc_du );

/**
 * @brief Writes a given value into the specified register. Later reads the
 * register and checks that the read value is equal to the written one. 
 * This check is done through an assertion, so can be disabled by disabling 
 * assertions.
 * @param p_val The value to be written.
 * @param p_offset The register's offset from the peripheral's base address
 *  where the target register is located.
 * @param p_mask The variable's mask to only modify its bits inside the whole
 * register.
 */
static inline void writeRegister( uint32_t p_val, uint32_t p_offset, 
                                uint32_t p_mask );


/**
 * @brief Analyzes a target to determine the size of its increment (in bytes). 
 * @param p_tgt A pointer to the target to analyze. 
 * @return The number of bytes of the increment.
 */ 
static inline uint32_t getIncrement_b( dma_target_t * p_tgt );

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED VARIABLES                             */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                            GLOBAL VARIABLES                              */
/**                                                                        **/
/****************************************************************************/

/**
 * Control Block (CB) of the DMA peripheral. 
 * Has variables and constant necessary/useful for its control. 
 */
static struct
{
    /**
    * Pointer to the transaction to be performed. 
    */
    dma_trans_t* trans;
    
    /**
    * Flag to lower as soon as a transaction is launched, and raised by the 
    * interrupt handler once it has finished. 
    */
    uint8_t intrFlag;

    /**
    * Fast interrupt controller control block
    */
    fast_intr_ctrl_t fic;
}dma_cb;

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

void dma_init()
{
    // @ToDo: let fic be in charge of this.
    dma_cb.fic.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS); 
    /* Clear the loaded transaction */
    dma_cb.trans = NULL;
    /* Clear all values in the DMA registers. */
    dma_peri->PTR_IN = 0;
    dma_peri->PTR_OUT = 0;
    dma_peri->DMA_START = 0;
    dma_peri->SRC_PTR_INC = 0;
    dma_peri->DST_PTR_INC = 0;
    dma_peri->SPI_MODE = 0;
    dma_peri->DATA_TYPE = 0;
}

dma_config_flags_t dma_create_environment( dma_env_t *p_env )
{
    /*
     * SANITY CHECKS
     */

    if( (uint8_t*)p_env->end < (uint8_t*)p_env->start ) 
    {
        return DMA_CONFIG_INCOMPATIBLE;
    }    
    return DMA_CONFIG_OK;
}

dma_config_flags_t dma_create_transaction(  dma_trans_t *p_trans,  
                                            dma_allow_realign_t p_allowRealign, 
                                            dma_perform_checks_t p_check )
{
    /*
    * SANITY CHECKS
    */

    make_sure_that( (dma_end_event_t)p_trans->end < DMA_END_EVENT__size );

    /*
     * CHECK IF TARGETS HAVE ERRORS
     */

    /* 
     * The transaction is NOT created if the targets include errors.
     * A successful target validation has to be done before loading it to the DMA.
     */
    uint8_t errorSrc = validateTarget( p_trans->src );
    uint8_t errorDst = validateTarget( p_trans->dst );
    /* 
     * If there are any errors or warnings in the valdiation of the targets, they
     * are added to the transaction flags, adding the source/destination identifying
     * flags as well. 
     */
    p_trans->flags |= errorSrc ? (errorSrc | DMA_CONFIG_SRC ) : DMA_CONFIG_OK;
    p_trans->flags |= errorDst ? (errorDst | DMA_CONFIG_SRC ) : DMA_CONFIG_OK; 
    /* If a critical error was detected, no further action is performed. */
    if( p_trans->flags & DMA_CONFIG_CRITICAL_ERROR )
    {
        return p_trans->flags;
    }
    
    /*
     * CHECK IF THERE ARE SEMAPHORE INCONSISTENCIES 
     */

    /* 
     * The DMA can only handle one semaphore at a time, therefore, if the two 
     * targets require a semaphore, the transaction has to be discarded.
     * None of the two values can be taken by default because this inconsistency
     * is probably a result of an error (likely wrong target selection).
     */
    if( p_check )
    {
        if( p_trans->src->smph && p_trans->dst->smph ) 
        {
            p_trans->flags |= ( DMA_CONFIG_INCOMPATIBLE | DMA_CONFIG_CRITICAL_ERROR );
            return p_trans->flags;
        }
    }
    /* 
     * As there is only one non-null semaphore among the targets, that one 
     * is selected. 
     * If both are zero, it does not matter which is picked. 
     */
    p_trans->smph = p_trans->src->smph ? p_trans->src->smph : p_trans->dst->smph;
    
    /*
     * SET UP THE DEFAULT CONFIGURATIONS 
     */

    /* The flags are cleaned in case the structure was used before.*/
    p_trans->flags = DMA_CONFIG_OK;
    /* The copy size of the source (in data units -of the source-) is 
    transformed to bytes, to be used as default size.*/
    uint8_t dataSize_b = DMA_DATA_TYPE_2_DATA_SIZE(p_trans->src->type);
    p_trans->size_b = p_trans->src->size_du * dataSize_b;
    /* By default, the source defines the data type.*/
    p_trans->type = p_trans->src->type;
    /* 
     * By default, the transaction increment is set to 0 and, if required,
     * it will be changed to 1 (in which case both src and dst will have an
     * increment of 1 data unit).
     */
    p_trans->inc_b = 0;
    
    /*
     * CHECK IF THERE ARE MISALIGNMENTS 
     */

    if( p_check  )
    {
        /* 
         * The source and destination targets are analyzed. 
         * If the target is a peripheral (i.e. uses one of semaphore slots)
         * then the misalignment is not checked.
         */
        uint8_t misalignment = 0;
        if( ! p_trans->src->smph )
        {
            misalignment = getMisalignment_b( p_trans->src->ptr, p_trans->type  );
        }
        p_trans->flags |= ( misalignment ? DMA_CONFIG_SRC : DMA_CONFIG_OK ); 

        uint8_t dstMisalignment = 0;
        if( ! p_trans->dst->smph ) 
        {
            dstMisalignment = getMisalignment_b( p_trans->dst->ptr, p_trans->type  );
        }
        p_trans->flags  |= ( dstMisalignment ? DMA_CONFIG_DST : DMA_CONFIG_OK );

        /* Only the largest misalignment is preserved.*/
        if( misalignment < dstMisalignment )
        {
            misalignment = dstMisalignment;
        }

        if( misalignment )
        {
            /* 
             * Misalignment flags will only be stored in the transaction, as 
             * they are data type dependent, and could vary from transaction 
             * to transaction, even with the same pair of targets.
             */
            p_trans->flags |= DMA_CONFIG_MISALIGN;

            /* 
             * If a misalignment is detected and realignment is not allowed, 
             * an error is returned. No operation should be performed by the DMA.
             * No further operations are done to prevent corrupting information
             * that could be useful for debugging purposes. 
             */
            if( !p_allowRealign)
            {
                return p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
            }

            /*
             * CHECK IF THERE IS A DISCONTINUITY 
             */
 
            /*  
             * If there is a misalignment AND the source or destination 
             * arrangements are discontinuous, it is not possible to use the 
             * DMA. An error is returned.
             * A discontinuity in an arrangement is defined as the increment
             * being larger than the data type size. 
             * e.g. 
             * |AAAA|AAAA|BBBB|BBBB|      |____|AAAA|AAAA|____|  
             * 0    1    2    3           0    1    2    3          
             *    
             * |CCCC|CCCC|____|____| ==>  |____|BBBB|BBBB|____| 
             * 4    5    6    7           4    5    6    7     
             *    
             * |____|____|____|____|      |____|CCCC|CCCC|____| 
             * 8    9    10   11          8    9    10   11    
             * 
             * In this case the source arrangement has 16bit HALF WORDs, with
             * an increment of 1 data unit, so all data is continuous. 
             * The destination arrangement has also 16bit HALF WORDs, but 
             * misaligned and with 2 data units of increment. 
             * To copy a misaligned arrangement of HALF WORDs, the DMA should
             * use BYTEs. However, by using bytes it would not be able to 
             * write twice and skip twice, as it has a single skip increment. 
             * 
             * The misalignment and discontinuity can be found in the source 
             * and destination respectively and vice versa, and this 
             * limitation would still exist.  
             * 
             * The discontinuous flag is added (the misaligned one was 
             * already there), and it is turned into a critical error.'
             * 
             * No further operations are done to prevent corrupting
             * information that could be useful for debugging purposes.
             */
            if( ( p_trans->src->inc_du > 1 ) || ( p_trans->dst->inc_du > 1 ) )
            {
                p_trans->flags |= DMA_CONFIG_DISCONTINUOUS;
                p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
                return p_trans->flags; 
            }

            /*
             * PERFORM THE REALIGNMENT 
             */
 
            /* 
             * If realignment is allowed and there are no discontinuities,
             * a more granular data type is used according to the detected 
             * misalignment in order to overcome it. 
             */       
            p_trans->type += misalignment;
            /* 
             * Source and destination increment should now be of the size
             * of the data.
             * As increments are given in bytes, in both cases should be the
             * size of a data unit. 
             */
            p_trans->inc_b = DMA_DATA_TYPE_2_DATA_SIZE( p_trans->type );
            /* The copy size does not change, as it is already stored in bytes.*/
        }
    
        /*
         * CHECK IF SOURCE HAS SIZE 0
         */

        /*
         * No further operations are done to prevent corrupting information 
         * that could be useful for debugging purposes. 
         */
        if( p_trans->src->size_du == 0 ) 
        {
            p_trans->flags |= DMA_CONFIG_SRC;
            p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
            return p_trans->flags; 
        }
        
        /*
         * CHECK IF THERE IS CROSS-OUTBOUND
         */

        /* 
         * As the source target does not know the constraints of the 
         * destination target, it is possible that the copied information
         * ends up beyond the bounds of the destination environment. 
         * This check is not performed if no destination environment was set.
         *
         * e.g. 
         * If 10 HALF WORDs are to be transferred with a HALF WORD in between
         * (increment of 2 data units: writes 2 bytes, skips 2 bytes, ...),
         * then ( 10 data units * 2 bytes ) + ( 9 data units * 2 bytes ) are
         * traveled for each = 38 bytes 
         * that need to be gone over in order to complete the transaction.
         * Having the transaction size in bytes, they need to be converted 
         * to data units:
         * size [data units] = size [bytes] / convertionRatio [bytes/data unit].
         * 
         * The transaction can be performed if the whole affected area can fit
         * inside the destination environment
         * (i.e. The start pointer + the 38 bytes -in this case-, is smaller
         * than the end pointer of the environment). 
         * 
         * No further operations are done to prevent corrupting information
         * that could be useful for debugging purposes.  
         */ 
        uint8_t isEnv = p_trans->dst->env;
        uint8_t isOutb = isOutbound(
                                    p_trans->dst->ptr, 
                                    p_trans->dst->env->end,
                                    p_trans->type,
                                    p_trans->src->size_du,
                                    p_trans->dst->inc_du );
        if( isEnv && isOutb )
        {
            p_trans->flags |= DMA_CONFIG_DST; 
            p_trans->flags |= DMA_CONFIG_OUTBOUNDS; 
            p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR;
            return p_trans->flags;  
        }
        
        // @ToDo: It should also be checked that the destination is behind the 
        // source if there will be overlap. 
        // @ToDo: Consider if (when a destination target has no environment) 
        // the destination size should be used as limit. 
    }   
        
    return p_trans->flags;
}

dma_config_flags_t dma_load_transaction( dma_trans_t* p_trans )
{
    
    /* 
     * The transaction is not allowed if it contain a critical error.
     * A successful transaction creation has to be done before loading it to
     * the DMA.
     */
    if( p_trans->flags & DMA_CONFIG_CRITICAL_ERROR )
    {   
        dma_cb.trans = NULL;
        return DMA_CONFIG_CRITICAL_ERROR;
    }
    
    /* Save the current transaction */
    dma_cb.trans = p_trans; 
    
    /*
     * ENABLE/DISABLE INTERRUPTS
     */

    /* 
     * If the selected en event is polling, interrupts are disabled. 
     * Otherwise the mie.MEIE bit is set to one to enable machine-level
     * fast DMA interrupt.
     */
    if( dma_cb.trans->end != DMA_END_EVENT_POLLING ){
        CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK );
    }
    else
    {
        CSR_CLEAR_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK );
    }


    /*
     * SET THE POINTERS
     */

    dma_peri->PTR_IN = dma_cb.trans->src->ptr;
    dma_peri->PTR_OUT = dma_cb.trans->dst->ptr;
    
    /*
     * SET THE INCREMENTS
     */

    /* 
     * The increments might have been changed (vs. the original value of 
     * the target) due to misalignment issues. If they have, use the changed
     * values, otherwise, use the target-specific ones.
     * Other reason to overwrite the target increment is if a semaphore is used.
     * In that case, a increment of 0 is necessary.
     */

    dma_peri->SRC_PTR_INC = getIncrement_b( dma_cb.trans->src );
    dma_peri->DST_PTR_INC = getIncrement_b( dma_cb.trans->dst );
    
    /*
     * SET SEMAPHORE AND DATA TYPE
     */    

    writeRegister( dma_cb.trans->smph, 
                DMA_SPI_MODE_REG_OFFSET, 
                DMA_SPI_MODE_SPI_MODE_MASK );
    writeRegister( dma_cb.trans->type, 
                DMA_DATA_TYPE_REG_OFFSET, 
                DMA_DATA_TYPE_DATA_TYPE_MASK );
    
    return DMA_CONFIG_OK;
}

dma_config_flags_t dma_launch( dma_trans_t* p_trans )
{
    /*
     * Make sure that the loaded transaction is the intended transaction. 
     * If the loaded trans was NULL'd, then this the transaction is never 
     * launched.
     */
    if( !p_trans || ( dma_cb.trans != p_trans ) )
    {
        return DMA_CONFIG_CRITICAL_ERROR;
    }
    
    /*
     * SET SIZE TO COPY + LAUNCH THE DMA OPERATION
     */

    /*
     * This has to be done prior to writing the register because otherwise
     * the interrupt could arrive before it is lowered.
     */
    dma_cb.intrFlag = 0;

    /* Load the size and start the transaction. */
    dma_peri->DMA_START = dma_cb.trans->size_b;

    /* 
     * If the end event was set to wait for the interrupt, the dma_launch
     * will not return until the interrupt arrives. 
     */
    while( p_trans->end == DMA_END_EVENT_INTR_WAIT && ! dma_cb.intrFlag ) {
        wait_for_interrupt();
    }

    return DMA_CONFIG_OK;
}


uint32_t dma_is_done()
{       
    uint32_t ret = dma_peri->DONE;
    make_sure_that( ret == 0 || ret == 1 );
    return ret;
}
/* @ToDo: Reconsider this decision.
 * In case a return wants to be forced in case of an error, there are 2 
 * alternatives: 
 *    1) Consider any value != 0 to be a valid 1 using a LOGIC AND: 
 *  return ( 1 && dma_peri->DONE );
 *    2) Consider only the LSB == 1 to be a valid 1 using a BITWISE AND. 
 *  return ( 1 &  dma_peri->DONE );
 * This would be fixed if the DONE register was a 1 bit field. 
 */   

__attribute__((weak)) void dma_intr_handler()
{
    /* 
     * The DMA transaction has finished!
     * This is a weak implementation.
     * Create your own function called 
     * void dma_intr_handler() 
     * to override this one.  
     * The sole purpose of this trivial code is to make sure the weak 
     * implementation is not optimized by the compiler. 
     */
    volatile uint8_t i;
    i++;
}
/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

dma_config_flags_t validateTarget( dma_target_t *p_tgt )
{
    /* Flags variable to pass encountered errors. */
    dma_config_flags_t flags;
    /*
     * SANITY CHECKS
     */

    /* Increment can be 0 when a semaphore is used. */
    make_sure_that( p_tgt->inc_du >= 0 ); 
    /* The size could be 0 if the target is only going to be used as a 
    destination. */
    make_sure_that( p_tgt->size_du >=  0 ); 
    /* The data type must be a valid type */
    make_sure_that( p_tgt->type < DMA_DATA_TYPE__size );
    /* The semaphore must be among the valid semaphore values. */
    make_sure_that( p_tgt->smph < DMA_SMPH__size );
    
    /*
     * INTEGRITY CHECKS
     */

    /* Only performed if an environment was set.*/
    uint8_t isEnv = p_tgt->env;
    /* If the target starts before the environment starts.*/
    uint8_t beforeEnv = p_tgt->ptr < p_tgt->env->start;
    /* If the target starts after the environment ends. */
    uint8_t afterEnv = p_tgt->ptr > p_tgt->env->end;
    /* If a size was defined. */
    uint8_t isSize = p_tgt->size_du;
    /* If the target selected size goes beyond the boundaries of the 
    environment. Only computed if there is a size defined.*/
    uint8_t isOutb = isOutbound( p_tgt->ptr, p_tgt->env->end, 
                                p_tgt->type, p_tgt->size_du, 
                                p_tgt->inc_du );
    
    if( isEnv && ( beforeEnv || afterEnv || (isSize && isOutb) ) )
    {
        flags |= DMA_CONFIG_OUTBOUNDS;
    }

    /* 
    * If there is a semaphore, there should not be environments
    * nor increments (or its an incompatible peripheral).
    * Otherwise, an increment is needed (or its an incompatible
    * memory).
    */
    uint8_t isSmph = p_tgt->smph;
    uint8_t isInc = p_tgt->inc_du;
    uint8_t incomPeri = ( isSmph && ( isEnv || isInc ) );
    uint8_t incomMem = ( ! p_tgt->smph && ! p_tgt->inc_du );
    if( incomPeri || incomMem )
    {
        flags |= DMA_CONFIG_INCOMPATIBLE;
    }
    
    /*
     * This is returned so this function can be called as: 
     * if( dma_create_target == DMA_CONFIG_OK ){ go ahead } 
     * or if( dma_create_target() ){ check for errors }
     */
    return flags; 
}

static inline uint8_t getMisalignment_b( uint8_t* p_ptr, 
                                        dma_data_type_t p_type )
{
    /*
     * Note: These checks only makes sense when the target is memory. This is 
     * not performed when the target is a peripheral (i.e. uses a semaphore). 
     * Check for word alignment:
     * The 2 LSBs of the data type must coincide with the 2 LSBs of the SRC 
     * pointer.
     * This guarantees word alignment. 
     * |____|____|____|____|____|____|____|____|    Memory address 0x*******y
     * 0    1    2    3    4    5    6    7     = y (In bytes)
     *  Byte words can start in any of these positions
     *  Half Words can only start on 0, 2, 4 or 6
     *  Words can only start on 0 or 4
     *  For example, if there was a Word starting on address ended in 2:
     * |____|____|\\\\|\\\\|\\\\|\\\\|____|____|
     * 0    1    2    3    4    5    6    7    
     * The DMA could only grab bytes 0-3 and 4-7, so it CANNOT copy into the 
     * destination pointer (x) the desired Word as follows:
     * |\\\\|\\\\|\\\\|\\\\|
     * x   x+1  x+2  x+3   
     * 
     * To overcome this, the ALLOW REALIGN flag is available in the DMA control
     * block. 
     * If the user set the ALLOW REALIGN flag, WORD reading from misaligned 
     * pointers will be converted to two HALF WORD readings with a HALF WORD
     * increment on both source and destination.  
     * 
     * HALF WORD misalignment is solved through the same method using two WORD
     * readings. 
     * 
     */  
    
    /* 
     * If the data type is WORD and the two LSBs of pointer are not 00,
     * there is a misalignment.
     */
    uint8_t isWord = ( p_type == DMA_DATA_TYPE_WORD );
    uint8_t notWordAligned = ( (uint32_t)p_ptr & DMA_WORD_ALIGN_MASK );
    uint8_t misalignment = isWord && notWordAligned;
    /* 
     * If the data type is WORD or HALF WORD and the LSB of pointer 
     * is not 0, there is a misalignment.
     * The inequality is of special importance because WORDs stored in odd
     * pointers need to turn into BYTE as well.
     */
    uint8_t wordOrHalfWord = ( p_type <= DMA_DATA_TYPE_HALF_WORD );
    uint8_t notHalfWordAligned = ( (uint32_t)p_ptr & DMA_HALF_WORD_ALIGN_MASK );
    misalignment += ( wordOrHalfWord && notHalfWordAligned );
    /* 
     * These two lines will end up with: 
     * misalignment == 0 if no realignment is needed.
     * misalignment == 1 if realignment is needed, but switching to half 
     * the word size would fix it
     * misalignment == 2 if a WORD is to be read from an odd pointer, so 
     * BYTE data type is needed necessarily. 
     */
    return misalignment;
}

static inline uint8_t isOutbound( uint8_t* p_start, 
                                uint8_t* p_end, 
                                uint32_t p_type, 
                                uint32_t p_size_du, 
                                uint32_t p_inc_du )
{
  /* 000 = A data unit to be copied
   * xxx = A data unit to be skipped
   * 
   * v The start               /------------\ The size of each increment
   * |OOOO|xxxx|xxxx|OOOO|xxxx|xxxx| . . . |OOOO|xxxx|xxxx|OOOO|xxxx|xxxx| 
   *  \--/ The size of a type    
   *  \------------------- Each increment n-1 times ------/
   *                              + 1 word (w/o increment) \--/  
   *  \------ All the affected region (rangeSize) ------------/ 
   *                                   The last affected byte ^ 
   * 
   * If the environment ends before the last affected byte, then there is
   * outbound writing and the function returns 1.  
   */
    uint32_t affectedUnits = ( p_size_du - 1 )*p_inc_du + 1;
    uint32_t rangeSize = DMA_DATA_TYPE_2_DATA_SIZE(p_type) * affectedUnits;
    uint32_t lasByteInsideRange = p_start + rangeSize -1;
    return ( p_end < lasByteInsideRange );
    // Size is be guaranteed to be non-zero before calling this function.  
}

// @ToDo: Consider changing the "mask" parameter for a bitfield definition (see dma_regs.h)
static inline void writeRegister( uint32_t p_val, 
                                uint32_t p_offset, 
                                uint32_t p_mask )
{
    uint8_t index = p_offset / DMA_REGISTER_SIZE_BYTES;
    uint32_t originalVal = (( uint32_t * ) dma_peri ) [ index ]; 
    uint32_t clearedVal = originalVal & ~p_mask; 
    uint32_t val = clearedVal | ( p_val & p_mask );
    (( uint32_t * ) dma_peri ) [ index ] = val;
}

static inline uint32_t getIncrement_b( dma_target_t * p_tgt )
{
    uint32_t inc_b = 0;
    /* If the target uses a semaphore, the increment remains 0. */
    if( !( p_tgt->smph ) ) 
    {   
        /*
         * If the transaction increment has been overwritten (due to 
         * misalignments), then that value is used (it's always 1, never 0).
         */
        if( ! (inc_b = dma_cb.trans->inc_b) )
        {
            /*
             * Otherwise, the target-specific increment is used 
             * (transformed into bytes).
             */
            uint8_t dataSize_b = DMA_DATA_TYPE_2_DATA_SIZE( dma_cb.trans->type );
            inc_b = ( p_tgt->inc_du * dataSize_b );
        }
    }
    return inc_b;
}

// @ToDo: Let fic be in charge of this. 
void handler_irq_fast_dma(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(&(dma_cb.fic), kDma_fic_e);
    // The flag is raised so the waiting loop can be broken.
    dma_cb.intrFlag = 1;
    // Call the weak implementation provided in this module, 
    // or the non-weak implementation from above. 
    dma_intr_handler();
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/

