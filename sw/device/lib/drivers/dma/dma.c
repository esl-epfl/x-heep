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
#include "core_v_mini_mcu.h" // @ToDo: Include this inside _regs.h


/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

// ToDo: Juan - remove this, is just a placeholder until real assert can be included
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
struct 
{
  /**
   * Control variables for the DMA peripheral 
  */
  dma    ctrl; 
  /**
    * The base address for the soc_ctrl hardware registers.
   */
  mmio_region_t baseAdd; 
} dma_cb; 


/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

void dma_init()
{
  // @ToDo: This should be deprecated. base address should be obtained from.....
  dma_cb.baseAdd = mmio_region_from_addr((uintptr_t)DMA_START_ADDRESS);
  // juan: prob some more stuff should go here.
  // e.g. this function could return something
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