/*
                              *******************
******************************* H SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : X_HEEP                                                       **
** filename : fast_intr_ctrl.h  
** version  : 1                                             **
** date     : 27/03/23                                                   **
**                                                                         **
*****************************************************************************
** 
** Copyright (c) EPFL contributors.                                     
** All rights reserved.                                                                   
**                                                                         
*****************************************************************************
*/

/***************************************************************************/
/***************************************************************************/
/**
* @file   fast_intr_ctrl.h
* @date   27/03/23
* @brief  The fast interrupt controller peripheral driver 
*/

#ifndef _FAST_INTR_CTRL_H_
#define _FAST_INTR_CTRL_H_

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>
#include "mmio.h"

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/
<%
    intr_sources = xheep.get_rh().use_target_as_sv_multi("fast_irq_target", xheep.get_mcu_node())
    intrs = []
    for i, intr_s in enumerate(intr_sources):
        ep = xheep.get_rh().get_source_ep_copy(intr_s.split(".")[-1])
        intrs.append(ep.handler)
%>

#define MIE_MASK_FIC_BASE_BIT 16
% for i, intr in enumerate(intrs):
% if intr != "":
#define MIE_MASK_${intr[len("fic_irq_"):].upper()} ( (uint32_t) 1 << (${i} + MIE_MASK_FIC_BASE_BIT) )
% endif
% endfor


/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/**
 * possible returns of the clear_fast_interrupt function.
 */
typedef enum fast_intr_ctrl_result {
  kFastIntrCtrlOk_e    = 0, /*!< successfully done. */
  kFastIntrCtrlError_e = 1, /*!< an error occured. */
} fast_intr_ctrl_result_t;


/**
 * Specify the modules connected to FIC.
 * This enum is used as an input for clearing the specifc bit inside 
 * pending register while recieving an interrupt through FIC.
 */
typedef enum fast_intr_ctrl_fast_interrupt {
% for i, intr in enumerate(intrs):
% if intr != "":
  k${intr[len("fic_irq_"):].title()}_fic_e = ${i},
% endif
% endfor
} fast_intr_ctrl_fast_interrupt_t;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Enable the generation of the fast interrupt
 * peripheral by writing inside FAST_INTR_ENABLE
 * @param fast_interrupt specify the peripheral that will be enabled
 * @param enable enable value
 * @retval kFastIntrCtrlOk_e (= 0) if successfully set the bit
 * @retval kFastIntrCtrlError_e (= 1) if an error occured during operation
 */
fast_intr_ctrl_result_t enable_fast_interrupt(fast_intr_ctrl_fast_interrupt_t\
 fast_interrupt, bool enable);

 /**
 * @brief Enable the generation of the fast interrupt
 * peripheral by writing inside FAST_INTR_ENABLE
 * @param enable enable value
 * @retval kFastIntrCtrlOk_e (= 0) if successfully set the bit
 * @retval kFastIntrCtrlError_e (= 1) if an error occured during operation
 */
fast_intr_ctrl_result_t enable_all_fast_interrupts(bool enable);


/**
 * @brief clean the bit inside FAST_INTR_PENDING register for the requested 
 * peripheral by writing inside FAST_INTR_CLEAR
 * @param fast_interrupt specify the peripheral that will be cleared
 * @retval kFastIntrCtrlOk_e (= 0) if successfully clear the bit
 * @retval kFastIntrCtrlError_e (= 1) if an error occured during operation
 */
fast_intr_ctrl_result_t clear_fast_interrupt(fast_intr_ctrl_fast_interrupt_t\
 fast_interrupt);

% for intr in intrs:
% if intr != "":
/**
 * @brief fast interrupt controller irq for ${intr}
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void ${intr}(void);

% endif
% endfor


/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/
#ifdef __cplusplus
}
#endif
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
#endif  // _FAST_INTR_CTRL_H_
