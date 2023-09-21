/*
                              *******************
******************************* H SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : X-HEEP                                                       **
** filename : power_manager.h                                              **
** version  : 1                                                            **
** date     : 22/08/23                                                     **
**                                                                         **
*****************************************************************************
**                                                                         **
** Copyright (c) EPFL contributors.                                        **
** All rights reserved.                                                    **
**                                                                         **
*****************************************************************************

VERSION HISTORY:
----------------
Version     : 1
Date        : -/-/-
Revised by  : -
Description : Original version

*/

/**
 * @todo
 * Create enums for modes and interrupts
 * remove unnecesary structs
 * initilization slave registers
 * review headers
 * Different counters for each peripheral
 * Test all external examples in questa and fpga
 * Test dma-power-gate in questa and fpga
 * Add to dma power gate cannot execute in verilator
 * Check clock gate in the simultion
*/

/**
 * @todo sanity checks - assert or error?
*/

/***************************************************************************/
/***************************************************************************/

/**
* @file   power_manager.h
* @date   22/08/23
* @brief  The Power Manager driver to set up and use the power manager
* peripheral
*
* X-HEEP power domains (take from paper)
*/

#ifndef _POWER_MANAGER_H_
#define _POWER_MANAGER_H_

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include "power_manager_regs.h"     // Generated
#include "power_manager_structs.h"  // Generated

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/**
 * Error return for power manager init counters functions
 *
 * This enum describes the return type from power manager HAL functions
 */
typedef enum power_manager_result {
  /*
   * Ok return status (no error)
   */
  kPowerManagerOk_e    = 0,
  /*
   * Error return value
   */
  kPowerManagerError_e = 1,
} power_manager_result_t;


/**
 * Domain states.
 *
 *
 */
typedef enum power_manager_sel_state {
  /**
   *
   */
  kOn_e     = 0,
  /**
   *
   */
  kOff_e    = 1,
  /**
   *
   */
  kRetOn_e  = 2,
  /**
   *
   */
  kRetOff_e = 3,
} power_manager_sel_state_t;

/**
 * Interrupt source.
 *
 *
 */
typedef enum power_manager_sel_intr {
  /**
   *
   */
  kTimer_0_pm_e  = 0,
  /**
   *
   */
  kPlic_pm_e     = 1,
  /**
   *
   */
  kTimer_1_pm_e  = 2,
  /**
   *
   */
  kTimer_2_pm_e  = 3,
  /**
   *
   */
  kTimer_3_pm_e  = 4,
  /**
   *
   */
  kDma_pm_e      = 5,
  /**
   *
   */
  kSpi_pm_e      = 6,
  /**
   *
   */
  kSpiFlash_pm_e = 7,
  /**
   *
   */
  kGpio_0_pm_e   = 8,
  /**
   *
   */
  kGpio_1_pm_e   = 9,
  /**
   *
   */
  kGpio_2_pm_e   = 10,
  /**
   *
   */
  kGpio_3_pm_e   = 11,
  /**
   *
   */
  kGpio_4_pm_e   = 12,
  /**
   *
   */
  kGpio_5_pm_e   = 13,
  /**
   *
   */
  kGpio_6_pm_e   = 14,
  /**
   *
   */
  kGpio_7_pm_e   = 15,
  /**
   *
   */
  kExt_0_pm_e    = 16,
  /**
   *
   */
  kExt_1_pm_e    = 17,
  /**
   *
   */
  kExt_2_pm_e    = 18,
  /**
   *
   */
  kExt_3_pm_e    = 19,
} power_manager_sel_intr_t;

/**
 * Monitor signals.
 *
 *
 */
typedef struct monitor_signals {
  /**
   *
   */
  uint32_t kSwitch_e;
  /**
   *
   */
  uint32_t kIso_e;
  /**
   *
   */
  uint32_t kReset_e;
} monitor_signals_t;

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
 *@brief Initialize all Power Manager configuration registers to a safe default state.
 * It can be called anytime to reset the Power Manager control block. @todo
 * @param peri Pointer to a register address following the power_manager structure. By
 * default (peri == NULL), the integrated power_manager will be used.
 */
void power_manager_init( power_manager *peri );

/**
 * Creates a new handle
 *
 * This function does not actuate the hardware.
 *
 * @param reset_off
 * @param reset_on
 * @param switch_off
 * @param switch_on
 * @param iso_off
 * @param iso_on
 * @param retentive_off
 * @param retentive_on
 * @return The result of the operation.
 */
power_manager_result_t power_gate_counters_init(uint32_t reset_off, uint32_t reset_on, uint32_t switch_off, uint32_t switch_on, uint32_t iso_off, uint32_t iso_on, uint32_t retentive_off, uint32_t retentive_on);

/**
 *
 *
 *
 *
 * @param sel_state
 * @return The result of the operation.
 */
power_manager_result_t clock_gate_periph(power_manager_sel_state_t sel_state);

/**
 *
 *
 *
 *
 * @param sel_block
 * @param sel_state
 * @return The result of the operation.
 */
power_manager_result_t clock_gate_ram_block(uint32_t sel_block, power_manager_sel_state_t sel_state);

/**
 *
 *
 *
 *
 * @param sel_external
 * @param sel_state
 * @return The result of the operation.
 */
power_manager_result_t clock_gate_external(uint32_t sel_external, power_manager_sel_state_t sel_state);

/**
 *
 *
 *
 *
 * @param sel_intr
 * @return The result of the operation.
 */
power_manager_result_t power_gate_core(power_manager_sel_intr_t sel_intr);

/**
 *
 *
 *
 *
 * @param sel_state
 * @return The result of the operation.
 */
power_manager_result_t power_gate_periph(power_manager_sel_state_t sel_state);

/**
 *
 *
 *
 *
 * @param sel_block
 * @param sel_state
 * @return The result of the operation: 0 for success, 1 if trying to access innexistant RAM
 */
power_manager_result_t power_gate_ram_block(uint32_t sel_block, power_manager_sel_state_t sel_state);

/**
 *
 *
 *
 *
 * @param sel_external
 * @param sel_state
 * @return The result of the operation.
 */
power_manager_result_t power_gate_external(uint32_t sel_external, power_manager_sel_state_t sel_state);

/**
 *
 *
 *
 *
 * @return The result of the operation.
 */
uint32_t periph_power_domain_is_off();

/**
 *
 *
 *
 *
 * @param sel_block
 * @return The result of the operation.
 */
uint32_t ram_block_power_domain_is_off(uint32_t sel_block);

/**
 *
 *
 *
 *
 * @param sel_external
 * @return The result of the operation.
 */
uint32_t external_power_domain_is_off(uint32_t sel_external);

/**
 *
 *
 *
 *
 * @return The result of the operation.
 */
monitor_signals_t monitor_power_gate_core();

/**
 *
 *
 *
 *
 * @return The result of the operation.
 */
monitor_signals_t monitor_power_gate_periph();

/**
 *
 *
 *
 *
 * @param sel_block
 * @return The result of the operation.
 */
monitor_signals_t monitor_power_gate_ram_block(uint32_t sel_block);

/**
 *
 *
 *
 *
 * @param sel_external
 * @return The result of the operation.
 */
monitor_signals_t monitor_power_gate_external(uint32_t sel_external);

/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  // _POWER_MANAGER_H_

/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
