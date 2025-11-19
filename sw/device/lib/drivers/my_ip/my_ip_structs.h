/*
                              *******************
******************************* H SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : my_ip_structs.h                                 **
** date     : 18/11/2025                                                      **
**                                                                         **
*****************************************************************************
**                                                                         **
**                                                                         **
*****************************************************************************

*/

/**
* @file   my_ip_structs.h
* @date   18/11/2025
* @brief  Contains structs for every register
*
* This file contains the structs of the registes of the peripheral.
* Each structure has the various bit fields that can be accessed
* independently.
* 
*/

#ifndef _MY_IP_STRUCTS_H
#define MY_IP_STRUCTS

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include <inttypes.h>
#include "core_v_mini_mcu.h"

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

#define my_ip_peri ((volatile my_ip *) MY_IP_START_ADDRESS)

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/



typedef struct {

  uint32_t INTR_STATE;                            /*!< Interrupt State Register*/

  uint32_t INTR_ENABLE;                           /*!< Interrupt Enable Register*/

  uint32_t INTR_TEST;                             /*!< Interrupt Test Register*/

  uint32_t CONTROL;                               /*!< Controls for SPI_FLASH W/R*/

  uint32_t R_ADDRESS;                             /*!< Address in Flash to read from*/

  uint32_t S_ADDRESS;                             /*!< Address to store read data from SPI_FLASH*/

  uint32_t LENGTH;                                /*!< Length of data to W/R*/

} my_ip;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

#ifndef _MY_IP_STRUCTS_C_SRC



#endif  /* _MY_IP_STRUCTS_C_SRC */

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/


/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/



#endif /* _MY_IP_STRUCTS_H */
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
