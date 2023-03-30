/*
                              *******************
******************************* C SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : rv_plic.h                                                    **
** date     : 28/03/2023                                                   **
**                                                                         **
*****************************************************************************
**                                                                         **
**                                                                         **
*****************************************************************************

*/

/***************************************************************************/
/***************************************************************************/

/**
* @file   rv_plic.h
* @date   28/03/2023
* @brief  This is the main file for the HAL of the RV_PLIC peripheral
*
* In this file there are the defintions of the public HAL functions for the RV_PLIC
* peripheral. They provide many low level functionalities to interact
* with the registers content, reading and writing them according to the specific
* function of each one.
*
*/

#ifndef _RV_PLIC_H_
#define _RV_PLIC_H_

/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "mmio.h"
#include "macros.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                        TYPEDEFS AND STRUCTURES                           */
/**                                                                        **/
/****************************************************************************/

/**
 * A PLIC interrupt target.
 *
 * This corresponds to a specific system that can service an interrupt. In
 * OpenTitan's case, that is the Ibex core. If there were multiple cores in the
 * system, each core would have its own specific interrupt target ID.
 *
 * This is an unsigned 32-bit value that is at least 0 and is less than the
 * `NumTarget` instantiation parameter of the `rv_plic` device.
 */
typedef uint32_t dif_plic_target_t;


/**
 * A PLIC interrupt source identifier.
 *
 * This corresponds to a specific interrupt, and not the device it originates
 * from.
 *
 * This is an unsigned 32-bit value that is at least zero and is less than the
 * `NumSrc` instantiation parameter of the `rv_plic` device.
 *
 * The value 0 corresponds to "No Interrupt".
 */
typedef uint32_t dif_plic_irq_id_t;


/**
 * The result of a PLIC operation.
 */
typedef enum dif_plic_result {
  /**
   * Indicates that the operation succeeded.
   */
  kDifPlicOk = 0,
  /**
   * Indicates some unspecified failure.
   */
  kDifPlicError = 1,
  /**
   * Indicates that some parameter passed into a function failed a
   * precondition.
   *
   * When this value is returned, no hardware operations occurred.
   */
  kDifPlicBadArg = 2,
} dif_plic_result_t;


/**
 * A toggle state: enabled, or disabled.
 *
 * This enum may be used instead of a `bool` when describing an enabled/disabled
 * state.
 */
typedef enum dif_plic_toggle {
  /*
   * The "enabled" state.
   */
  kDifPlicToggleEnabled,
  /**
   * The "disabled" state.
   */
  kDifPlicToggleDisabled,
} dif_plic_toggle_t;


/**
 * An interrupt trigger type.
 */
typedef enum dif_plic_irq_trigger {
  /**
   * Trigger on an edge (when the signal changes from low to high).
   */
  kDifPlicIrqTriggerEdge,
  /**
   * Trigger on a level (when the signal remains high).
   */
  kDifPlicIrqTriggerLevel,
} dif_plic_irq_trigger_t;


/**
 * Enum for describing all the different types of interrupt
 * sources that the rv_plic handles
*/
typedef enum irq_sources
{
  IRQ_UART_SRC,   // from 1 to 8
  IRQ_GPIO_SRC,   // from 9 to 32 
  IRQ_I2C_SRC,    // from 33 to 48
  IRQ_SPI_SRC,    // line 49
  IRQ_BAD = -1    // default failure case
} irq_sources_t;


/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED VARIABLES                             */
/**                                                                        **/
/****************************************************************************/

extern int8_t external_intr_flag;

/****************************************************************************/
/**                                                                        **/
/*                          EXPORTED FUNCTIONS                              */
/**                                                                        **/
/****************************************************************************/

/**
 * Initilises the PLIC peripheral's registers with default values.
 *
 * @return The result of the operation.
 */
dif_plic_result_t plic_Init(void);


/**
 * Sets wherher a particular interrupt is curently enabled or disabled.
 * This is done by setting a specific bit in the Interrupt Enable registers.
 * 
 * For a specific target, each interrupt source has a dedicated enable/disable bit
 * inside the relative Interrupt Enable registers, basing on the interrupt
 * source id.
 * 
 * This function sets that bit to 0 or 1 depending on the state that it is specified
 * 
 * @param irq An interrupt source identification
 * @param target An interrupt target
 * @param state The new toggle state for the interrupt
 * @return The result of the operation
*/
dif_plic_result_t plic_irq_set_enabled(dif_plic_irq_id_t irq,
                                       dif_plic_target_t target,
                                       dif_plic_toggle_t state);


dif_plic_result_t plic_irq_get_enabled(dif_plic_irq_id_t irq,
                                       dif_plic_target_t target,
                                       dif_plic_toggle_t *state);


dif_plic_result_t plic_irq_set_trigger(dif_plic_irq_id_t irq,
                                           dif_plic_irq_trigger_t trigger);

/**
 * Sets a priority value for a specific interrupt source
 * 
 * @param irq An interrupt source identification
 * @param priority A priority value to set
 * @return The result of the operation
*/
dif_plic_result_t plic_irq_set_priority(dif_plic_irq_id_t irq, uint32_t priority);


dif_plic_result_t plic_target_set_threshold(uint32_t threshold);


dif_plic_result_t plic_irq_is_pending(dif_plic_irq_id_t irq,
                                          bool *is_pending);
                                          
/**
 * Claims an IRQ and gets the information about the source.
 *
 * Claims an IRQ and returns the IRQ related data to the caller. This function
 * reads a target specific Claim/Complete register. #dif_plic_irq_complete must
 * be called in order to allow another interrupt with the same source id to be
 * delivered. This usually would be done once the interrupt has been serviced.
 *
 * Another IRQ can be claimed before a prior IRQ is completed. In this way, this
 * functionality is compatible with nested interrupt handling. The restriction
 * is that you must Complete a Claimed IRQ before you will be able to claim an
 * IRQ with the same ID. This allows a pair of Claim/Complete calls to be
 * overlapped with another pair -- and there is no requirement that the
 * interrupts should be Completed in the reverse order of when they were
 * Claimed.
 *
 * @param target Target that claimed the IRQ.
 * @param[out] claim_data Data that describes the origin of the IRQ.
 * @return The result of the operation.
 */
dif_plic_result_t plic_irq_claim(dif_plic_target_t target, dif_plic_irq_id_t *claim_data);


dif_plic_result_t plic_irq_complete(dif_plic_target_t target,
    const dif_plic_irq_id_t *complete_data);

#endif /* _RV_PLIC_H_ */

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
