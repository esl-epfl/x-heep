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

#define DMA_LAUNCH_RETURN(x)    {\
                                    dma_cb.launchResult = p_ret;\
                                    return dma_cb.launchResult;\
                                }

#define DMA_DATA_TYPE_2_DATA_SIZE(type) (0b00000100 >> (type) )     

/* If any critical error was detected during this process, the launching is aborted. */
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

dma_cb_t dma_cb;

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Does the initial set up of the DMA control block.
 */
void dma_init()
{
  // @ToDo: This should be deprecated. base address should be obtained from.....
  dma_cb.baseAdd = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);
  // juan: prob some more stuff should go here.
  // e.g. this function could return something
}


/**
 * 
 * @brief Write to the read (source) pointer register of the DMA.
 * @param p_src Any valid memory address.
 */
void dma_set_src( uint32_t * p_src ) 
{                                           
  make_sure_that( p_src < DMA_MEM_PTR_MAX );
  dma_cb.ctrl.PTR_IN = p_src; 
} // @ToDo: Rename DMA_PTR_IN_*  ->  DMA_SRC_PTR_* 


/**
 * @brief Write to the write (destination) pointer register of the DMA.
 * @param p_src Any valid memory address.
 */
void dma_set_dst( uint32_t * p_dst )
{
  make_sure_that( p_dst < DMA_MEM_PTR_MAX );
  dma_cb.ctrl.PTR_OUT = p_dst;
} // @ToDo: Rename DMA_PTR_OUT_*  ->  DMA_DST_PTR_*


/**
 * @brief Write to the count/start register of the DMA // juan q: review this explanation.
 * @param p_copySize_du Size (in data units) to be copied from the source to the destination pointers.
 *                      Number of data units (du) = copy size in bytes / size of the data type.
 *                      e.g. If 16 Half Words (DMA_DATA_TYPE_HALF_WORD) are to be copied then p_copySize_du = 16.   
 */                       // juan q: in bytes or in data units?
void dma_set_size( uint32_t * p_copySize_du)
{
  make_sure_that( p_copySize < DMA_MEM_SIZE_MAX );
  dma_cb.ctrl.DMA_START = p_copySize_du;
} // @ToDo: Rename DMA_DMA_START_*  ->  DMA_CNT_START_*
// juan q ruben: Does the count keep the amount left to transmit? So if cnt == 0, done == 1 ? 



/**
 * @brief Write to the source-pointer-increment register of the DMA.
 * @param p_inc_du Number of data units to increment after each read.
 */
void dma_set_src_ptr_inc( uint32_t p_inc_du )
{
    /*
     * Increment size must be passed as a multiple of the data_unit to prevent overlapping. 
     */
  make_sure_that( /*There is no possible check*/ );
  dma_cb.ctrl.SRC_PTR_INC = p_inc_du;
}


/**
 * @brief Write to the destination-pointer-increment register of the DMA.
 * @param p_inc_du Number of data units to increment after each write.
 */
void dma_set_dst_ptr_inc( uint32_t p_inc_du )
{
    /*
     * Increment size must be passed as a multiple of the data_unit to prevent overlapping. 
     */
  make_sure_that( /*There is no possible check*/ );
  dma_cb.ctrl.DST_PTR_INC = p_inc_du;
}


/**
 * @brief Write to the SPI mode register of the DMA.
 * @param p_src A valid SPI mode:
 *              - DMA_DIR_M2M    : Transfer Memory to Memory. //juan: add a sdk function to do this
                - DMA_DIR_SPI_RX         : Receive from SPI. Wait for Tx FIFO. //juan q: what is this?
                - DMA_DIR_SPI_TX         : Send to SPI. Wait for Rx FIFO.
                - DMA_DIR_SPI_FLASH_RX   : Receive from SPI Flash.     
                - DMA_DIR_SPI_FLASH_TX   : Send to SPI Flash. 
 */
void dma_set_direction( dma_dir_t p_dir)
{
  make_sure_that( p_dir < DMA_DIR__size );
  dma_cb.ctrl.SPI_MODE = p_dir;
}


/**
 * @brief Write to the data type register of the DMA.
 * @param p_src A valid data type. // juan: specify which, they are in dma_regs.h
 */
void dma_set_data_type( dma_data_type_t p_type)
{
  make_sure_that( p_type < DMA_DATA_TYPE__size );
  dma_cb.ctrl.DATA_TYPE = p_type;
} 


/**
 * @brief Sets the type of event that will determine the end of the transfer.
 * @param p_event A valid type of event.
 */
void dma_set_end_event( dma_end_event_t p_event )
{
    make_sure_that( p_event < DMA_END_EVENT__size );
    dma_cb.endEvent = p_event;
}



/**
 * @brief Writes the configuration values stored in the DMA control block into their corresponding registers. 
 *        Beforehand, sanity checks are performed and errors are returned in case any condition is violated.
 *        Also, default values are added when needed. // juan: reqrite this and document in each function the default values (i.e. you dont need to callthis function).
 * @param p_safetyLevel Whether to perform sanity checks (if globally enabled). More than one condition can be masked with the bitwise or operand (x|y).         
 * @param p_allowRealign Whether to allow dma_launch() to change data type and increments to avoid misalignments.  
 * @return 
 */
uint32_t dma_launch( dma_safety_level_t p_safetyLevel, dma_allow_realign_t p_allowRealign ) // juan: should be more than uint32_t.. should be an enum error
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
   
   
    //////////  SET THE POINTERS   //////////
    
    mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_PTR_IN_REG_OFFSET), dma_cb.ctrl.PTR_IN );
    mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_PTR_OUT_REG_OFFSET), dma_cb.ctrl.PTR_OUT );
       
    //////////  SET THE INCREMENTS   //////////    
    uint8_t dataSize = DMA_DATA_TYPE_2_DATA_SIZE(dma_cb.ctrl.DATA_TYPE); // Converts pointer increments to bytes. The register of the DMA needs number of bytes. 
    mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_SRC_PTR_INC_REG_OFFSET), dma_cb.ctrl.SRC_PTR_INC * dataSize); 
    mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DST_PTR_INC_REG_OFFSET), dma_cb.ctrl.DST_PTR_INC * dataSize);
            
    //////////  SET DIRECTION AND DATA TYPE   //////////    
    mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_SPI_MODE_REG_OFFSET), (uint32_t) dma_cb.ctrl.SPI_MODE );
    mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DATA_TYPE_REG_OFFSET), ( uint32_t ) dma_cb.ctrl.DATA_TYPE );
    
    //////////  SET SIZE TO COPY + LAUNCH THE DMA OPERATION   //////////    
    mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DMA_START_REG_OFFSET), dma_cb.ctrl.DMA_START );
    
    return dma_cb.launchResult;
}



void dma_reset()
{

// juan: consider some default values!! What if the user just forgot to set something up?
}

uint32_t dma_abort()
{
    // juan: let the user know how many data units where left. But watch out... may prefer not to communicate with the dma. 
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
 * @retval 0 if no realignment is needed.
 * @retval 1 if realignment is needed, but switching to half the word size would fix it.
 * @retval 2 if a WORD is to be read from an odd pointer, so BYTE data type is needed instead. 
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


/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/

// juan: Agregar las condiciones de fallo en la docu de las funciones de set. 

// juan : cubrirme que si el origen/destino es SPI no hay que hacer chequeos de alneacion.