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

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

/**
 * Returns the size in bytes of a certain datatype, as a sizeof(type) would. 
 */
#define DMA_DATA_TYPE_2_DATA_SIZE(type) (0b00000100 >> (type) )     

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
static inline uint8_t getMisalignment( uint32_t* p_ptr, dma_data_type_t p_type );

/**
 * @brief Writes a given value into the specified register. Later reads the register and checks that the read value is equal to the written one. 
 *          This check is done through an assertion, so can be disabled by disabling assertions.
 * @param p_val The value to be written.
 * @param p_ptr The memory offset from the memory's base address where the target register is located. // juan: double check this claim
 */
static inline void writeRegister( uint32_t p_val, uint32_t* p_ptr );

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
    
    // @ToDo: This should be deprecated. base address should be obtained from.....
    dma_cb.baseAdd = mmio_region_from_addr((uint32_t*)DMA_START_ADDRESS);

    
    // The dummy/null variables are loaded to the DMA.     
    dma_create_environment( &defaultEnv, (uint32_t*) NULL, (uint32_t*) NULL );
    dma_create_target( &defaultTargetA, (uint32_t*) NULL, 1, 0, DMA_DATA_TYPE_BYTE, DMA_SMPH__undef, &defaultEnv );
    dma_create_target( &defaultTargetB, (uint32_t*) NULL, 1, 0, DMA_DATA_TYPE_BYTE, DMA_SMPH__undef, &defaultEnv );
    dma_create_transaction( &defaultTrans, &defaultTargetA, &defaultTargetB, DMA_END_EVENT_POLLING, DMA_ALLOW_REALIGN); 
    dma_load_transaction( &defaultTrans );
}


dma_config_flags_t dma_create_environment( dma_env_t *p_env, uint32_t* p_start, uint32_t* p_end )
{
    /* PERFORM SANITY CHECKS */
    make_sure_that( p_end >= p_start );
    make_sure_that( p_perm < DMA_PERM__size );
    //make_sure_that( p_endEvents < DMA_END_EVENT__size ); // juan
    
    // Load the start and end pointers of the enviroment.
    p_env->start = p_start;
    p_env->end   = p_end;
    
    return DMA_CONFIG_OK;
}


dma_config_flags_t dma_create_target( dma_target_t *p_tgt, uint32_t* p_ptr, uint32_t p_inc_du, uint32_t p_size_du, dma_data_type_t p_type, uint8_t p_smph, dma_env_t* p_env )
{
    //////////  SANITY CHECKS   //////////
    make_sure_that( p_type < DMA_DATA_TYPE__size );
    make_sure_that( p_inc_du > 0 );
    
    //////////  STORING OF THE INFORMATION   //////////
    p_tgt->ptr = p_ptr;
    p_tgt->inc_du = p_inc_du;
    p_tgt->size_du = p_size_du;
    p_tgt->type = p_type; 
    p_tgt->smph = p_smph;
    p_tgt->env = p_env;
    
    //////////  INTEGRITY CHECKS   //////////
    if( ( p_tgt->env ) && ( ( p_tgt->ptr <  p_tgt->env->start ) /* Only performed if an environment was set. */
        || ( p_tgt->ptr > p_tgt->env->end ) 
        || ( ( p_tgt->ptr + p_tgt->size_du * p_tgt->inc_du ) > ( p_tgt->env->end + 1 ) ) ) )
            p_tgt->flags |= DMA_CONFIG_OUTBOUNDS;
    
    /* If an invalid semaphore is selected, the target cannot be used.
     This is not asserted as a way to allow the __undef semaphore to be used as a disable flag.*/ //juan: not sure if this is the best way of doing so. 
    if( p_tgt->smph >= DMA_SMPH__size ) p_tgt->flags |= DMA_CONFIG_CRITICAL_ERROR;
    
    return p_tgt->flags; // This is returned so this function can be called as: if( dma_create_target == DMA_CONFIG_OK ){ go ahead } or if( dma_create_target() ){ check for errors } 
}


dma_config_flags_t dma_create_transaction( dma_trans_t *p_trans, dma_target_t *p_src, dma_target_t *p_dst, dma_end_event_t p_end, dma_allow_realign_t p_allowRealign )
{
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
    if( p_src->smph && p_dst->smph ) return ( DMA_CONFIG_INCOMPATIBLE | DMA_CONFIG_CRITICAL_ERROR );
    /* As there is only one non-null semaphore among the targets, that one is selected. 
     * If both are zero, it does not matter which is picked. */
    p_trans->smph = p_src->smph ? p_src->smph : p_dst->smph;

    
    //////////  SET UP THE DEFAULT CONFIGURATIONS //////////
    p_trans->size_b = p_src->size_du * DMA_DATA_TYPE_2_DATA_SIZE(p_src->type); // The copy size of the source (in data units -of the source-) is transformed to bytes, to be used as default size.
    p_trans->type = p_src->type; // By default, the source defines the data type. // ok? 
    p_trans->inc_du = 0; // By default, the transaction increment is set to 0 and, if required, it will be changed to 1 (in which case both src and dst will have an increment of 1 data unit)
    // The pointer to the targets is stored in the transaction
    p_trans->src = p_src;
    p_trans->dst = p_dst;
    // The end event is stored in the transaction
    p_trans->end = p_end;
    
    
    //////////  CHECK IF THERE ARE MISALIGNMENTS   //////////
    
    /* The source and destination targets are analyzed. 
     * If the target is a peripheral (i.e. uses one of semaphore slots)
     * then the misalignment is not checked.*/
    uint8_t misalignment =  p_src->smph ? 0 : getMisalignment( p_src->ptr, p_trans->type  );
    p_trans->flags |= ( misalignment ? DMA_CONFIG_SRC : 0 ); 

    uint8_t dstMisalignment = p_dst->smph ? 0 : getMisalignment( p_dst->ptr, p_trans->type  );
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
        if( p_src->inc_du > p_trans->type ) || ( p_dst->inc_du > p_trans->type ) ) return p_trans->flags |= ( DMA_CONFIG_DISCONTINUOUS | DMA_CONFIG_CRITICAL_ERROR ); // No further operations are done to prevent corrupting information that could be useful for debugging purposes. 


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
     * then 10 data units * 4 bytes traveled for each = 40 bytes that need to be 
     * gone over in order to complete the transaction.
     * Having the transaction size in bytes, they need to be converted to data units:
     * size [data units] = size [bytes] / convertionRatio [bytes / data unit].
     * 
     * Th transaction can be computed if the whole affected area can fit inside the destination environment
     * (i.e. The start pointer + the 40 bytes -in this case-, is smaller than the end pointer of the environment).   
     */ 
    if( ( p_dst->env ) && ( ( p_dst->ptr + p_trans->inc_b * ( p_trans->size_b / DMA_DATA_TYPE_2_DATA_SIZE( p_trans->type ) ) ) > ( p_dst->env->end + 1 ) )  ) return p_trans->flags |= ( DMA_CONFIG_DST | DMA_CONFIG_OUTBOUNDS | DMA_CONFIG_CRITICAL_ERROR ); // No further operations are done to prevent corrupting information that could be useful for debugging purposes. 
        
    return p_trans->flags;
}


dma_config_flags_t dma_load_transaction( dma_trans_t* p_trans )
{
    /* 
     * The transaction is not allowed if it contain a critical error.
     * A successful transaction creation has to be done before loading it to the DMA.
     */
    if( p_trans->flags & DMA_CONFIG_CRITICAL_ERROR ) return DMA_CONFIG_CRITICAL_ERROR;
    dma_cb.trans = p_trans; // Save the current transaction
    
    /**
    * Each register writing is performed in two steps:
    *   1) Writing the value
    *   2) Asserting that the value read from the register is the desired one.
    */

    //////////  SET THE POINTERS   //////////
    writeRegister( dma_cb.trans->src->ptr, DMA_PTR_IN_REG_OFFSET );
    writeRegister( dma_cb.trans->dst->ptr, DMA_PTR_OUT_REG_OFFSET );
    
    //////////  SET THE INCREMENTS   //////////    
                    // The increments might have been changed (!=0) due to misalignment issues. If they have, use the changed values, otherwise, use the target-specific ones. 
    writeRegister( dma_cb.trans->inc_b ? dma_cb.trans->inc_b : dma_cb.trans->src->inc_du * DMA_DATA_TYPE_2_DATA_SIZE( dma_cb.trans->type ), DMA_SRC_PTR_INC_REG_OFFSET ); // First converts pointer increments to bytes. The register of the DMA needs number of bytes. 
    writeRegister( dma_cb.trans->inc_b ? dma_cb.trans->inc_b : dma_cb.trans->dst->inc_du * DMA_DATA_TYPE_2_DATA_SIZE( dma_cb.trans->type ), DMA_DST_PTR_INC_REG_OFFSET );
    
            
    //////////  SET SEMAPHORE AND DATA TYPE   //////////    
    writeRegister( dma_cb.trans->smph , DMA_SPI_MODE_REG_OFFSET );
    writeRegister( dma_cb.trans->type, DMA_DATA_TYPE_REG_OFFSET );
    
    
    // juan: configure interrupts and that stuff. 
    
    
    return DMA_CONFIG_OK;
}

/**
 * @brief Launches the loaded transaction. 
 * @param p_trans A pointer to the desired transaction. This is only used to double check that the loaded transaction is the desired one. 
 *                  This check can be avoided by passing a NULL pointer.
 * @retval DMA_CONFIG_CRITICAL_ERROR if the passed pointer does not correspond itself with the loaded transaction (i.e. it is likely the transaction to be loaded is not the desired one).
 * @retval DMA_CONFIG_OK == 0 otherwise.
 */
dma_config_flags_t dma_launch( dma_trans_t* p_trans )
{
    if( p_trans && dma_cb.trans != p_trans) return DMA_CONFIG_CRITICAL_ERROR; // Make sure that the loaded transaction is the intended transaction
    //////////  SET SIZE TO COPY + LAUNCH THE DMA OPERATION   //////////
    writeRegister(dma_cb.trans->size_b, DMA_DMA_START_REG_OFFSET ); 
    return DMA_CONFIG_OK;
}


uint32_t dma_is_done()
{
  uint32_t ret = mmio_region_read32(dma_cb.baseAdd, (uint32_t*)(DMA_DONE_REG_OFFSET));
  make_sure_that( ret == 0 || ret == 1 );
  return ret;
}   // juan q jose: What to do in the above case
  /* In case a return wants to be forced in case of an error, there are 2 alternatives: 
   *    1) Consider any value != 0 to be a valid 1 using a LOGIC AND: 
   *            return ( 1 && mmio_region_read32(dma_cb.baseAdd, (uint32_t*)(DMA_DONE_REG_OFFSET)));
   *    2) Consider only the LSB == 1 to be a valid 1 using a BITWISE AND. 
   *            return ( 1 &  mmio_region_read32(dma_cb.baseAdd, (uint32_t*)(DMA_DONE_REG_OFFSET)));
   * */   
    // juan q ruben: if cnt== 0 => done == 1 ?? For how long? 
    // @ToDo: Rename DONE_*  ->  IDLE_*
// @ToDo: Make register DONE a 1 bit field in hw/ip/dma/data/dma.hjson. Watch out for compatibility/sync with hardware.  




/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/


static inline uint8_t getMisalignment( uint32_t* p_ptr, dma_data_type_t p_type )
{
    /*
    * Check for word alignment:
    * The 2 LSBs of the data type must coincide with the 2 LSBs of the SRC pointer // juan: only if the source is memory! 
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
    uint8_t misalginment = ( p_type == DMA_DATA_TYPE_WORD ) && ( p_ptr & 3 );
    /* 
    * If WORD or HALF WORD and the LSB of pointer is not 0 there is a misalignment.
    * The inequality is of special importance because WORDs stored in odd pointers need to turn into BYTE as well.
    */
    misalginment += ( ( p_typeE <= DMA_DATA_TYPE_HALF_WORD ) && (p_ptr & 1 ) );
    /* 
     * These two lines will end up with: 
     * misalignment == 0 if no realignment is needed.
     * misalignment == 1 if realignment is needed, but switching to half the word size would fix it
     * misalignment == 2 if a WORD is to be read from an odd pointer, so BYTE data type is needed instead. 
     */
    return misalginment;
}


static inline void writeRegister( uint32_t p_val, uint32_t* p_ptr )
{
    mmio_region_write32(dma_cb.baseAdd, p_ptr, p_val );
    make_sure_that( p_val == mmio_region_read32( dma_cb.baseAdd, (uint32_t*)(p_ptr) ) );
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/



// juan: how could one easily modify the target?? Maybe from the SDK.

 // juan: clear the error flags! Think about where 