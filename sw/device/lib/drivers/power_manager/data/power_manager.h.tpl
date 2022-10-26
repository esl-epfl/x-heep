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
  kPeriph_e     = 0,
% for bank in range(ram_numbanks):
  kRam_${bank}_e      = ${bank+1},
% endfor
% for ext in range(external_domains):
  kExternal_${ext}_e = ${ram_numbanks+ext+1},
% endfor
} power_manager_sel_domain_t;

/**
 * Domain states.
 */
typedef enum power_manager_sel_state {
  kOn_e     = 0,
  kOff_e    = 1,
  kRetOn_e  = 2,
  kRetOff_e = 3,
} power_manager_sel_state_t;

/**
 * Interrupt source.
 */
typedef enum power_manager_sel_intr {
  kTimer_0_pm_e  = 0,
  kPlic_pm_e     = 1,
  kTimer_1_pm_e  = 2,
  kTimer_2_pm_e  = 3,
  kTimer_3_pm_e  = 4,
  kDma_pm_e      = 5,
  kSpi_pm_e      = 6,
  kSpiFlash_pm_e = 7,
  kGpio_0_pm_e   = 8,
  kGpio_1_pm_e   = 9,
  kGpio_2_pm_e   = 10,
  kGpio_3_pm_e   = 11,
  kGpio_4_pm_e   = 12,
  kGpio_5_pm_e   = 13,
  kGpio_6_pm_e   = 14,
  kGpio_7_pm_e   = 15,
  kExt_0_pm_e    = 16,
  kExt_1_pm_e    = 17,
  kExt_2_pm_e    = 18,
  kExt_3_pm_e    = 19,
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
  uint32_t retentive_off;
  uint32_t retentive_on;
} power_manager_counters_t;

typedef struct power_manager_ram_map_t {
  uint32_t power_gate;
  uint32_t power_gate_ack;
  uint32_t set_retentive;
  uint32_t switch_off_counter;
  uint32_t switch_on_counter;
  uint32_t wait_ack_switch_on_counter;
  uint32_t wait_ack_switch_on_counter_bit;
  uint32_t iso_off_counter;
  uint32_t iso_on_counter;
  uint32_t retentive_off_counter;
  uint32_t retentive_on_counter;
  uint32_t counter_stop;
  uint32_t switch_off_stop_bit;
  uint32_t switch_on_stop_bit;
  uint32_t iso_off_stop_bit;
  uint32_t iso_on_stop_bit;
  uint32_t retentive_off_stop_bit;
  uint32_t retentive_on_stop_bit;
} power_manager_ram_map_t;

static power_manager_ram_map_t power_manager_ram_map[${ram_numbanks}] = {
% for bank in range(ram_numbanks):
  {
    POWER_MANAGER_POWER_GATE_RAM_BLOCK_${bank}_REG_OFFSET,
    POWER_MANAGER_POWER_GATE_RAM_BLOCK_${bank}_ACK_REG_OFFSET,
    POWER_MANAGER_SET_RETENTIVE_RAM_BLOCK_${bank}_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_SWITCH_OFF_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_SWITCH_ON_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_WAIT_ACK_SWITCH_ON_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_WAIT_ACK_SWITCH_ON_COUNTER_RAM_${bank}_WAIT_ACK_SWITCH_ON_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_ISO_OFF_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_ISO_ON_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_RETENTIVE_OFF_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_RETENTIVE_ON_COUNTER_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_REG_OFFSET,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_RAM_${bank}_SWITCH_OFF_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_RAM_${bank}_SWITCH_ON_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_RAM_${bank}_ISO_OFF_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_RAM_${bank}_ISO_ON_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_RAM_${bank}_RETENTIVE_OFF_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_RAM_${bank}_COUNTERS_STOP_RAM_${bank}_RETENTIVE_ON_STOP_BIT_COUNTER_BIT,
  },
% endfor
};

typedef struct power_manager_external_map_t {
  uint32_t power_gate;
  uint32_t power_gate_ack;
  uint32_t reset_assert_counter;
  uint32_t reset_deassert_counter;
  uint32_t switch_off_counter;
  uint32_t switch_on_counter;
  uint32_t wait_ack_switch_on_counter;
  uint32_t wait_ack_switch_on_counter_bit;
  uint32_t iso_off_counter;
  uint32_t iso_on_counter;
  uint32_t counter_stop;
  uint32_t reset_assert_stop_bit;
  uint32_t reset_deassert_stop_bit;
  uint32_t switch_off_stop_bit;
  uint32_t switch_on_stop_bit;
  uint32_t iso_off_stop_bit;
  uint32_t iso_on_stop_bit;
} power_manager_external_map_t;

static power_manager_external_map_t power_manager_external_map[${external_domains}] = {
% for ext in range(external_domains):
  {
    POWER_MANAGER_POWER_GATE_EXTERNAL_${ext}_REG_OFFSET,
    POWER_MANAGER_POWER_GATE_EXTERNAL_${ext}_ACK_REG_OFFSET,
    POWER_MANAGER_EXTERNAL_${ext}_RESET_ASSERT_COUNTER_REG_OFFSET,
    POWER_MANAGER_EXTERNAL_${ext}_RESET_DEASSERT_COUNTER_REG_OFFSET,
    POWER_MANAGER_EXTERNAL_${ext}_SWITCH_OFF_COUNTER_REG_OFFSET,
    POWER_MANAGER_EXTERNAL_${ext}_SWITCH_ON_COUNTER_REG_OFFSET,
    POWER_MANAGER_EXTERNAL_${ext}_WAIT_ACK_SWITCH_ON_COUNTER_REG_OFFSET,
    POWER_MANAGER_EXTERNAL_${ext}_WAIT_ACK_SWITCH_ON_COUNTER_EXTERNAL_${ext}_WAIT_ACK_SWITCH_ON_COUNTER_BIT,
    POWER_MANAGER_EXTERNAL_${ext}_ISO_OFF_COUNTER_REG_OFFSET,
    POWER_MANAGER_EXTERNAL_${ext}_ISO_ON_COUNTER_REG_OFFSET,
    POWER_MANAGER_EXTERNAL_${ext}_COUNTERS_STOP_REG_OFFSET,
    POWER_MANAGER_EXTERNAL_${ext}_COUNTERS_STOP_EXTERNAL_${ext}_RESET_ASSERT_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_EXTERNAL_${ext}_COUNTERS_STOP_EXTERNAL_${ext}_RESET_DEASSERT_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_EXTERNAL_${ext}_COUNTERS_STOP_EXTERNAL_${ext}_SWITCH_OFF_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_EXTERNAL_${ext}_COUNTERS_STOP_EXTERNAL_${ext}_SWITCH_ON_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_EXTERNAL_${ext}_COUNTERS_STOP_EXTERNAL_${ext}_ISO_OFF_STOP_BIT_COUNTER_BIT,
    POWER_MANAGER_EXTERNAL_${ext}_COUNTERS_STOP_EXTERNAL_${ext}_ISO_ON_STOP_BIT_COUNTER_BIT,
  },
% endfor
};

power_manager_result_t power_gate_counters_init(power_manager_counters_t* counters, uint32_t reset_off, uint32_t reset_on, uint32_t switch_off, uint32_t switch_on, uint32_t iso_off, uint32_t iso_on, uint32_t retentive_off, uint32_t retentive_on);

power_manager_result_t power_gate_core(const power_manager_t *power_manager, power_manager_sel_intr_t sel_intr, power_manager_counters_t* cpu_counters);

power_manager_result_t power_gate_periph(const power_manager_t *power_manager, power_manager_sel_state_t sel_state, power_manager_counters_t* periph_counters);

power_manager_result_t power_gate_ram_block(const power_manager_t *power_manager, uint32_t sel_block, power_manager_sel_state_t sel_state, power_manager_counters_t* ram_block_counters);

power_manager_result_t power_gate_external(const power_manager_t *power_manager, uint32_t sel_external, power_manager_sel_state_t sel_state, power_manager_counters_t* external_counters);

uint32_t periph_power_domain_is_off(const power_manager_t *power_manager);

uint32_t ram_block_power_domain_is_off(const power_manager_t *power_manager, uint32_t sel_block);

uint32_t external_power_domain_is_off(const power_manager_t *power_manager, uint32_t sel_external);

#ifdef __cplusplus
}
#endif

#endif  // _POWER_MANAGER_H_
