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
* @brief  The Direct Memory Access (DMA) driver to set up and use the DMA peripheral
*
* 
* 
*/


/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "dma.h"

// To manage interrupts
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

// ToDo: Juan - remove this, is just a placeholder until real assert can be included
#define make_sure_that(x) printf( "%s@%u\n\r",x ? "Success" : "Error",__LINE__ );

#define DMA_CSR_REG_MIE_MASK(enable) (enable << 19)

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
 * @brief Gets how misaligned a pointer is, taking into account the data type size. 
 * @param p_ptr The source or destination pointer. 
 * @return How misaligned the pointer is, in bytes. 
 */
static inline uint8_t getMisalignment_b( uint8_t* p_ptr, dma_data_type_t p_type );


/**
 * @brief Determines whether a given region will fit before the end of an environment.
 * @param p_start Pointer to the beginning of the region.
 * @param p_end Pointer to the last byte of the environment.
 * @param p_type The data type to be transferred.
 * @param p_size_du The number of data units to be transferred.
 * @param p_inc_du The size in data units of each increment. 
 * @retval 1 There is an outbound.
 * @retval 0 There is NOT an outbound.   
 */
static inline uint8_t isOutbound( uint8_t* p_start, uint8_t* p_end, uint32_t p_type, uint32_t p_size_du, uint32_t p_inc_du );

/**
 * @brief Writes a given value into the specified register. Later reads the register and checks that the read value is equal to the written one. 
 *          This check is done through an assertion, so can be disabled by disabling assertions.
 * @param p_val The value to be written.
 * @param p_offset The memory offset from the memory's base address where the target register is located. // juan: double check this claim
 */
static inline void writeRegister( uint32_t p_val, uint32_t p_offset );


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
    * The base address for the soc_ctrl hardware registers.
   */
  mmio_region_t baseAdd; 
  
 /*
  * Pointer to the transaction to be performed. 
  */
  dma_trans_t* trans;
  
  /*
   * Flag to lower as soon as a transaction is launched, and raised by the interrupt handler once it has finished. 
   */
  uint8_t intrFlag;

  /*
   * Fast interrupt controller control block
   */
  fast_intr_ctrl_t fic;
}dma_cb;

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/


// juan: make sure that after the dma_init you could launch a transcation and nothing would happen. Make all registers be loaded with null values.

void dma_init()
{
    /* Dummy empty targets and transactions are created clear the DMA.*/
    static dma_env_t defaultEnv;
    static dma_target_t defaultTargetA, defaultTargetB;
    static dma_trans_t defaultTrans;
    
    // @ToDo: juan: This should be deprecated. base address should be obtained from.....
    dma_cb.baseAdd = mmio_region_from_addr((uint8_t*)DMA_START_ADDRESS);
    dma_cb.fic.base_addr = mmio_region_from_addr((uintptr_t)FAST_INTR_CTRL_START_ADDRESS);
    
    // The dummy/null variables are loaded to the DMA.     
    
    // Juan: re-think these configurations! 
    
    dma_create_environment( &defaultEnv, (uint8_t*) NULL, (uint8_t*) NULL );
    dma_create_target( &defaultTargetA, (uint8_t*) NULL, 1, 0, DMA_DATA_TYPE_BYTE, DMA_SMPH_MEMORY, &defaultEnv, DMA_PERFORM_CHECKS_ONLY_SANITY );
    dma_create_target( &defaultTargetB, (uint8_t*) NULL, 1, 0, DMA_DATA_TYPE_BYTE, DMA_SMPH_MEMORY, &defaultEnv, DMA_PERFORM_CHECKS_ONLY_SANITY );
    dma_create_transaction( &defaultTrans, &defaultTargetA, &defaultTargetB, DMA_END_EVENT_INTR_WAIT, DMA_ALLOW_REALIGN, DMA_PERFORM_CHECKS_ONLY_SANITY); 
    dma_load_transaction( &defaultTrans );
    
}


dma_config_flags_t dma_create_environment( dma_env_t *p_env, uint8_t* p_start, uint8_t* p_end )
{
    /* PERFORM SANITY CHECKS */
    if( (uint8_t*)p_end < (uint8_t*)p_start ) return DMA_CONFIG_INCOMPATIBLE;
    
    // Load the start and end pointers of the enviroment.
    p_env->start = (uint8_t*)p_start;
    p_env->end   = (uint8_t*)p_end;
    
    return DMA_CONFIG_OK;
}


dma_config_flags_t dma_create_target( dma_target_t *p_tgt, uint8_t* p_ptr, uint32_t p_inc_du, uint32_t p_size_du, dma_data_type_t p_type, dma_semaphore_t p_smph, dma_env_t* p_env, dma_perform_checks_t p_check )
{
    //////////  SANITY CHECKS   //////////
    make_sure_that( (dma_data_type_t)p_type < DMA_DATA_TYPE__size );
    make_sure_that( (uint32_t)p_inc_du >= 0 ); // Increment can be 0 when a sempahore is used. 
    make_sure_that( (uint32_t)p_size_du >=  0 );
    make_sure_that( (dma_semaphore_t) p_smph < DMA_SMPH__size );
    
    
    //////////  STORING OF THE INFORMATION   //////////
    p_tgt->flags = DMA_CONFIG_OK; //The flags are cleaned in case the structure was used before.
    p_tgt->ptr = (dma_target_t *)p_ptr;
    p_tgt->inc_du = (uint32_t)p_inc_du;
    p_tgt->size_du = (uint32_t)p_size_du;
    p_tgt->type = (dma_data_type_t)p_type; 
    p_tgt->smph = (dma_semaphore_t)p_smph;
    p_tgt->env = (dma_env_t*)p_env;
    
    //////////  INTEGRITY CHECKS   //////////
    if( p_check )
    {
        if( ( p_tgt->env ) && ( /* Only performed if an environment was set. */
               ( (uint8_t*)(p_tgt->ptr) <  (uint8_t*)(p_tgt->env->start) )  // If the target starts before the environment starts.
            || ( (uint8_t*)(p_tgt->ptr) > (uint8_t*)(p_tgt->env->end) )     // If the target starts after the environment ends 
            || isOutbound( p_tgt->ptr, p_tgt->env->end, p_tgt->type, p_tgt->size_du, p_tgt->inc_du ) // If the target selected size goes beyond the boundaries of the environment.   
                ) ) p_tgt->flags |= DMA_CONFIG_OUTBOUNDS;

        /* If there is a semaphore, there should not be environments nor increments.
         * Otherwise, an increment is needed.*/ 
        if( ( p_tgt->smph && ( p_tgt->env || p_tgt->inc_du ) )
            ||  ( ! p_tgt->smph &&  ! p_tgt->inc_du   ) ) p_tgt->flags |= DMA_CONFIG_INCOMPATIBLE;
    }
    
    return p_tgt->flags; // This is returned so this function can be called as: if( dma_create_target == DMA_CONFIG_OK ){ go ahead } or if( dma_create_target() ){ check for errors } 
}


dma_config_flags_t dma_create_transaction( dma_trans_t *p_trans, dma_target_t *p_src, dma_target_t *p_dst, dma_end_event_t p_end, dma_allow_realign_t p_allowRealign, dma_perform_checks_t p_check )
{
    //////////  SANITY CHECKS    //////////
    make_sure_that( (dma_end_event_t)p_end < DMA_END_EVENT__size );
    
    
    //////////  CHECK IF TARGETS HAVE ERRORS    //////////
     /* 
     * The transaction is NOT created if the targets include errors.
     * A successful target creation has to be done before loading it to the DMA.
     */
    if( ( p_src->flags & DMA_CONFIG_CRITICAL_ERROR ) || ( p_dst->flags & DMA_CONFIG_CRITICAL_ERROR ) ) return DMA_CONFIG_CRITICAL_ERROR;
    
    //////////  CHECK IF THERE ARE SEMAPHORE INCONSISTENCIES   //////////
    /* 
     * The DMA can only handle one semaphore at a time, therefore, if the two targets require a semaphore, the transaction has to be discarded.
     * None of the two values can be taken by default because this inconsistency is probably a result of an error (likely wrong target selection).
     */
    if( p_check )
    {
        if( p_src->smph && p_dst->smph ) return ( DMA_CONFIG_INCOMPATIBLE | DMA_CONFIG_CRITICAL_ERROR );
        /* As there is only one non-null semaphore among the targets, that one is selected. 
         * If both are zero, it does not matter which is picked. */
    }
    p_trans->smph = p_src->smph ? p_src->smph : p_dst->smph;

    
    //////////  SET UP THE DEFAULT CONFIGURATIONS //////////
    p_trans->flags = DMA_CONFIG_OK; //The flags are cleaned in case the structure was used before.
    p_trans->size_b = p_src->size_du * DMA_DATA_TYPE_2_DATA_SIZE(p_src->type); // The copy size of the source (in data units -of the source-) is transformed to bytes, to be used as default size.
    p_trans->type = p_src->type; // By default, the source defines the data type. // ok? 
    p_trans->inc_b = 0; // By default, the transaction increment is set to 0 and, if required, it will be changed to 1 (in which case both src and dst will have an increment of 1 data unit)
    // The pointer to the targets is stored in the transaction
    p_trans->src = p_src;
    p_trans->dst = p_dst;
    p_trans->end = p_end;
    
    //////////  CHECK IF THERE ARE MISALIGNMENTS   //////////
    if( p_check  )
    {
        /* The source and destination targets are analyzed. 
         * If the target is a peripheral (i.e. uses one of semaphore slots)
         * then the misalignment is not checked.*/
        uint8_t misalignment =  p_src->smph ? 0 : getMisalignment_b( p_src->ptr, p_trans->type  );
        p_trans->flags |= ( misalignment ? DMA_CONFIG_SRC : 0 ); 

        uint8_t dstMisalignment = p_dst->smph ? 0 : getMisalignment_b( p_dst->ptr, p_trans->type  );
        p_trans->flags  |= ( dstMisalignment ? DMA_CONFIG_DST : 0 );

        /* Only the largest misalignment is preserved.*/
        misalignment = misalignment >= dstMisalignment ? misalignment : dstMisalignment; // If a misalignment was detected during this process, it will be attributed to the destination arrangement.

        if( misalignment )
        {
            /* Misalignment flags will only be stored in the transaction, as they 
            are data type dependent, and could vary from transaction to transaction,
            even with the same pair of targets.*/
            p_trans->flags |= DMA_CONFIG_MISALIGN;

            /* If a misalignment is detected and realignment is not allowed, an error is returned. No operation should be performed by the DMA */
            if( !p_allowRealign) return p_trans->flags |= DMA_CONFIG_CRITICAL_ERROR; // No further operations are done to prevent corrupting information that could be useful for debugging purposes. 

            //////////  CHECK IF THERE IS A DISCONTINUITY   //////////
            /* If there is a misalignment AND the source or destination arrangements are discontinuous, it is not possible to use the DMA. An error is returned.
             * A discontinuity in an arrangement is defined as the increment being larger than the data type size. 
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
             * In this case the source arrangement has 16bit HALF WORDs, with an increment of 1 data unit, so all data is continuous. 
             * The destination arrangement has also 16bit HALF WORDs, but misaligned and with 2 data units of increment. 
             * To copy a misaligned arrangement of HALF WORDs, the DMA should use BYTEs. However, by using bytes it would not be able to 
             * write twice and skip twice, as it has a single skip increment. 
             * 
             * The misalignment and discontinuity can be found in the source and destination respectively and vice versa, and this limitation would still exist.  
             * 
             * The discontinuous flag is added (the misaligned one was already there), and it is turned into a critical error.
             */
            if( ( p_src->inc_du > 1 ) || ( p_dst->inc_du > 1 ) ) return p_trans->flags |= ( DMA_CONFIG_DISCONTINUOUS | DMA_CONFIG_CRITICAL_ERROR ); // No further operations are done to prevent corrupting information that could be useful for debugging purposes. 


            //////////  PERFORM THE REALIGNMENT  //////////
            /* If realignment is allowed and there are no discontinuities, a more granular data type is used according to the detected misalignment in order to overcome it. */       
            p_trans->type += misalignment;
            /* Source and destination increment should now be of the size of the data.
             * As increments are given in bytes, in both cases should be the size of a data unit. */
            p_trans->inc_b = DMA_DATA_TYPE_2_DATA_SIZE( p_trans->type );
            /* The copy size does not change, as it is already stored in bytes*/
        }
    
        //////////  CHECK IF THERE IS CROSS-OUTBOUND   //////////
        /* As the source target does not know the constraints of the destination target, 
         * it is possible that the copied information ends up beyond the bounds of 
         * the destination environment. 
         * This check is not performed if no destination environment was set.
         *
         * e.g. 
         * If 10 HALF WORDs are to be transferred with a HALF WORD in between
         * (increment of 2 data units: writes 2 bytes, skips 2 bytes, ...),
         * then ( 10 data units * 2 bytes ) + ( 9 data units * 2 bytes ) are traveled for each = 38 bytes 
         * that need to be gone over in order to complete the transaction.
         * Having the transaction size in bytes, they need to be converted to data units:
         * size [data units] = size [bytes] / convertionRatio [bytes / data unit].
         * 
         * The transaction can be performed if the whole affected area can fit inside the destination environment
         * (i.e. The start pointer + the 38 bytes -in this case-, is smaller than the end pointer of the environment).   
         */ 
        if( ( p_dst->env ) && isOutbound( p_dst->ptr, p_dst->env->end, p_trans->type, p_trans->src->size_du, p_trans->dst->inc_du ) ) return p_trans->flags |= ( DMA_CONFIG_DST | DMA_CONFIG_OUTBOUNDS | DMA_CONFIG_CRITICAL_ERROR ); // No further operations are done to prevent corrupting information that could be useful for debugging purposes. 
        
        // @ToDo: It should also be checked that the source and destination are not the same region.. or at least that the destination is behind the source (to be used to shift the position of a buffer). 
        // @ToDo: Consider if (when a destination target has no environment) the destination size should be used as limit. 
    }   
        
    return p_trans->flags;
}


dma_config_flags_t dma_load_transaction( dma_trans_t* p_trans )
{
    
    /* 
     * The transaction is not allowed if it contain a critical error.
     * A successful transaction creation has to be done before loading it to the DMA.
     */
    if( p_trans->flags & DMA_CONFIG_CRITICAL_ERROR )
    {   
        dma_cb.trans = NULL;
        return DMA_CONFIG_CRITICAL_ERROR;
    }
    
    dma_cb.trans = p_trans; // Save the current transaction
    
        //////////  ENABLE/DISABLE INTERRUPTS    //////////    
    /* 
     * If the selected en event is polling, interrupts are disabled. 
     * Otherwise the mie.MEIE bit is set to one to enable machine-level fast dma interrupt.
     */
    CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK( dma_cb.trans->end != DMA_END_EVENT_POLLING ) );
    
    
    //////////  SET THE POINTERS   //////////
    writeRegister( dma_cb.trans->src->ptr, DMA_PTR_IN_REG_OFFSET );
    writeRegister( dma_cb.trans->dst->ptr, DMA_PTR_OUT_REG_OFFSET );
    
    //////////  SET THE INCREMENTS   //////////    
    
    /* The increments might have been changed (vs. the original value of the target) due to misalignment issues. If they have, use the changed values, otherwise, use the target-specific ones.
      Other reason to overwrite the target increment is if a semaphore is used. In that case, a increment of 0 is necessary. */
    writeRegister( getIncrement_b( dma_cb.trans->src ), DMA_SRC_PTR_INC_REG_OFFSET );  
    writeRegister( getIncrement_b( dma_cb.trans->dst ), DMA_DST_PTR_INC_REG_OFFSET );
    
            
    //////////  SET SEMAPHORE AND DATA TYPE   //////////    
    writeRegister( dma_cb.trans->smph, DMA_SPI_MODE_REG_OFFSET );
    writeRegister( dma_cb.trans->type, DMA_DATA_TYPE_REG_OFFSET );
    
       
    return DMA_CONFIG_OK;
}

dma_config_flags_t dma_launch( dma_trans_t* p_trans )
{
    if( !p_trans || ( dma_cb.trans != p_trans ) ) return DMA_CONFIG_CRITICAL_ERROR; // Make sure that the loaded transaction is the intended transaction. If the loaded trans was NULL'd, then this the transaction is never launched.
    //////////  SET SIZE TO COPY + LAUNCH THE DMA OPERATION   //////////
    dma_cb.intrFlag = 0; // This has to be done prior to writing the register because otherwise the interrupt could arrive before it is lowered (i.e. causing  
    writeRegister(dma_cb.trans->size_b, DMA_DMA_START_REG_OFFSET ); 
    // If the end event was set to wait for the interrupt, the dma_launch will not return until the interrupt arrives. 
    while( p_trans->end == DMA_END_EVENT_INTR_WAIT && ! dma_cb.intrFlag ) {
        wait_for_interrupt();
    }
    return DMA_CONFIG_OK;
}


uint32_t dma_is_done()
{
  uint32_t ret = mmio_region_read32(dma_cb.baseAdd, (uint32_t)(DMA_DONE_REG_OFFSET));
  make_sure_that( ret == 0 || ret == 1 );
  return ret;
}   // juan q jose: What to do in the above case
  /* In case a return wants to be forced in case of an error, there are 2 alternatives: 
   *    1) Consider any value != 0 to be a valid 1 using a LOGIC AND: 
   *            return ( 1 && mmio_region_read32(dma_cb.baseAdd, (uint8_t*)(DMA_DONE_REG_OFFSET)));
   *    2) Consider only the LSB == 1 to be a valid 1 using a BITWISE AND. 
   *            return ( 1 &  mmio_region_read32(dma_cb.baseAdd, (uint8_t*)(DMA_DONE_REG_OFFSET)));
   * */   


__attribute__((weak)) void dma_intr_handler()
{
    printf("Weak implementation: The DMA has finished!\n");
}


/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/


static inline uint8_t getMisalignment_b( uint8_t* p_ptr, dma_data_type_t p_type )
{
    /*
    * Note: These checks only makes sense when the target is memory. This is not performed when the target is a peripheral (i.e. uses a semaphore). 
    * Check for word alignment:
    * The 2 LSBs of the data type must coincide with the 2 LSBs of the SRC pointer.
    * This guarantees word alignment. 
    * |____|____|____|____|____|____|____|____|    Memory address 0x*******y
    * 0    1    2    3    4    5    6    7     = y (In bytes)
    *  Byte words can start in any of these positions
    *  Half Words can only start on 0, 2, 4 or 6
    *  Words can only start on 0 or 4
    *  For example, if there was a Word starting on address ended in 2:
    * |____|____|\\\\|\\\\|\\\\|\\\\|____|____|
    * 0    1    2    3    4    5    6    7    
    * The DMA could only grab bytes 0-3 and 4-7, so it CANNOT copy into the destination pointer (x) the desired Word as follows:
    * |\\\\|\\\\|\\\\|\\\\|
    * x   x+1  x+2  x+3   
    * 
    * To overcome this, the ALLOW REALIGN flag is available in the DMA control block. 
    * If the user set the ALLOW REALIGN flag, WORD reading from mis-aligned pointers will be converted to two HALF WORD readings with a HALF WORD
    * increment on both source and destination.  
    * 
    * HALF WORD misalignment is solved through the same method using two WORD readings. 
    * 
    */  
    
   /* If WORD and the two LSBs of pointer are not 00 there is a misalignment.*/
    uint8_t misalginment = ( p_type == DMA_DATA_TYPE_WORD ) && ( (uint32_t)p_ptr & 3 );
    /* 
    * If WORD or HALF WORD and the LSB of pointer is not 0 there is a misalignment.
    * The inequality is of special importance because WORDs stored in odd pointers need to turn into BYTE as well.
    */
    misalginment += ( ( p_type <= DMA_DATA_TYPE_HALF_WORD ) && ( (uint32_t)p_ptr & 1 ) );
    /* 
     * These two lines will end up with: 
     * misalignment == 0 if no realignment is needed.
     * misalignment == 1 if realignment is needed, but switching to half the word size would fix it
     * misalignment == 2 if a WORD is to be read from an odd pointer, so BYTE data type is needed instead. 
     */
    return misalginment;
}




static inline uint8_t isOutbound( uint8_t* p_start, uint8_t* p_end, uint32_t p_type, uint32_t p_size_du, uint32_t p_inc_du )
{
  /* 000 = A data unit to be copied
   * xxx = A data unit to be skipped
   * 
   * The start v              /------------\ The size of each increment
   *          |OOOO|xxxx|xxxx|OOOO|xxxx|xxxx| . . . |OOOO|xxxx|xxxx|OOOO|xxxx|xxxx| 
   *           \--/ The size of a type    
   *          \------------------- Each increment n-1 times ------/ \--/ + 1 word (no increment) 
   *          \--------------------------------------------------------/ All the affected region
   *                                                                   ^ The last affected byte
   * 
   *  If the environment ends before the last affected byte, then there is outbound writing and the function return 1.  
   *                 /------------ This is the address of the las byte inside the range.*/
    return ( p_end < p_start + ( DMA_DATA_TYPE_2_DATA_SIZE(p_type) * ( ( p_size_du - 1 )*p_inc_du + 1 ) )  -1 );
    // juan: what happens if size == 0 ??
}

static inline void writeRegister( uint32_t p_val, uint32_t p_offset )
{
    mmio_region_write32(dma_cb.baseAdd, p_offset, p_val ); // Writes to the register
    make_sure_that( p_val == mmio_region_read32( dma_cb.baseAdd, (uint32_t)(p_offset) ) ); // Checks that the written value was stored correctly
}


static inline uint32_t getIncrement_b( dma_target_t * p_tgt )
{
    uint32_t inc = 0;
    // juan: Is this check necessary? If a smph was set, the inc was forced to be 0.
    if( !( p_tgt->smph ) ) // If the target uses a semaphore, the increment remains 0.
    {
        if( ! (inc = dma_cb.trans->inc_b) ) // If the transaction increment has been overwritten (due to misalignments), then that value is used (it's always 1, never 0). 
        {
            inc = ( p_tgt->inc_du * DMA_DATA_TYPE_2_DATA_SIZE( dma_cb.trans->type ) ); // Otherwise, the target-specific increment is used (transformed into bytes).
        }
    }
    return inc;
}


// juan q jose: How should this function be declared?
void handler_irq_fast_dma(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(&(dma_cb.fic), kDma_fic_e);
    // The flag is raised so the waiting loop can be broken.
    dma_cb.intrFlag = 1;
    // Only perform call the interrupt handler if the transaction requested to end by polling. 
    // juan: This is only a temporary solution while there is no way of disabling interrupts. 
    if(dma_cb.trans->end != DMA_END_EVENT_POLLING ) dma_intr_handler();
}






/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/



// juan: how could one easily modify the target?? Maybe from the SDK.

 // juan: clear the error flags! Think about where 

// juan q jose: configure interrupts? Shouldnt that be done from an sdk level (including fic?). If should be here: End events

///**
// * Possible way of knowing a DMA transfer has finished. 
// * This also determines what the application will consider as "finished".
// * e.g. For SPI transmission, the application may consider the transfer has 
// * finished once the DMA has transferred all its data to the SPI buffer (in 
// * which case POLLING or INTERRUPT should be used), or might as well wait 
// * until the SPI has finished sending all the data in its buffer (in which 
// * case SPI should be used).
// */
//typedef enum
//{
//    DMA_END_EVENT_POLLING   = 0x00, // Application must query the DONE register flag with dma_is_done() to know when the transfer finished. DMA interrupts are disabled.
//    DMA_END_EVENT_DMA_INT   = 0x01, // The application will receive a DMA interrupt once the DMA transfer has finished (i.e. the DONE register flag is high).   
//    DMA_END_EVENT_PERIP_INT = 0x02, // The application will receive a peripheral interrupt once the peripheral process has finished. DMA interrupts will be disabled.
//    DMA_END_EVENT__size,
//    DMA_END_EVENT__undef,       // Default. DMA will not be used. 
//} dma_end_event_t; 



// juan q: Should we add permissions to environments?  Read/write/change? 

// juan q jose: In BINDI/Stack/HAL/drivers_nrf/uart/nrf_drv_uart.h functions defined in .h have the _STATIC_INLINE macro.... how is that ok????

// juan: right now i am imposing that every peripheral must have a semaphore. 

// juan q jose: can we make the assert include some variable as parameter so we can know the "wrong value" ?? 
