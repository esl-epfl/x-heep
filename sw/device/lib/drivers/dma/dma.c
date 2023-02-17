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
 * Returns the size in Bytes of a certain datatype, as a sizeof(type) would. 
 */
#define DMA_DATA_TYPE_2_DATA_SIZE(type) (0b00000100 >> (type) )     

/**
 * If any critical error was detected during this process, the configureing is aborted.
 */
#define ABORT_IF_CRITICAL_ERROR() { if( dma_cb.configureResult & DMA_CONFIGURATION_CRITICAL_ERROR ) return dma_cb.configureResult} 




#define DMA_TRANS_STACK_DEPTH   5

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

static inline uint8_t getMisalignment( uint32_t* p_ptr, dma_data_type_t p_type );
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
static struct //anonymous
{
 
  /**
    * The base address for the soc_ctrl hardware registers.
   */
  mmio_region_t baseAdd; 
  
 /*
  * Information of the transaction to be performed. 
  */
  dma_trans_t* trans;

}dma_cb;

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/


void dma_init()
{
    static dma_env_t defaultEnv;
    static dma_target_t defaultTargetA, defaultTargetB;
    static dma_trans_t defaultTrans;
    
    // @ToDo: This should be deprecated. base address should be obtained from.....
    dma_cb.baseAdd = mmio_region_from_addr((uint32_t*)DMA_START_ADDRESS);
    dma_reset();
    // juan: prob some more stuff should go here.
    // e.g. this function could return something



    dma_create_environment( &defaultEnv, (uint32_t*) NULL, (uint32_t*) NULL );
    dma_create_target( &defaultTargetA, (uint32_t*) NULL, 1, 0, DMA_DATA_TYPE_BYTE, DMA_TARGET_NO_SEMAPHORE, &defaultEnv );
    dma_create_target( &defaultTargetB, (uint32_t*) NULL, 1, 0, DMA_DATA_TYPE_BYTE, DMA_TARGET_NO_SEMAPHORE, &defaultEnv );
    dma_create_transaction( &defaultTrans, &defaultTargetA, &defaultTargetB, DMA_END_EVENT_POLLING, DMA_ALLOW_REALIGN); 
    dma_load_transaction( &defaultTrans );
}


/**
 * @brief Creates an environment where targets can be added. An environment is, 
 * for instance, a RAM block, or section of memory that can be used by the DMA. 
 * Properly defining an environment can prevent the DMA from accessing restricted
 * memory regions. 
 * Targets for peripherals do not need an environment.
 * @param p_env Target to the dma_env_t structure where information will be allocated. 
 * @param p_start Pointer to the first accessible address of the environment.   
 * @param p_end Pointer to the last accessible address of the environment.
 * @return 
 */
dma_config_flags_t dma_create_environment( dma_env_t *p_env, uint32_t* p_start, uint32_t* p_end /*, dma_env_characs_t p_characs, dma_end_event_t p_endEvents*/ )
{
    /* PERFORM SANITY CHECKS */
    make_sure_that( p_end >= p_start );
    make_sure_that( p_perm < DMA_PERM__size );
    //make_sure_that( p_endEvents < DMA_END_EVENT__size );
    
    p_env->start = p_start;
    p_env->end   = p_end;
    
    /* Only allow the start and end to be the same of the environment is a peripheral.*/
    //if( ( ! ( p_characs & DMA_ENVIRONMENT_CHARACS_PERIPHERAL ) ) && ( p_start == p_end ) ) return DMA_CONFIGURATION_OVERLAP;
    
    //p_env->characs = p_characs;
    //p_env->endEvents = p_events;
    
    return DMA_CONFIGURATION_OK;
}





/* Two alternatives: 
    1) dma_target_t create()
    2) dma_config create( target* ) 
 In both cases the target is stored statically in application level: GOOD - HAL does not need to know how many targets there will be. 
 In both cases the target is written by this function.
 */
dma_config_flags_t dma_create_target( dma_target_t *p_tgt, uint32_t* p_ptr, uint32_t p_inc_du, uint32_t p_size_du, dma_data_type_t p_type, uint8_t p_semaphore, dma_env_t* p_env )
{
    //////////  SANITY CHECKS   //////////
    make_sure_that( p_type < DMA_DATA_TYPE__size );
    make_sure_that( p_inc_du > 0 );
    
    //////////  STORING OF THE INFORMATION   //////////
    p_tgt->ptr = p_ptr;
    p_tgt->inc_du = p_inc_du;
    p_tgt->size_du = p_size_du;
    p_tgt->type = p_type; 
    p_tgt->env = p_env;
    p-tgt->semaphore = p_semaphore;
    
    //////////  INTEGRITY CHECKS   //////////
    if( ( p_tgt->ptr <  p_tgt->env->start )
        || ( p_tgt->ptr > p_tgt->env->end ) 
        || ( ( p_tgt->ptr + p_tgt->size_du * p_tgt->inc_du ) > ( p_tgt->env->end + 1 ) )
            p_tgt->flags |= DMA_CONFIGURATION_OUTBOUNDS;
    
    return p_tgt->flags; // This is returned so this function can be called as if( dma_create_target == DMA_CONFIGURATION_OK ){ GO AHEAD } or if( dma_create_target() ){ check for errors } 
}

// juan: how could one easily modify the target?? Maybe from the SDK. 








dma_config_flags_t dma_create_transaction( dma_trans_t *p_trans, dma_target_t *p_src, dma_target_t *p_dst, dma_end_event_t p_end, dma_allow_realign_t p_allowRealign )
{
 /* Check that the destination env can handle the copy*/

    
    //////////  SET UP THE DEFAULT CONFIGURATIONS //////////
    p_trans->size_b = p_src->size_du * DMA_DATA_TYPE_2_DATA_SIZE(p_src->type);
    p_trans->type = p_src->type; // juan : By default, the source defines the data type?
    p_trans->inc_du = 0; // By default, the transaction increment is set to 0 and, if needed can be changed to 1 (in which case both src and dst will have an increment of 1 data unit)
    
    
    p_trans->src = p_src;
    p_trans->dst = p_dst;
    
    p_trans->end = p_end;
    
    //////////  CHECK IF THERE ARE MISALIGNMENTS   //////////
    /* Misalignment flags will only be stored in the transaction, as they 
     are data type dependent, and could vary from transaction to transaction,
     even with the same pair of targets.*/
    
    /* First the source arrangement is analyzed*/
    uint8_t misalignment =  getMisalignment( p_src->ptr, p_trans->type  );
    p_trans->flags |= ( misalignment ? DMA_CONFIGURATION_SRC : 0 ); 

    /* The destination arrangement is analyzed. */
    uint8_t dstMisalignment = getMisalignment( p_dst->ptr, p_trans->type  );
    p_trans->flags  |= ( dstMisalignment ? DMA_CONFIGURATION_DST : 0 );

    /* Only the largest misalignment is preserved.*/
    misalignment = misalignment > dstMisalignment ? misalignment : dstMisalignment; /* If a misalignment was detected during this process, it will be attributed to the destination arrangement.*/  
    
    if( misalignment )
    {
        p_trans->flags |= DMA_CONFIGURATION_MISALIGN; // The misalignment flag is raised. 

        /* If a misalignment is detected and realignment is not allowed, an error is returned. No operation should be performed by the DMA */
        if( !p_allowRealign) return p_trans->flags |= DMA_CONFIGURATION_CRITICAL_ERROR; // No further operations are done to prevent corrupting information that could be useful for debugging purposes. 

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
         * write twice and skip twice, but it only has a skip increment. 
         * 
         * The misalignment and discontinuity can be found in different arrangements and this limitation would still exist.  
         * 
         * The discontinuous flag is added (the misaligned one was already there - reason for which this is not an inline function), and it is turned into a critical error.
         */
        if( p_src->inc_du > p_trans->type ) || ( p_dst->inc_du > p_trans->type ) ) return p_trans->flags |= ( DMA_CONFIGURATION_DISCONTINUOUS | DMA_CONFIGURATION_CRITICAL_ERROR ); // No further operations are done to prevent corrupting information that could be useful for debugging purposes. 


        //////////  PERFORM THE REALIGNMENT  //////////
        /* Otherwise, a more granular data type is used according to the detected misalignment in order to overcome it. */       
        p_trans->type += misalignment;
        /* Source and destination increment should now be of the size of the data.
         * As increments are given in data units, in both cases should be the size of a data unit. */
        p_trans->inc_b = DMA_DATA_TYPE_2_DATA_SIZE( p_trans->type );
        /* The copy size does not change, as it is stored in Bytes*/
        
        // juan: use this for increments
    }

    
    //////////  CHECK IF THERE IS CROSS OUTBOUND   //////////
    /* As the source target does not know the constraints of the destination target, 
     * it is possible that the copied information ends up beyond the bounds of 
     * the destination environment. 
     */
    if( ( p_dst->ptr + p_trans->size_b ) > p_dst->env->end +1 ) return p_trans->flags |= ( DMA_CONFIGURATION_DST | DMA_CONFIGURATION_OUTBOUNDS | DMA_CONFIGURATION_CRITICAL_ERROR ); // No further operations are done to prevent corrupting information that could be useful for debugging purposes. 
    
    
    return p_trans->flags;
}



void dma_load_transaction( dma_trans_t* p_trans )
{
    /**
    * Each register write is performed in two steps:
    *   1) Writing the value
    *   2) Asserting that the value read from the register is the desired one.
    */
    
    dma_cb.trans = p_trans; // Save the current transaction
    
   //////////  SET THE POINTERS   //////////
    writeRegister( dma_cb.trans->src->ptr, DMA_PTR_IN_REG_OFFSET );
    writeRegister( dma_cb.trans->dst->ptr, DMA_PTR_OUT_REG_OFFSET );
    
    //////////  SET THE INCREMENTS   //////////    
                    // The increment might have been changed due to misalignment issues. If it has, use the changed value, otherwise, use the target specific one. 
    writeRegister( dma_cb.trans->inc_b ? dma_cb.trans->inc_b : dma_cb.trans->src->inc_du * DMA_DATA_TYPE_2_DATA_SIZE( dma_cb.trans->type ), DMA_SRC_PTR_INC_REG_OFFSET ); // First converts pointer increments to bytes. The register of the DMA needs number of bytes. 
    writeRegister( dma_cb.trans->inc_b ? dma_cb.trans->inc_b : dma_cb.trans->dst->inc_du * DMA_DATA_TYPE_2_DATA_SIZE( dma_cb.trans->type ), DMA_DST_PTR_INC_REG_OFFSET );
    
            
    //////////  SET DIRECTION AND DATA TYPE   //////////    
    writeRegister(  /* juan ! define how this will be set to the mode.  */ , DMA_SPI_MODE_REG_OFFSET );
    writeRegister( dma_cb.trans->type, DMA_DATA_TYPE_REG_OFFSET );
    
    
    // juan: configure interrupts and that stuff. 
    
    
    
}


dma_config_flags_t dma_launch( dma_trans_t* p_trans )
{
    if( dma_cb.trans != p_trans) return DMA_CONFIGURATION_CRITICAL_ERROR; // Make sure that the loaded transaction is the intended transaction
    //////////  SET SIZE TO COPY + LAUNCH THE DMA OPERATION   //////////
    writeRegister(dma_cb.trans->size_b, DMA_DMA_START_REG_OFFSET ); 
    return DMA_CONFIGURATION_OK;
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

/**
 * @brief Gets how misaligned a pointer is, taking into account the data type size. 
 * @param p_ptr The source or destination pointer. 
 * @return How misaligned the pointer is, in bytes. 
 */
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

/**
 * @brief Writes a given value into the specified register. Later reads the register and checks that the read value is equal to the written one. 
 *          This check is done through an assertion, so can be disabled by disabling assertions.
 * @param p_val The value to be written.
 * @param p_ptr The memory offset from the memory's base address where the target register is located.
 */
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

// juan: Agregar las condiciones de fallo en la docu de las funciones de set. 

// juan : cubrirme que si el origen/destino es SPI no hay que hacer chequeos de alneacion.

// juan q jose: How do you want to manage the relationship between DMA and SPI? 