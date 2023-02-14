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

// juan: include real assert after Jose
//#include <assert.h>

#include "dma.h"

#include "dma_structs.h"  // Generated
#include "dma_regs.h"     // Generated

#include "mmio.h"
#include "core_v_mini_mcu.h" //juan r: ok? Considerar incluir el core-v-mini-mcu.h en el xxx_reg.h


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
 *  Control Block (CB) of the DMA peripheral. 
 * Has variables and constant necessary/useful for its control. 
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
} dma_cb; // juan: not a good name, specially if the stefano_structs struct is going to be called ctrl


/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

void dma_init()
{
  // juan: This should be deprecated. base address should be obtained from.....
  dma_cb.baseAdd = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);
  // juan: prob some more stuff should go here.
  // e.g. this function could return something
}

// juan r: is it ok to inline exported function? Put definition with prototype in .h
void dma_set_src( uint32_t * p_src ) // juan q: what is the {} standard? 
{                                           // juan q: naming standards? Let put of accuerd. Review whats already done. 
  assert_me_please( p_src < DMA_MEM_PTR_MAX );        
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_PTR_IN_REG_OFFSET), p_src);
}                                                   // juan r: can we rename this as src_ptr?? for consistency
                                                     //               - no, make the change in the function name, and leave a ToDo. 
                                                    // juan r: in dma_regs.h #19: "// Input data pointer (word aligned)"  what is word aligned here?  

void dma_set_dst( uint32_t * p_dst )
{
  assert_me_please( p_dst < DMA_MEM_PTR_MAX );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_PTR_OUT_REG_OFFSET), p_dst);
}

void dma_set_cnt_start( uint32_t * p_copySize_du)
{
  assert_me_please( p_copySize < DMA_MEM_SIZE_MAX );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DMA_START_REG_OFFSET), p_copySize_du);
}

// juan r: what could be returned? this way the function name could be more explicit.  Es bool ver abajo
int32_t dma_is_done()
{
  return mmio_region_read32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DONE_REG_OFFSET));
}

/*
 // juan r: "you can set Half-Word data size for the peripheral to access its data register and set Word data size
           for the Memory to gain in access time. Each two half words will be packed and written in
           a single access to a Word in the Memory." 
 *          "When FIFO is disabled, it is not allowed to configure different Data Sizes for Source
           and Destination. In this case the Peripheral Data Size will be applied to both Source
           and Destination."
 *          STM32 DMA driver: https://github.com/bkht/STM32-HAL-DMA-Interrupt/blob/master/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
 *          Line 63
 * 
 * This would be a difference in increments? Ya esta respondido
 * 
 * 
 * 
 */


void dma_set_src_ptr_inc( uint32_t p_inc )
{
  assert_me_please( p_inc <= DMA_MEM_INC_MAX && IS_POWER_OF_2(p_inc) );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_SRC_PTR_INC_REG_OFFSET), p_inc);
}

void dma_set_dst_ptr_inc( uint32_t p_inc )
{
  assert_me_please( p_inc <= DMA_MEM_INC_MAX && IS_POWER_OF_2(p_inc) );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DST_PTR_INC_REG_OFFSET), p_inc);
}

void dma_set_spi_mode( dma_spi_mode_t p_mode)
{
  assert_me_please( p_mode < DMA_SPI_MODE__size );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_SPI_MODE_REG_OFFSET), (uint32_t) p_mode);
}

void dma_set_data_type( dma_data_type_t p_type)
{
  assert_me_please( p_type < DMA_DATA_TYPE__size );
  mmio_region_write32(dma_cb.baseAdd, (ptrdiff_t)(DMA_DATA_TYPE_REG_OFFSET), ( uint32_t ) p_type);
} 



// juan r: should add a enable/disable interrupt? where is interrupt set?  Yes see notebook
// juan r: should the DMA be enabled/disabled? no
// juan r: should get counter with a function? Yes please

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


lib/drv/inc/


// juan r: is it ok if I use netbeans? pleantearlo en la biweekly
// juan r: is there a script to create the new c/h file from template? Ask stefano
//          it could save preset information (author, institution, copyright, project) 
// juan r: should we rename this module as drv_dma.c/h?  ask jose in biweekly
// juan r: way to distinguish between register and region in abbreviation? stay aleert

// juan r: I will need different streams >>> which information is needed for each?  

// juan r: is the way we are hierarchically organizing the project IDE-friendly': Decide which IDE in the biweekly.


// juan: for example DMA-peripheral firstly just toogle a GPIO and sniff it in the oscilloscope. 
//       it could be compared vs. doing the same thing w/ the CPU

// juan : Hacer que done tome solo el bit 0 y retorne bool ??? Esto deber'a ser un toDo. Lo del bool no.  