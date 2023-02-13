/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************                          
**                                                                         
** project  : X-HEEP                                                       
** filename : j_dma.c                                                      
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
* @file   j_dma.c
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

// juan: include real assert after Jose
//#include <assert.h>

#include "dma_structs.h"  // Generated
#include "dma_regs.h"     // Generated

#include "mmio.h"
#include "core_v_mini_mcu.h" //juan q: ok?


/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

// juan: remove this, is just a placeholder until real assert can be included
#define assert_me_please(x)

// source: https://stackoverflow.com/questions/600293/how-to-check-if-a-number-is-a-power-of-2
#define IS_POWER_OF_2(x) ( x && ( ( x & -x ) == x ) )

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
 *  Configuration and information parameters of the DMA peripheral
 */
static struct 
{
  /**
   * Control variables for the DMA peripheral 
  */
  dma    ctrl; 
  /**
    * The base address for the soc_ctrl hardware registers.
   */
  mmio_region_t baseAdd; 
} dma_ctrl;


/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

void dma_init(){
  // juan: This should be deprecated. base address should be obtained from.....
  dma_ctrl.baseAdd = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);
  // juan: prob some more stuff should go here.
  // e.g. this function could return something
}

// juan q: is it ok to inline exported function?
/*inline*/ void dma_set_src( uint32_t * p_src ) // juan q: what is the {} standard? 
{                                           // juan q: naming standards?
  assert_me_please( p_src < DMA_MEM_PTR_MAX )        // 
  mmio_region_write32(dma_ctrl.baseAdd, (ptrdiff_t)(DMA_PTR_IN_REG_OFFSET), p_src);
}

/*inline*/ void dma_set_dst( uint32_t * p_dst )
{
  assert_me_please( p_dst < DMA_MEM_PTR_MAX )
  mmio_region_write32(dma_ctrl.baseAdd, (ptrdiff_t)(DMA_PTR_OUT_REG_OFFSET), p_dst);
}

/*inline*/ void dma_set_cnt_start( uint32_t * p_copySize)
{
  assert_me_please( p_copySize < DMA_MEM_SIZE_MAX )
  mmio_region_write32(dma_ctrl.baseAdd, (ptrdiff_t)(DMA_DMA_START_REG_OFFSET), p_copySize);
}

// juan q: what could be returned? this way the function name could be more explicit. 
/*inline*/ int32_t dma_is_done()
{
  return mmio_region_read32(dma_ctrl.baseAdd, (ptrdiff_t)(DMA_DONE_REG_OFFSET));
}

/*inline*/ void dma_set_src_ptr_inc( uint32_t p_inc )
{
  assert_me_please( p_inc <= DMA_MEM_INC_MAX && IS_POWER_OF_2(p_inc) )
  mmio_region_write32(dma_ctrl.baseAdd, (ptrdiff_t)(DMA_SRC_PTR_INC_REG_OFFSET), p_inc);
}

/*inline*/ void dma_set_dst_ptr_inc( uint32_t p_inc )
{
  assert_me_please( p_inc <= DMA_MEM_INC_MAX && IS_POWER_OF_2(p_inc) )
  mmio_region_write32(dma_ctrl.baseAdd, (ptrdiff_t)(DMA_DST_PTR_INC_REG_OFFSET), p_inc);
}

/*inline*/ void dma_set_spi_mode( uint32_t p_mode)
{
  // juan: #assert_me_please( check is under SPI_MODE__size)
  mmio_region_write32(dma_ctrl.baseAdd, (ptrdiff_t)(DMA_SPI_MODE_REG_OFFSET), p_mode);
}

/*inline*/ void dma_set_data_type( uint32_t p_type)
{
  // juan: #assert_me_please( check is under DATA_TYPE__size)
  mmio_region_write32(dma_ctrl.baseAdd, (ptrdiff_t)(DMA_DATA_TYPE_REG_OFFSET), p_type);
} 

/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/




// juan q: is it ok if I use netbeans? 
// juan q: is there a script to create the new c/h file from template?
//          it could save preset information (author, institution, copyright, project) 
// juan q: should we rename this module as drv_dma.c/h?  
// juan q: way to distinguish between register and region in abbreviation? 