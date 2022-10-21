// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#ifndef _POWER_MANAGER_H_
#define _POWER_MANAGER_H_

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"
#include "power_manager_regs.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Results.
 */
typedef enum power_manager_result {
  kPowerManagerOk_e    = 0,
  kPowerManagerError_e = 1,
} power_manager_result_t;

/**
 * Domains.
 */
typedef enum power_manager_sel_domain {
% for bank in range(ram_numbanks):
  kRam_${bank}_e   = ${bank},
% endfor
  kPeriph_e  = ${ram_numbanks},
} power_manager_sel_domain_t;

/**
 * Domain states.
 */
typedef enum power_manager_sel_state {
  kOn_e  = 0,
  kOff_e = 1,
} power_manager_sel_state_t;


/**
 * ACK Memories.
 */

typedef struct power_manager_ram_map_t {
  uint32_t ack_bitfield;
  uint32_t ack_reg_addr;
  uint32_t switch_off_counter;
  uint32_t switch_on_counter;
  uint32_t iso_off_counter;
  uint32_t iso_on_counter;
  uint32_t power_gate_reg_addr;
  uint32_t power_gate_reg_ack_addr;
  uint32_t stop_counter_switch_off_bitfield;
  uint32_t stop_counter_switch_on_bitfield;
  uint32_t stop_counter_iso_off_bitfield;
  uint32_t stop_counter_is_on_bitfield;
  uint32_t stop_reg_addr;
} power_manager_ram_map_t;

static power_manager_ram_map_t power_manager_ram_map[${ram_numbanks}] = {
% for bank in range(ram_numbanks):
  {
    POWER_MANAGER_RAM_${bank}_WAIT_ACK_SWITCH_ON_COUNTER_RAM_${bank}_WAIT_ACK_SWITCH_ON_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_WAIT_ACK_SWITCH_ON_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_SWITCH_OFF_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_SWITCH_ON_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_ISO_OFF_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_ISO_ON_COUNTER_REG_OFFSET,
    POWER_MANAGER_POWER_GATE_RAM_BLOCK_${bank}_REG_OFFSET,
    POWER_MANAGER_POWER_GATE_RAM_BLOCK_${bank}_ACK_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_RAM_${bank}_SWITCH_OFF_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_RAM_${bank}_SWITCH_ON_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_RAM_${bank}_ISO_OFF_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_RAM_${bank}_ISO_ON_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_REG_OFFSET,
   },
% endfor
};

/**
 * Interrupt source.
 */
typedef enum power_manager_sel_intr {
  kTimer_0_e  = 0,
  kPlic_e     = 1,
  kTimer_1_e  = 2,
  kTimer_2_e  = 3,
  kTimer_3_e  = 4,
  kDma_e      = 5,
  kSpi_e      = 6,
  kSpiFlash_e = 7,
  kGpio_0_e   = 8,
  kGpio_1_e   = 9,
  kGpio_2_e   = 10,
  kGpio_3_e   = 11,
  kGpio_4_e   = 12,
  kGpio_5_e   = 13,
  kGpio_6_e   = 14,
  kGpio_7_e   = 15,
  kExt_0_e    = 16,
  kExt_1_e    = 17,
  kExt_2_e    = 18,
  kExt_3_e    = 19,
} power_manager_sel_intr_t;

/**
 * Initialization parameters for POWER MANAGER.
 *
 */
typedef struct power_manager {
  /**
   * The base address for the power_manager hardware registers.
   */
  mmio_region_t base_addr;
} power_manager_t;

typedef struct power_manager_counters {
  /**
   * The counter to set and unset the reset and switch of the CPU.
   */
  uint32_t reset_off;
  uint32_t reset_on;
  uint32_t switch_off;
  uint32_t switch_on;
  uint32_t iso_off;
  uint32_t iso_on;

} power_manager_counters_t;

power_manager_result_t power_gate_counters_init(power_manager_counters_t* counters, uint32_t reset_off, uint32_t reset_on, uint32_t switch_off, uint32_t switch_on, uint32_t iso_off, uint32_t iso_on);

power_manager_result_t power_gate_core(const power_manager_t *power_manager, power_manager_sel_intr_t sel_intr, power_manager_counters_t* cpu_counters);

power_manager_result_t power_gate_domain(const power_manager_t *power_manager, power_manager_sel_domain_t sel_domain, power_manager_sel_state_t sel_state, power_manager_counters_t* domain_counters);

uint32_t power_domain_is_off(const power_manager_t *power_manager, power_manager_sel_domain_t sel_domain);

#ifdef __cplusplus
}
#endif

#endif  // _POWER_MANAGER_H_
