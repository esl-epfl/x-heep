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

#define DMA_DATA_TYPE_2_DATA_SIZE(type) (0b00000100 >> (type) )     

/**
 * If any critical error was detected during this process, the launching is aborted.
 */
#define ABORT_IF_CRITICAL_ERROR() { if( dma_cb.launchResult & DMA_LAUNCH_CRITICAL_ERROR ) return dma_cb.launchResult} 


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

static inline uint32_t getChunkSize( uint32_t p_ptr );
static inline uint32_t getMemSize( uint32_t p_ptr );
static inline uint8_t getMisalignment( uint32_t p_ptr );
static inline void writeRegister( uint32_t p_val, ptrdiff_t p_ptr );

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
struct //anonymous
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
}dma_cb;

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/


void dma_init()
{
  // @ToDo: This should be deprecated. base address should be obtained from.....
  dma_cb.baseAdd = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);
  dma_reset();
  // juan: prob some more stuff should go here.
  // e.g. this function could return something
}


void dma_set_src( uint32_t * p_src ) 
{                                           
  make_sure_that( p_src < DMA_MEM_PTR_MAX );
  dma_cb.ctrl.PTR_IN = p_src; 
} // @ToDo: Rename DMA_PTR_IN_*  ->  DMA_SRC_PTR_* 


void dma_set_dst( uint32_t * p_dst )
{
  make_sure_that( p_dst < DMA_MEM_PTR_MAX );
  dma_cb.ctrl.PTR_OUT = p_dst;
} // @ToDo: Rename DMA_PTR_OUT_*  ->  DMA_DST_PTR_*


void dma_set_size( uint32_t * p_copySize_du)
{
  make_sure_that( p_copySize < DMA_MEM_SIZE_MAX );
  dma_cb.ctrl.DMA_START = p_copySize_du;
} // @ToDo: Rename DMA_DMA_START_*  ->  DMA_CNT_START_*
// juan q ruben: Does the count keep the amount left to transmit? So if cnt == 0, done == 1 ? 



void dma_set_src_ptr_inc( uint32_t p_inc_du )
{
    /*
     * Increment size must be passed as a multiple of the data_unit to prevent overlapping. 
     */
  make_sure_that( /*There is no possible check*/ );
  dma_cb.ctrl.SRC_PTR_INC = p_inc_du;
}


void dma_set_dst_ptr_inc( uint32_t p_inc_du )
{
    /*
     * Increment size must be passed as a multiple of the data_unit to prevent overlapping. 
     */
  make_sure_that( /*There is no possible check*/ );
  dma_cb.ctrl.DST_PTR_INC = p_inc_du;
}


void dma_set_direction( dma_dir_t p_dir)
{
  make_sure_that( p_dir < DMA_DIR__size );
  dma_cb.ctrl.SPI_MODE = p_dir;
}


void dma_set_data_type( dma_data_type_t p_type)
{
  make_sure_that( p_type < DMA_DATA_TYPE__size );
  dma_cb.ctrl.DATA_TYPE = p_type;
} 


void dma_set_end_event( dma_end_event_t p_event )
{
    make_sure_that( p_event < DMA_END_EVENT__size );
    dma_cb.endEvent = p_event;
}


uint32_t dma_get_cnt_du()
{
    uint32_t ret = mmio_region_read32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DMA_START_REG_OFFSET));
    make_sure_that( /* juan q ruben: what could be asserted in this case? */ );
    return ret *= DMA_DATA_TYPE_2_DATA_SIZE( dma_cb.ctrl.DATA_TYPE );
}


uint32_t dma_is_done()
{
  uint32_t ret = mmio_region_read32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DONE_REG_OFFSET));
  make_sure_that( ret == 0 || ret == 1 );
  return ret;
}   // juan q jose: What to do in the above case
  /* In case a return wants to be forced in case of an error, there are 2 alternatives: 
   *    1) Consider any value != 0 to be a valid 1 using a LOGIC AND: 
   *            return ( 1 && mmio_region_read32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DONE_REG_OFFSET)));
   *    2) Consider only the LSB == 1 to be a valid 1 using a BITWISE AND. 
   *            return ( 1 &  mmio_region_read32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DONE_REG_OFFSET)));
   * */   
    // juan q ruben: if cnt== 0 => done == 1 ?? For how long? 
    // @ToDo: Rename DONE_*  ->  IDLE_*
// @ToDo: Make register DONE a 1 bit field in hw/ip/dma/data/dma.hjson. Watch out for compatibility/sync with hardware.  


dma_launch_ret_t dma_launch( dma_safety_level_t p_safetyLevel, dma_allow_realign_t p_allowRealign ) 
{
    
    // juan q ruben: This might be a good moment to evaluate whether it is really worth it to use the DMA. Consider all these checks that have to be done!! 
    // Another alternative (non exclusive) is to allow the user to bypass any checks. 
    // This can be useful when you have hand-picked your parameters, so you are sure there will not be a problem. 
    // You can save a lot of valuable processing time. 
    // We could even take all the asserts to this point so they can also be by-passed. 
    
    
    //////////  PERFORM SANITY CHECKS   //////////
    if( p_safetyLevel & DMA_SAFETY_SANITY_CHECKS ){
    
        /*
         *  Asserts would go here
         */
    }
    
    
    //////////  SET COMPULSORY VALUES   //////////
   switch( dma_cb.ctrl.SPI_MODE )
   {
       case DMA_DIR_SPI_RX: // DMA will receive information from SPI
           dma_cb.ctrl.PTR_IN = 1; // juan: find the SPI rx buffer pointer.
           dma_cb.endEvent = DMA_END_EVENT_SPI;
           break;

       case DMA_DIR_SPI_TX:
           dma_cb.ctrl.PTR_OUT = 2; // juan: find the SPI tx buffer pointer
           // The user can choose whether to wait for the SPI to finish or 
           break;

       case DMA_DIR_SPI_FLASH_RX:
           dma_cb.ctrl.PTR_IN = 3; // juan: find the SPI FLASH rx buffer pointer
           dma_cb.endEvent = DMA_END_EVENT_SPI;
           break;

       case DMA_DIR_SPI_FLASH_TX:
           dma_cb.ctrl.PTR_OUT = 4; // juan: find the SPI tx buffer pointer
           break;

       case DMA_DIR_M2M:
           // Nothing to be done
           break;

       default:
           make_sure_that(0); // This should never happen
           break;
   }
    
    //////////  PERFORM INTEGRITY CHECKS CHECKS   //////////
    if( p_safetyLevel & DMA_SAFETY_INTEGRITY_CHECKS ){

    
    // juan: This might overwrite some configs added by the user. Warn them beforehand in the function docu
    
    
        //////////  CHECK IF THERE ARE MISALIGNMENTS   //////////
        
        /* First the source arrangement is analyzed*/
        uint8_t misalignment =  getMisalignment(  dma_cb.ctrl.SRC_PTR_INC );
        dma_cb.launchResult |= ( misalignment ? DMA_LAUNCH_SRC : 0 ); 

        /* The destination arrangement is analyzed. */
        uint8_t dstMisalignment = getMisalignment( dma_cb.ctrl.DST_PTR_INC );
        dma_cb.launchResult |= ( dstMisalignment ? DMA_LAUNCH_DST : 0 );
      
        /* Only the largest misalignment is preserved.*/
        misalignment = misalignment > dstMisalignment ? misalignment : dstMisalignment; /* If a misalignment was detected during this process, it will be attributed to the destination arrangement.*/  

        if( misalignment )
        {
            dma_cb.launchResult |= DMA_LAUNCH_MISALIGN; // The misalignment flag is raised. 

            /* If a misalignment is detected and realignment is not allowed, an error is returned. No operation should be performed by the DMA */
            if( !p_allowRealign) dma_cb.launchResult |= DMA_LAUNCH_CRITICAL_ERROR;

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
            if( dma_cb.ctrl.SRC_PTR_INC > dma_cb.ctrl.DATA_TYPE ) || ( dma_cb.ctrl.DST_PTR_INC > dma_cb.ctrl.DATA_TYPE ) ) dma_cb.launchResult |= ( DMA_LAUNCH_DISCONTINUOUS | DMA_LAUNCH_CRITICAL_ERROR );

            //////////  CHECK IF THERE WAS A CRITICAL ERROR   //////////
            ABORT_IF_CRITICAL_ERROR(); // No further operations are done to prevent corrupting information that could be useful for debugging purposes. 
        
            //////////  PERFORM THE REALIGNMENT  //////////
            /* Otherwise, a more granular data type is used according to the detected misalignment in order to overcome it. */       
            dma_cb.ctrl.DATA_TYPE += misalignment;
            /* Source and destination increment should now be of the size of the data.
             * As increments are given in data units, in both cases should be 1. */
            dma_cb.ctrl.SRC_PTR_INC = 1;       
            dma_cb.ctrl.DST_PTR_INC = 1;      
        }
        
        
        //////////  CHECK IF THERE ARE MEMORY OVERFLOWS  //////////
        switch( dma_cb.ctrl.SPI_MODE )
        {
            /* If the source of the DMA transmission is the memory, it should be checked that the DMA will not be reading beyond the end of the memory.   */
            case DMA_DIR_M2M:
            case DMA_DIR_SPI_TX:
            case DMA_DIR_SPI_FLASH_TX:
                /* 
                 * This is done by:
                 *  1) Finding the size of the chunk of memory to be gone over in order to cover all the data units. 
                 *      This is ( the number of data units to be copied, in data units) times ( the size of each increment, which includes the copied data unit and the blank spaces, in data units) times ( the conversion rate between data units and bytes ) 
                 *  2) Finding the size of the available chunk of memory
                 *      This is ( the total size of the memory ) minus ( the relative position of the source pointer with regard to the beginning of the memory)
                 *  3) The number obtained in 2) must be larger than that of 1).   
                 */
                if( getChunkSize( dma_cb.ctrl.SRC_PTR_INC ) > getmemSize( dma_cb.ctrl.PTR_IN ) ) dma_cb.launchResult |= ( DMA_LAUNCH_OVERFLOW | DMA_LAUNCH_SRC | DMA_LAUNCH_CRITICAL_ERROR ); 
                break;
            
            /* If the destination of the DMA transmission is the memory, it should be checked that the DMA will not be writing beyond the end of the memory.   */
            case DMA_DIR_SPI_RX:
            case DMA_DIR_SPI_FLASH_RX:
                /* The process in analogous to the previous cases. */
                if( getChunkSize( dma_cb.ctrl.DST_PTR_INC ) > getmemSize( dma_cb.ctrl.PTR_OUT ) ) dma_cb.launchResult |= ( DMA_LAUNCH_OVERFLOW | DMA_LAUNCH_DST | DMA_LAUNCH_CRITICAL_ERROR ); 
                break;
                
            default:
                make_sure_that(0); // This should never happen
        }
        //////////  CHECK IF THERE WAS A CRITICAL ERROR   //////////
        ABORT_IF_CRITICAL_ERROR();
    }
   
   //////////  WRITE THE REGISTER     //////////
   
   /**
    * Each register write is performed in three steps:
    *   1) Saving the value to be written in a dummy variable val
    *   2) Actually writing the value
    *   3) Asserting that the value read from the register is the value stored in val. 
    */
    
   /* Dummy value to be used ro write and re-read*/
    uint32_t val;
    
    
   //////////  SET THE POINTERS   //////////
    writeRegister( dma_cb.ctrl.PTR_IN, DMA_PTR_IN_REG_OFFSET );
    writeRegister( dma_cb.ctrl.PTR_OUT, DMA_PTR_OUT_REG_OFFSET );
    
    //////////  SET THE INCREMENTS   //////////    
    writeRegister( dma_cb.ctrl.SRC_PTR_INC * DMA_DATA_TYPE_2_DATA_SIZE(dma_cb.ctrl.DATA_TYPE), DMA_SRC_PTR_INC_REG_OFFSET ); // First converts pointer increments to bytes. The register of the DMA needs number of bytes. 
    writeRegister( dma_cb.ctrl.DST_PTR_INC * DMA_DATA_TYPE_2_DATA_SIZE(dma_cb.ctrl.DATA_TYPE), DMA_DST_PTR_INC_REG_OFFSET );
    
            
    //////////  SET DIRECTION AND DATA TYPE   //////////    
    writeRegister( dma_cb.ctrl.SPI_MODE, DMA_SPI_MODE_REG_OFFSET );
    writeRegister( dma_cb.ctrl.DATA_TYPE, DMA_DATA_TYPE_REG_OFFSET );
    
    //////////  SET SIZE TO COPY + LAUNCH THE DMA OPERATION   //////////
    writeRegister( dma_cb.ctrl.DMA_START * DMA_DATA_TYPE_2_DATA_SIZE( dma_cb.ctrl.DATA_TYPE ), DMA_DMA_START_REG_OFFSET ); // First converts the copy size to bytes so it can be written into the DMA register.
    
    return dma_cb.launchResult;
}



void dma_reset()
{

// juan: consider some default values!! What if the user just forgot to set something up?
}

uint32_t dma_abort()
{
    // juan q ruben: let the user know how many data units where left. But watch out... may prefer not to communicate with the dma. 
}


/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Gets the size of the affected memory during a read/write operation of the DMA. 
 * @param p_ptr Source or Destination pointer, to a valid memory region.
 * @return The size of the affected memory, in bytes.
 */
static inline uint32_t getChunkSize( uint32_t p_ptr )
{
    return dma_cb.ctrl.DMA_START * p_ptr * DMA_DATA_TYPE_2_DATA_SIZE(dma_cb.ctrl.DATA_TYPE);
}

/**
 * @brief Gets the size of the available memory during a read/write operation of the DMA. 
 * @param p_ptr Source or Destination pointer, to a valid memory region.
 * @return The size of the available memory, in bytes.
 */
static inline uint32_t getMemSize( uint32_t p_ptr )
{
    // juan q jose fill this information... from where?  
    return DMA_MEMORY_SIZE_B - ( p_ptr - DMA_MEMORY_INIT);
}

/**
 * @brief Gets how misaligned a pointer is, taking into account the data type size. 
 * @param p_ptr The source or destination pointer. 
 * @return How misaligned the pointer is, in bytes. 
 */
static inline uint8_t getMisalignment( uint32_t p_ptr )
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
    uint8_t misalginment = ( dma_cb.ctrl.DATA_TYPE == DMA_DATA_TYPE_WORD ) && ( dma_cb.ctrl.DST_PTR_INC & 3 );
    /* 
    * If WORD or HALF WORD and the LSB of pointer is not 0 there is a misalignment.
    * The inequality is of special importance because WORDs stored in odd pointers need to turn into BYTE as well.
    */
    misalginment += ( ( dma_cb.ctrl.DATA_TYPE <= DMA_DATA_TYPE_HALF_WORD ) && ( dma_cb.ctrl.DST_PTR_INC & 1 ) );
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
static inline void writeRegister( uint32_t p_val, ptrdiff_t p_ptr )
{
    mmio_region_write32(dma_cb.baseAdd, p_ptr, p_val );
    make_sure_that( p_val == mmio_region_read32( dma_cb.baseAdd, (ptrdiff_t)(p_ptr) ) );
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/

// juan: Agregar las condiciones de fallo en la docu de las funciones de set. 

// juan : cubrirme que si el origen/destino es SPI no hay que hacer chequeos de alneacion.