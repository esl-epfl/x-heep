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
 * Monitor signals.
 */
typedef struct monitor_signals {
  uint32_t kSwitch_e;
  uint32_t kIso_e;
  uint32_t kReset_e;
} monitor_signals_t;

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
  uint32_t clk_gate;
  uint32_t power_gate_ack;
  uint32_t switch_off;
  uint32_t wait_ack_switch;
  uint32_t iso;
  uint32_t retentive;
  uint32_t monitor_power_gate;
} power_manager_ram_map_t;

static power_manager_ram_map_t power_manager_ram_map[${xheep.ram_numbanks()}] = {
% for bank in xheep.iter_ram_banks():
  (power_manager_ram_map_t) {
    .clk_gate = POWER_MANAGER_RAM_${bank.name()}_CLK_GATE_REG_OFFSET,
    .power_gate_ack = POWER_MANAGER_POWER_GATE_RAM_BLOCK_${bank.name()}_ACK_REG_OFFSET,
    .switch_off = POWER_MANAGER_RAM_${bank.name()}_SWITCH_REG_OFFSET,
    .wait_ack_switch = POWER_MANAGER_RAM_${bank.name()}_WAIT_ACK_SWITCH_ON_REG_OFFSET,
    .iso = POWER_MANAGER_RAM_${bank.name()}_ISO_REG_OFFSET,
    .retentive = POWER_MANAGER_RAM_${bank.name()}_RETENTIVE_REG_OFFSET,
    .monitor_power_gate = POWER_MANAGER_MONITOR_POWER_GATE_RAM_BLOCK_${bank.name()}_REG_OFFSET
  },
% endfor
};

typedef struct power_manager_external_map_t {
  uint32_t clk_gate;
  uint32_t power_gate_ack;
  uint32_t reset;
  uint32_t switch_off;
  uint32_t wait_ack_switch;
  uint32_t iso;
  uint32_t retentive;
  uint32_t monitor_power_gate;
} power_manager_external_map_t;

static power_manager_external_map_t power_manager_external_map[${external_domains}] = {
% for ext in range(external_domains):
  (power_manager_external_map_t) {
    .clk_gate = POWER_MANAGER_EXTERNAL_${ext}_CLK_GATE_REG_OFFSET,
    .power_gate_ack = POWER_MANAGER_POWER_GATE_EXTERNAL_${ext}_ACK_REG_OFFSET,
    .reset = POWER_MANAGER_EXTERNAL_${ext}_RESET_REG_OFFSET,
    .switch_off = POWER_MANAGER_EXTERNAL_${ext}_SWITCH_REG_OFFSET,
    .wait_ack_switch = POWER_MANAGER_EXTERNAL_${ext}_WAIT_ACK_SWITCH_ON_REG_OFFSET,
    .iso = POWER_MANAGER_EXTERNAL_${ext}_ISO_REG_OFFSET,
    .retentive = POWER_MANAGER_EXTERNAL_RAM_${ext}_RETENTIVE_REG_OFFSET,
    .monitor_power_gate = POWER_MANAGER_MONITOR_POWER_GATE_EXTERNAL_${ext}_REG_OFFSET,
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

monitor_signals_t monitor_power_gate_core(const power_manager_t *power_manager);

monitor_signals_t monitor_power_gate_periph(const power_manager_t *power_manager);

monitor_signals_t monitor_power_gate_ram_block(const power_manager_t *power_manager, uint32_t sel_block);

monitor_signals_t monitor_power_gate_external(const power_manager_t *power_manager, uint32_t sel_external);


#ifdef __cplusplus
}
#endif

#endif  // _POWER_MANAGER_H_
