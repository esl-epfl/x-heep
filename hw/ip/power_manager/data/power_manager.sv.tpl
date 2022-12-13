// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

`include "common_cells/assertions.svh"

module power_manager #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    parameter logic SWITCH_IDLE_VALUE = 1'b1, //the value to have Vdd.daughter = Vdd.mother, i.e. on state
    parameter logic ISO_IDLE_VALUE = 1'b1, //the value to not clamp isolatation cells
    parameter logic RESET_IDLE_VALUE = 1'b1, //the value when the reset is not active (deasserted)
    /*
    these values are used at reset time, i.e.
      the switch of all the power domains are conducting (ON)
      we are not isolating values
      we are resetting
      This should guarantee that the chip boots with the power stable if the
      always-on reset is asserted long-enough to accomplish a power-cycle
      Any value different than the following won't guarantee functionality
      as we do not have any POWER CYCLE FSM in place at reset time,
      this is a simple power manager.
    */
    parameter logic SWITCH_VALUE_AT_RESET = SWITCH_IDLE_VALUE, //the value of the switch at reset
    parameter logic ISO_VALUE_AT_RESET = ISO_IDLE_VALUE, //the value for isolation cells at reset
    parameter logic RESET_VALUE_AT_RESET = ~RESET_IDLE_VALUE //the value when the reset is active
) (
    input logic clk_i,
    input logic rst_ni,

    // Bus Interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    // Status signal
    input logic core_sleep_i,

    // Input interrupt array
    input logic [31:0] intr_i,

    // External interrupts
    input logic [core_v_mini_mcu_pkg::NEXT_INT-1:0] ext_irq_i,

    // Clock gating signals
    output logic peripheral_subsystem_clkgate_en_o,
    output logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0]memory_subsystem_clkgate_en_o,

    // Power gating signals
    output logic cpu_subsystem_powergate_switch_o,
    input  logic cpu_subsystem_powergate_switch_ack_i,
    output logic cpu_subsystem_powergate_iso_o,
    output logic cpu_subsystem_rst_no,
    output logic peripheral_subsystem_powergate_switch_o,
    input  logic peripheral_subsystem_powergate_switch_ack_i,
    output logic peripheral_subsystem_powergate_iso_o,
    output logic peripheral_subsystem_rst_no,
    output logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch_o,
    input  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch_ack_i,
    output logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_iso_o,
    output logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_set_retentive_o,
    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_switch_o,
    input  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_switch_ack_i,
    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_iso_o,
    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_rst_no,
    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_ram_banks_set_retentive_o
);

  import power_manager_reg_pkg::*;

  power_manager_reg2hw_t reg2hw;
  power_manager_hw2reg_t hw2reg;

  logic start_on_sequence;

  assign hw2reg.intr_state.d[15:0] = {
    intr_i[29:22],  // gpio
    intr_i[21],  // spi_flash
    intr_i[20],  // spi
    intr_i[19],  // dma
    intr_i[18],  // rv_timer_3
    intr_i[17],  // rv_timer_2
    intr_i[16],  // rv_timer_1
    intr_i[11],  // plic
    intr_i[7]  // rv_timer_0
  };

  if (core_v_mini_mcu_pkg::NEXT_INT > 16) begin
    assign hw2reg.intr_state.d[31:16] = ext_irq_i[15:0];
  end else begin
    assign hw2reg.intr_state.d[31:16] = $unsigned(ext_irq_i);
  end

  assign hw2reg.intr_state.de = 1'b1;

  power_manager_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) power_manager_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .hw2reg,
      .devmode_i(1'b1)
  );

  logic cpu_subsystem_powergate_switch;
  logic cpu_subsystem_powergate_iso;
  logic cpu_subsystem_rst_n;
  logic peripheral_subsystem_powergate_switch;
  logic peripheral_subsystem_powergate_iso;
  logic peripheral_subsystem_rst_n;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_iso;
  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_switch;
  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_iso;
  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_rst_n;

  assign cpu_subsystem_powergate_switch_o = cpu_subsystem_powergate_switch;
  assign cpu_subsystem_powergate_iso_o = cpu_subsystem_powergate_iso;
  assign cpu_subsystem_rst_no = cpu_subsystem_rst_n;
  assign peripheral_subsystem_powergate_switch_o = peripheral_subsystem_powergate_switch;
  assign peripheral_subsystem_powergate_iso_o = peripheral_subsystem_powergate_iso;
  assign peripheral_subsystem_rst_no = peripheral_subsystem_rst_n;
  assign memory_subsystem_banks_powergate_switch_o = memory_subsystem_banks_powergate_switch;
  assign memory_subsystem_banks_powergate_iso_o = memory_subsystem_banks_powergate_iso;
  assign external_subsystem_powergate_switch_o = external_subsystem_powergate_switch;
  assign external_subsystem_powergate_iso_o = external_subsystem_powergate_iso;
  assign external_subsystem_rst_no = external_subsystem_rst_n;

  // --------------------------------------------------------------------------------------
  // CLK_GATING 
  // --------------------------------------------------------------------------------------

    assign peripheral_subsystem_clkgate_en_o = reg2hw.periph_clk_gate.q;

% for bank in range(ram_numbanks):
    assign memory_subsystem_clkgate_en_o[${bank}] = reg2hw.ram_${bank}_clk_gate.q;
% endfor

  // --------------------------------------------------------------------------------------
  // CPU_SUBSYSTEM DOMAIN
  // --------------------------------------------------------------------------------------

  logic cpu_subsystem_powergate_switch_ack_sync;

  sync #(
      .ResetValue(1'b0)
  ) sync_cpu_ack_i (
      .clk_i,
      .rst_ni,
      .serial_i(cpu_subsystem_powergate_switch_ack_i),
      .serial_o(cpu_subsystem_powergate_switch_ack_sync)
  );

  assign hw2reg.power_gate_core_ack.de = 1'b1;
  assign hw2reg.power_gate_core_ack.d = cpu_subsystem_powergate_switch_ack_sync;

  //if you want to wait for ACK, or just bypass it
  logic cpu_switch_wait_ack;
  assign cpu_switch_wait_ack = reg2hw.cpu_wait_ack_switch_on_counter.q ? reg2hw.power_gate_core_ack.q == SWITCH_IDLE_VALUE : 1'b1;

  always_comb begin : power_manager_start_on_sequence_gen
    if ((reg2hw.en_wait_for_intr.q & reg2hw.intr_state.q)!='0) begin
      start_on_sequence = 1'b1;
    end else begin
      start_on_sequence = 1'b0;
    end
  end

  logic cpu_powergate_counter_start_reset_assert, cpu_powergate_counter_expired_reset_assert;
  logic cpu_powergate_counter_start_reset_deassert, cpu_powergate_counter_expired_reset_deassert;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_cpu_reset_assert_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.cpu_counters_stop.cpu_reset_assert_stop_bit_counter.q),
      .start_i(cpu_powergate_counter_start_reset_assert),
      .done_o(cpu_powergate_counter_expired_reset_assert),
      .hw2reg_d_o(hw2reg.cpu_reset_assert_counter.d),
      .hw2reg_de_o(hw2reg.cpu_reset_assert_counter.de),
      .hw2reg_q_i(reg2hw.cpu_reset_assert_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_cpu_reset_deassert_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.cpu_counters_stop.cpu_reset_deassert_stop_bit_counter.q),
      .start_i(cpu_powergate_counter_start_reset_deassert),
      .done_o(cpu_powergate_counter_expired_reset_deassert),
      .hw2reg_d_o(hw2reg.cpu_reset_deassert_counter.d),
      .hw2reg_de_o(hw2reg.cpu_reset_deassert_counter.de),
      .hw2reg_q_i(reg2hw.cpu_reset_deassert_counter.q)
  );

  power_manager_counter_sequence #(
      .IDLE_VALUE(RESET_IDLE_VALUE),
      .ONOFF_AT_RESET(RESET_VALUE_AT_RESET)
  ) power_manager_counter_sequence_cpu_reset_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i((reg2hw.power_gate_core.q && core_sleep_i) || reg2hw.master_cpu_force_reset_assert.q),
      .start_on_sequence_i (start_on_sequence || reg2hw.master_cpu_force_reset_deassert.q),
      .switch_ack_i (cpu_switch_wait_ack),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(cpu_powergate_counter_expired_reset_assert),
      .counter_expired_switch_on_i (cpu_powergate_counter_expired_reset_deassert),

      .counter_start_switch_off_o(cpu_powergate_counter_start_reset_assert),
      .counter_start_switch_on_o (cpu_powergate_counter_start_reset_deassert),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(cpu_subsystem_rst_n)
  );

  logic cpu_powergate_counter_start_switch_off, cpu_powergate_counter_expired_switch_off;
  logic cpu_powergate_counter_start_switch_on, cpu_powergate_counter_expired_switch_on;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_cpu_powergate_switch_off_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.cpu_counters_stop.cpu_switch_off_stop_bit_counter.q),
      .start_i(cpu_powergate_counter_start_switch_off),
      .done_o(cpu_powergate_counter_expired_switch_off),
      .hw2reg_d_o(hw2reg.cpu_switch_off_counter.d),
      .hw2reg_de_o(hw2reg.cpu_switch_off_counter.de),
      .hw2reg_q_i(reg2hw.cpu_switch_off_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_cpu_powergate_switch_on_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.cpu_counters_stop.cpu_switch_on_stop_bit_counter.q),
      .start_i(cpu_powergate_counter_start_switch_on),
      .done_o(cpu_powergate_counter_expired_switch_on),
      .hw2reg_d_o(hw2reg.cpu_switch_on_counter.d),
      .hw2reg_de_o(hw2reg.cpu_switch_on_counter.de),
      .hw2reg_q_i(reg2hw.cpu_switch_on_counter.q)
  );

  power_manager_counter_sequence #(
      .IDLE_VALUE(SWITCH_IDLE_VALUE),
      .ONOFF_AT_RESET(SWITCH_VALUE_AT_RESET)
  ) power_manager_counter_sequence_cpu_switch_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i((reg2hw.power_gate_core.q && core_sleep_i) || reg2hw.master_cpu_force_switch_off.q),
      .start_on_sequence_i (start_on_sequence || reg2hw.master_cpu_force_switch_on.q),
      .switch_ack_i (1'b1),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(cpu_powergate_counter_expired_switch_off),
      .counter_expired_switch_on_i (cpu_powergate_counter_expired_switch_on),

      .counter_start_switch_off_o(cpu_powergate_counter_start_switch_off),
      .counter_start_switch_on_o (cpu_powergate_counter_start_switch_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(cpu_subsystem_powergate_switch)
  );

  logic cpu_powergate_counter_start_iso_off, cpu_powergate_counter_expired_iso_off;
  logic cpu_powergate_counter_start_iso_on, cpu_powergate_counter_expired_iso_on;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_cpu_powergate_iso_off_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.cpu_counters_stop.cpu_iso_off_stop_bit_counter.q),
      .start_i(cpu_powergate_counter_start_iso_off),
      .done_o(cpu_powergate_counter_expired_iso_off),
      .hw2reg_d_o(hw2reg.cpu_iso_off_counter.d),
      .hw2reg_de_o(hw2reg.cpu_iso_off_counter.de),
      .hw2reg_q_i(reg2hw.cpu_iso_off_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_cpu_powergate_iso_on_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.cpu_counters_stop.cpu_iso_on_stop_bit_counter.q),
      .start_i(cpu_powergate_counter_start_iso_on),
      .done_o(cpu_powergate_counter_expired_iso_on),
      .hw2reg_d_o(hw2reg.cpu_iso_on_counter.d),
      .hw2reg_de_o(hw2reg.cpu_iso_on_counter.de),
      .hw2reg_q_i(reg2hw.cpu_iso_on_counter.q)
  );

  power_manager_counter_sequence #(
    .IDLE_VALUE(ISO_IDLE_VALUE),
    .ONOFF_AT_RESET(ISO_VALUE_AT_RESET)
  ) power_manager_counter_sequence_cpu_iso_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i((reg2hw.power_gate_core.q && core_sleep_i) || reg2hw.master_cpu_force_iso_off.q),
      .start_on_sequence_i (start_on_sequence || reg2hw.master_cpu_force_iso_on.q),
      .switch_ack_i (cpu_switch_wait_ack),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(cpu_powergate_counter_expired_iso_off),
      .counter_expired_switch_on_i (cpu_powergate_counter_expired_iso_on),

      .counter_start_switch_off_o(cpu_powergate_counter_start_iso_off),
      .counter_start_switch_on_o (cpu_powergate_counter_start_iso_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(cpu_subsystem_powergate_iso)
  );

  // --------------------------------------------------------------------------------------
  // PERIPHERAL_SUBSYSTEM DOMAIN
  // --------------------------------------------------------------------------------------

  logic peripheral_subsystem_powergate_switch_ack_sync;

  sync #(
      .ResetValue(1'b0)
  ) sync_periph_ack_i (
      .clk_i,
      .rst_ni,
      .serial_i(peripheral_subsystem_powergate_switch_ack_i),
      .serial_o(peripheral_subsystem_powergate_switch_ack_sync)
  );

  assign hw2reg.power_gate_periph_ack.de = 1'b1;
  assign hw2reg.power_gate_periph_ack.d = peripheral_subsystem_powergate_switch_ack_sync;

  //if you want to wait for ACK, or just bypass it
  logic periph_switch_wait_ack;
  assign periph_switch_wait_ack = reg2hw.periph_wait_ack_switch_on.q ? reg2hw.power_gate_periph_ack.q == SWITCH_IDLE_VALUE : 1'b1;

  power_manager_sequence #(
      .IDLE_VALUE(RESET_IDLE_VALUE),
      .ONOFF_AT_RESET(RESET_VALUE_AT_RESET)
  ) power_manager_sequence_periph_reset_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.periph_reset.q),
      .start_on_sequence_i(~reg2hw.periph_reset.q),
      .switch_ack_i(periph_switch_wait_ack),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(peripheral_subsystem_rst_n)
  );

  power_manager_sequence #(
      .IDLE_VALUE(SWITCH_IDLE_VALUE),
      .ONOFF_AT_RESET(SWITCH_VALUE_AT_RESET)
  ) power_manager_sequence_periph_switch_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.periph_switch.q),
      .start_on_sequence_i(~reg2hw.periph_switch.q),
      .switch_ack_i(1'b1),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(peripheral_subsystem_powergate_switch)
  );

  power_manager_sequence #(
    .IDLE_VALUE(ISO_IDLE_VALUE),
    .ONOFF_AT_RESET(ISO_VALUE_AT_RESET)
  ) power_manager_sequence_periph_iso_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.periph_iso.q),
      .start_on_sequence_i(~reg2hw.periph_iso.q),
      .switch_ack_i(periph_switch_wait_ack),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(peripheral_subsystem_powergate_iso)
  );

% for bank in range(ram_numbanks):
  // --------------------------------------------------------------------------------------
  // RAM_${bank} DOMAIN
  // --------------------------------------------------------------------------------------

  logic ram_${bank}_subsystem_powergate_switch_ack_sync;

  sync #(
      .ResetValue(1'b0)
  ) sync_ram_${bank}_ack_i (
      .clk_i,
      .rst_ni,
      .serial_i(memory_subsystem_banks_powergate_switch_ack_i[${bank}]),
      .serial_o(ram_${bank}_subsystem_powergate_switch_ack_sync)
  );

  assign hw2reg.power_gate_ram_block_${bank}_ack.de = 1'b1;
  assign hw2reg.power_gate_ram_block_${bank}_ack.d = ram_${bank}_subsystem_powergate_switch_ack_sync;

  //if you want to wait for ACK, or just bypass it
  logic ram_${bank}_switch_wait_ack;
  assign ram_${bank}_switch_wait_ack = reg2hw.ram_${bank}_wait_ack_switch_on.q ? reg2hw.power_gate_ram_block_${bank}_ack.q == SWITCH_IDLE_VALUE : 1'b1;

  power_manager_sequence #(
      .IDLE_VALUE(SWITCH_IDLE_VALUE),
      .ONOFF_AT_RESET(SWITCH_VALUE_AT_RESET)
  ) power_manager_sequence_ram_${bank}_switch_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.ram_${bank}_switch.q),
      .start_on_sequence_i (~reg2hw.ram_${bank}_switch.q),
      .switch_ack_i (1'b1),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(memory_subsystem_banks_powergate_switch[${bank}])
  );

  power_manager_sequence #(
    .IDLE_VALUE(ISO_IDLE_VALUE),
    .ONOFF_AT_RESET(ISO_VALUE_AT_RESET)
  ) power_manager_sequence_ram_${bank}_iso_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.ram_${bank}_iso.q),
      .start_on_sequence_i (~reg2hw.ram_${bank}_iso.q),
      .switch_ack_i (ram_${bank}_switch_wait_ack),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(memory_subsystem_banks_powergate_iso[${bank}])
  );

  power_manager_sequence #(
    .IDLE_VALUE(ISO_IDLE_VALUE),
    .ONOFF_AT_RESET(ISO_VALUE_AT_RESET)
  ) power_manager_sequence_ram_${bank}_retentive_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.ram_${bank}_retentive.q),
      .start_on_sequence_i (~reg2hw.ram_${bank}_retentive.q),
      .switch_ack_i (1'b1),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(memory_subsystem_banks_set_retentive_o[${bank}])
  );

% endfor
% for ext in range(external_domains):
  // --------------------------------------------------------------------------------------
  // EXTERNAL_SUBSYSTEM_${ext} DOMAIN
  // --------------------------------------------------------------------------------------

  logic external_${ext}_subsystem_powergate_switch_ack_sync;

  sync #(
      .ResetValue(1'b0)
  ) sync_external_${ext}_ack_i (
      .clk_i,
      .rst_ni,
      .serial_i(external_subsystem_powergate_switch_ack_i[${ext}]),
      .serial_o(external_${ext}_subsystem_powergate_switch_ack_sync)
  );

  assign hw2reg.power_gate_external_${ext}_ack.de = 1'b1;
  assign hw2reg.power_gate_external_${ext}_ack.d = external_${ext}_subsystem_powergate_switch_ack_sync;

  //if you want to wait for ACK, or just bypass it
  logic external_${ext}_switch_wait_ack;
  assign external_${ext}_switch_wait_ack = reg2hw.external_${ext}_wait_ack_switch_on.q ? reg2hw.power_gate_external_${ext}_ack.q == SWITCH_IDLE_VALUE : 1'b1;

  power_manager_sequence #(
      .IDLE_VALUE(RESET_IDLE_VALUE),
      .ONOFF_AT_RESET(RESET_VALUE_AT_RESET)
  ) power_manager_sequence_external_${ext}_reset_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.external_${ext}_reset.q),
      .start_on_sequence_i (~reg2hw.external_${ext}_reset.q),
      .switch_ack_i (external_${ext}_switch_wait_ack),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(external_subsystem_rst_n[${ext}])
  );

  power_manager_sequence #(
      .IDLE_VALUE(SWITCH_IDLE_VALUE),
      .ONOFF_AT_RESET(SWITCH_VALUE_AT_RESET)
  ) power_manager_sequence_external_${ext}_switch_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.external_${ext}_switch.q),
      .start_on_sequence_i (~reg2hw.external_${ext}_switch.q),
      .switch_ack_i (1'b1),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(external_subsystem_powergate_switch[${ext}])
  );

  power_manager_sequence #(
    .IDLE_VALUE(ISO_IDLE_VALUE),
    .ONOFF_AT_RESET(ISO_VALUE_AT_RESET)
  ) power_manager_sequence_external_${ext}_iso_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.external_${ext}_iso.q),
      .start_on_sequence_i (~reg2hw.external_${ext}_iso.q),
      .switch_ack_i (external_${ext}_switch_wait_ack),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(external_subsystem_powergate_iso[${ext}])
  );

  power_manager_sequence #(
    .IDLE_VALUE(ISO_IDLE_VALUE),
    .ONOFF_AT_RESET(ISO_VALUE_AT_RESET)
  ) power_manager_sequence_external_ram_${ext}_retentive_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.external_ram_${ext}_retentive.q),
      .start_on_sequence_i (~reg2hw.external_ram_${ext}_retentive.q),
      .switch_ack_i (1'b1),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(external_ram_banks_set_retentive_o[${ext}])
  );

% endfor
  // --------------------------------------------------------------------------------------
  // MONITOR
  // --------------------------------------------------------------------------------------

  assign hw2reg.monitor_power_gate_core.de = 1'b1;
  assign hw2reg.monitor_power_gate_core.d = {cpu_subsystem_rst_n, cpu_subsystem_powergate_iso, cpu_subsystem_powergate_switch};

  assign hw2reg.monitor_power_gate_periph.de = 1'b1;
  assign hw2reg.monitor_power_gate_periph.d = {peripheral_subsystem_rst_n, peripheral_subsystem_powergate_iso, peripheral_subsystem_powergate_switch};

% for bank in range(ram_numbanks):
  assign hw2reg.monitor_power_gate_ram_block_${bank}.de = 1'b1;
  assign hw2reg.monitor_power_gate_ram_block_${bank}.d = {memory_subsystem_banks_powergate_iso[${bank}], memory_subsystem_banks_powergate_switch[${bank}]};

% endfor
% for ext in range(external_domains):
  assign hw2reg.monitor_power_gate_external_${ext}.de = 1'b1;
  assign hw2reg.monitor_power_gate_external_${ext}.d = {external_subsystem_rst_n[${ext}], external_subsystem_powergate_iso[${ext}], external_subsystem_powergate_switch[${ext}]};

% endfor

endmodule : power_manager
