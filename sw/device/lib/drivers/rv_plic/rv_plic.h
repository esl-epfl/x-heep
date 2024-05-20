/*
                              *******************
******************************* C SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : rv_plic.h                                                    **
** date     : 18/04/2023                                                   **
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
* @date   18/04/2023
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
#include "rv_plic_structs.h"

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
typedef uint32_t plic_target_t;


/**
 * The result of a PLIC operation.
 */
typedef enum plic_result {
  /**
   * Indicates that the operation succeeded.
   */
  kPlicOk = 0,
  /**
   * Indicates some unspecified failure.
   */
  kPlicError = 1,
  /**
   * Indicates that some parameter passed into a function failed a
   * precondition.
   *
   * When this value is returned, no hardware operations occurred.
   */
  kPlicBadArg = 2,
} plic_result_t;


/**
 * A toggle state: enabled, or disabled.
 *
 * This enum may be used instead of a `bool` when describing an enabled/disabled
 * state.
 */
typedef enum plic_toggle {
  /**
   * The "disabled" state.
   */
  kPlicToggleDisabled,
  /*
   * The "enabled" state.
   */
  kPlicToggleEnabled
} plic_toggle_t;


/**
 * An interrupt trigger type.
 */
typedef enum plic_irq_trigger {
  /**
   * Trigger on a level (when the signal remains high).
   */
  kPlicIrqTriggerLevel,
  /**
   * Trigger on an edge (when the signal changes from low to high).
   */
  kPlicIrqTriggerEdge
} plic_irq_trigger_t;

#define QTY_INTR_PLIC 64
/**
 * Pointer used to dynamically access the different interrupt handlers.
*/
typedef void (*rv_plic_handler_funct_t)(uint32_t);

/**
 * Colletions of information about one plic and it's runtime components.
 */
typedef struct {
  RV_PLIC* rv_plic_peri;
  
  /**
    * Array for the ISRs.
    * Each element will be initialized to be the address of the handler function
    * relative to its index. So each element will be a callable function.
    */
  rv_plic_handler_funct_t handlers[QTY_INTR_PLIC];
  rv_plic_handler_funct_t default_handlers[QTY_INTR_PLIC];
} rv_plic_inf_t;

#include "rv_plic_gen.h"

/****************************************************************************/
/**                                                                        **/
/*                          EXPORTED FUNCTIONS                              */
/**                                                                        **/
/****************************************************************************/

/**
 * Generic handler for the interrupts in inputs to RV_PLIC.
 * Its basic purpose is to understand which source generated
 * the interrupt and call the proper specific handler. The source
 * is detected by reading the CC0 register (claim interrupt), containing
 * the ID of the source.
 * Once the interrupt routine is finished, this function sets to 1 the
 * external_intr_flag and calls plic_irq_complete() function to conclude
 * the handling.
*/
void handler_irq_external(void);

/**
 * Initilises the PLIC peripheral's registers with default values.
 *
 * @return The result of the operation.
 */
plic_result_t plic_Init(rv_plic_inf_t *inf);


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
 * @param state The new toggle state for the interrupt
 * @return The result of the operation
*/
plic_result_t plic_irq_set_enabled(rv_plic_inf_t *inf, uint32_t irq,
                                    plic_toggle_t state);


/**
 * Reads a specific bit of the Interrupt Enable registers to understand
 * if the corresponding interrupt is enabled or disabled.
 *
 * For a specific target, each interrupt source has a dedicated enable/disable bit
 * inside the relative Interrupt Enable registers, basing on the interrupt
 * source id.
 *
 * The resulting bit is saved inside the state variable passed as parameter
 *
 * @param irq An interrupt source identification
 * @param state The toggle state of the interrupt, as read from the IE registers
 * @return The result of the operation
*/
plic_result_t plic_irq_get_enabled(rv_plic_inf_t *inf, uint32_t irq,
                                    plic_toggle_t *state);

/**
 * Sets the interrupt request trigger type.
 *
 * For a specific interrupt line, identified by irq, sets if its trigger
 * type has to be edge or level.
 * Edge means that the interrupt is triggered when the source passes from low to high.
 * Level means that the interrupt is triggered when the source stays at a high level.
 *
 * @param irq An interrupt source identification
 * @param triggger The trigger state for the interrupt
 * @result The result of the operation
 *
*/
plic_result_t plic_irq_set_trigger(rv_plic_inf_t *inf, uint32_t irq,
                                    plic_irq_trigger_t trigger);

/**
 * Sets a priority value for a specific interrupt source
 *
 * @param irq An interrupt source identification
 * @param priority A priority value to set
 * @return The result of the operation
*/
plic_result_t plic_irq_set_priority(rv_plic_inf_t *inf,  uint32_t irq,
                                      uint32_t priority);

/**
 * Sets the priority threshold.
 *
 * PLIC will only interrupt a target when
 * IRQ source priority is set higher than the priority threshold for the
 * corresponding target.
 *
 * @param threshold The threshold value to be set
 * @return The result of the operation
*/
plic_result_t plic_target_set_threshold(rv_plic_inf_t *inf, uint32_t threshold);

/**
 * Returns whether a particular interrupt is currently pending.
 *
 * @param irq An interrupt source identification
 * @param[out] is_pending Boolean flagcorresponding to whether an interrupt is pending or not
*/
plic_result_t plic_irq_is_pending(rv_plic_inf_t *inf, uint32_t irq,
                                   bool *is_pending);

/**
 * Claims an IRQ and gets the information about the source.
 *
 * Claims an IRQ and returns the IRQ related data to the caller. This function
 * reads a target specific Claim/Complete register. #plic_irq_complete must
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
 * @param[out] claim_data Data that describes the origin of the IRQ.
 * @return The result of the operation.
 */
plic_result_t plic_irq_claim(rv_plic_inf_t *inf, uint32_t *claim_data);

/**
 * Completes the claimed interrupt request.
 *
 * After an interrupt request is served, the core writes the interrupt source
 * ID into the Claim/Complete register.
 *
 * This function must be called after plic_irq_claim(), when the core is
 * ready to service others interrupts with the same ID. If this function
 * is not called, future claimed interrupts will not have the same ID.
 *
 * @param complete_data Previously claimed IRQ data that is used to signal
 * PLIC of the IRQ servicing completion.
 * @return The result of the operation
*/
plic_result_t plic_irq_complete(rv_plic_inf_t *inf, const uint32_t *complete_data );


/**
 * Forces the software interrupt.
 *
 * This function causes an interrupt to the to be serviced as if
 * hardware had asserted it.
 *
 * An interrupt handler is expected to call `plic_software_irq_acknowledge`
 * when the interrupt has been handled.
 *
 * @return The result of the operation
 */
void plic_software_irq_force(rv_plic_inf_t *inf);


/**
 * Acknowledges the software interrupt.
 *
 * This function indicates to the hardware that the software interrupt has been
 * successfully serviced. It is expected to be called from a software interrupt
 * handler.
 *
 * @return The result of the operation
 */
void plic_software_irq_acknowledge(rv_plic_inf_t *inf);

/**
 * Returns software interrupt pending state
 *
 * @return The result of the operation
*/
plic_result_t plic_software_irq_is_pending(rv_plic_inf_t *inf);

/**
 * Adds a handler function for an interrupt to the handlers list.
 * @param id The interrupt ID of an interrupt (from core_v_mini_mcu.h)
 * @param handler A pointer to a function that will be called upon interrupt.
 * @return The result of the operation
*/
plic_result_t plic_assign_irq_handler(rv_plic_inf_t *inf, uint32_t id, rv_plic_handler_funct_t handler );

/**
 * Resets all peripheral handlers to their pre-set ones. All external handlers
 * are re-set to the dummy handler.
 */
void plic_reset_handlers_list(rv_plic_inf_t *inf);

#endif /* _RV_PLIC_H_ */

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
