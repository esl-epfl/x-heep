/*
                              *******************
******************************* H SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : pdm2pcm_structs.h                                 **
** date     : 11/03/2025                                                      **
**                                                                         **
*****************************************************************************
**                                                                         **
**                                                                         **
*****************************************************************************

*/

/**
* @file   pdm2pcm_structs.h
* @date   11/03/2025
* @brief  Contains structs for every register
*
* This file contains the structs of the registes of the peripheral.
* Each structure has the various bit fields that can be accessed
* independently.
* 
*/

#ifndef _PDM2PCM_STRUCTS_H
#define PDM2PCM_STRUCTS

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

#define pdm2pcm_peri ((volatile pdm2pcm *) PDM2PCM_START_ADDRESS)

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/



typedef struct {

  uint32_t CLKDIVIDX;                             /*!< Control register*/

  uint32_t CONTROL;                               /*!< Control register*/

  uint32_t STATUS;                                /*!< Status register*/

  uint32_t REACHCOUNT;                            /*!< Number of signal taps stored into the FIFO to assert the FILLD bit in the STATUS register.*/

  uint32_t DECIMCIC;                              /*!< Samples count after which to decimate in the CIC filter.*/

  uint32_t DECIMHB1;                              /*!< Samples count after which to decimate after the first halfband filter.*/

  uint32_t DECIMHB2;                              /*!< Samples count after which to decimate after the second halfband filter.*/

  uint32_t HB1COEF00;                             /*!< Filter HB1 coefficient 0*/

  uint32_t HB1COEF01;                             /*!< Filter HB1 coefficient 1*/

  uint32_t HB1COEF02;                             /*!< Filter HB1 coefficient 2*/

  uint32_t HB1COEF03;                             /*!< Filter HB1 coefficient 3*/

  uint32_t HB2COEF00;                             /*!< Filter HB2 coefficient 0*/

  uint32_t HB2COEF01;                             /*!< Filter HB2 coefficient 1*/

  uint32_t HB2COEF02;                             /*!< Filter HB2 coefficient 2*/

  uint32_t HB2COEF03;                             /*!< Filter HB2 coefficient 3*/

  uint32_t HB2COEF04;                             /*!< Filter HB2 coefficient 4*/

  uint32_t HB2COEF05;                             /*!< Filter HB2 coefficient 5*/

  uint32_t HB2COEF06;                             /*!< Filter HB2 coefficient 6*/

  uint32_t FIRCOEF00;                             /*!< Filter FIR coefficient 0*/

  uint32_t FIRCOEF01;                             /*!< Filter FIR coefficient 1*/

  uint32_t FIRCOEF02;                             /*!< Filter FIR coefficient 2*/

  uint32_t FIRCOEF03;                             /*!< Filter FIR coefficient 3*/

  uint32_t FIRCOEF04;                             /*!< Filter FIR coefficient 4*/

  uint32_t FIRCOEF05;                             /*!< Filter FIR coefficient 5*/

  uint32_t FIRCOEF06;                             /*!< Filter FIR coefficient 6*/

  uint32_t FIRCOEF07;                             /*!< Filter FIR coefficient 7*/

  uint32_t FIRCOEF08;                             /*!< Filter FIR coefficient 8*/

  uint32_t FIRCOEF09;                             /*!< Filter FIR coefficient 9*/

  uint32_t FIRCOEF10;                             /*!< Filter FIR coefficient 10*/

  uint32_t FIRCOEF11;                             /*!< Filter FIR coefficient 11*/

  uint32_t FIRCOEF12;                             /*!< Filter FIR coefficient 12*/

  uint32_t FIRCOEF13;                             /*!< Filter FIR coefficient 13*/

  uint32_t RXDATA;                                /*!< PCM Receive data     */

} pdm2pcm;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

#ifndef _PDM2PCM_STRUCTS_C_SRC



#endif  /* _PDM2PCM_STRUCTS_C_SRC */

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



#endif /* _PDM2PCM_STRUCTS_H */
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
