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

    // Power gating signals
    output logic                                             cpu_subsystem_powergate_switch_o,
    input  logic                                             cpu_subsystem_powergate_switch_ack_i,
    output logic                                             cpu_subsystem_powergate_iso_o,
    output logic                                             cpu_subsystem_rst_no,
    output logic                                             peripheral_subsystem_powergate_switch_o,
    input  logic                                             peripheral_subsystem_powergate_switch_ack_i,
    output logic                                             peripheral_subsystem_powergate_iso_o,
    output logic                                             peripheral_subsystem_rst_no,
    output logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0]        memory_subsystem_banks_powergate_switch_o,
    input  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0]        memory_subsystem_banks_powergate_switch_ack_i,
    output logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0]        memory_subsystem_banks_powergate_iso_o,
    output logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0]        memory_subsystem_banks_set_retentive_o,
    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_switch_o,
    input  logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_switch_ack_i,
    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_powergate_iso_o,
    output logic [core_v_mini_mcu_pkg::EXTERNAL_DOMAINS-1:0] external_subsystem_rst_no
);

  import power_manager_reg_pkg::*;

  power_manager_reg2hw_t reg2hw;
  power_manager_hw2reg_t hw2reg;

  logic start_on_sequence;

  assign hw2reg.intr_state.d = {
    1'b0,
    ext_irq_i,
    intr_i[29:22], // gpio
    intr_i[21], // spi_flash
    intr_i[20], // spi
    intr_i[19], // dma
    intr_i[18], // rv_timer_3
    intr_i[17], // rv_timer_2
    intr_i[16], // rv_timer_1
    intr_i[11], // plic
    intr_i[7] // rv_timer_0
  };

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

  // --------------------------------------------------------------------------------------
  // CPU_SUBSYSTEM DOMAIN
  // --------------------------------------------------------------------------------------

  logic cpu_reset_counter_start_switch_off, cpu_reset_counter_expired_switch_off;
  logic cpu_reset_counter_start_switch_on, cpu_reset_counter_expired_switch_on;

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

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_cpu_reset_assert_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.cpu_counters_stop.cpu_reset_assert_stop_bit_counter.q),
      .start_i(cpu_reset_counter_start_switch_off),
      .done_o(cpu_reset_counter_expired_switch_off),
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
      .start_i(cpu_reset_counter_start_switch_on),
      .done_o(cpu_reset_counter_expired_switch_on),
      .hw2reg_d_o(hw2reg.cpu_reset_deassert_counter.d),
      .hw2reg_de_o(hw2reg.cpu_reset_deassert_counter.de),
      .hw2reg_q_i(reg2hw.cpu_reset_deassert_counter.q)
  );

  always_comb begin : power_manager_start_on_sequence_gen
    if ((reg2hw.en_wait_for_intr.q & reg2hw.intr_state.q) == 32'b0) begin
      start_on_sequence = 1'b0;
    end else begin
      start_on_sequence = 1'b1;
    end
  end

  power_manager_counter_sequence #(
      .IDLE_VALUE(RESET_IDLE_VALUE),
      .ONOFF_AT_RESET(RESET_VALUE_AT_RESET)
  ) power_manager_counter_sequence_cpu_reset_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.power_gate_core.q && core_sleep_i),
      .start_on_sequence_i (start_on_sequence),
      .switch_ack_i (cpu_switch_wait_ack),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(cpu_reset_counter_expired_switch_off),
      .counter_expired_switch_on_i (cpu_reset_counter_expired_switch_on),

      .counter_start_switch_off_o(cpu_reset_counter_start_switch_off),
      .counter_start_switch_on_o (cpu_reset_counter_start_switch_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(cpu_subsystem_rst_no)
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
      .start_off_sequence_i(reg2hw.power_gate_core.q && core_sleep_i),
      .start_on_sequence_i (start_on_sequence),
      .switch_ack_i (1'b1),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(cpu_powergate_counter_expired_switch_off),
      .counter_expired_switch_on_i (cpu_powergate_counter_expired_switch_on),

      .counter_start_switch_off_o(cpu_powergate_counter_start_switch_off),
      .counter_start_switch_on_o (cpu_powergate_counter_start_switch_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(cpu_subsystem_powergate_switch_o)
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
      .start_off_sequence_i(reg2hw.power_gate_core.q && core_sleep_i),
      .start_on_sequence_i (start_on_sequence),
      .switch_ack_i (cpu_switch_wait_ack),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(cpu_powergate_counter_expired_iso_off),
      .counter_expired_switch_on_i (cpu_powergate_counter_expired_iso_on),

      .counter_start_switch_off_o(cpu_powergate_counter_start_iso_off),
      .counter_start_switch_on_o (cpu_powergate_counter_start_iso_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(cpu_subsystem_powergate_iso_o)
  );

  // --------------------------------------------------------------------------------------
  // PERIPHERAL_SUBSYSTEM DOMAIN
  // --------------------------------------------------------------------------------------

  logic periph_reset_counter_start_switch_off, periph_reset_counter_expired_switch_off;
  logic periph_reset_counter_start_switch_on, periph_reset_counter_expired_switch_on;

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
  assign periph_switch_wait_ack = reg2hw.periph_wait_ack_switch_on_counter.q ? reg2hw.power_gate_periph_ack.q == SWITCH_IDLE_VALUE : 1'b1;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_periph_reset_assert_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.periph_counters_stop.periph_reset_assert_stop_bit_counter.q),
      .start_i(periph_reset_counter_start_switch_off),
      .done_o(periph_reset_counter_expired_switch_off),
      .hw2reg_d_o(hw2reg.periph_reset_assert_counter.d),
      .hw2reg_de_o(hw2reg.periph_reset_assert_counter.de),
      .hw2reg_q_i(reg2hw.periph_reset_assert_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_periph_reset_deassert_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.periph_counters_stop.periph_reset_deassert_stop_bit_counter.q),
      .start_i(periph_reset_counter_start_switch_on),
      .done_o(periph_reset_counter_expired_switch_on),
      .hw2reg_d_o(hw2reg.periph_reset_deassert_counter.d),
      .hw2reg_de_o(hw2reg.periph_reset_deassert_counter.de),
      .hw2reg_q_i(reg2hw.periph_reset_deassert_counter.q)
  );

  power_manager_counter_sequence #(
      .IDLE_VALUE(RESET_IDLE_VALUE),
      .ONOFF_AT_RESET(RESET_VALUE_AT_RESET)
  ) power_manager_counter_sequence_periph_reset_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.power_gate_periph.q),
      .start_on_sequence_i (~reg2hw.power_gate_periph.q),
      .switch_ack_i (periph_switch_wait_ack),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(periph_reset_counter_expired_switch_off),
      .counter_expired_switch_on_i (periph_reset_counter_expired_switch_on),

      .counter_start_switch_off_o(periph_reset_counter_start_switch_off),
      .counter_start_switch_on_o (periph_reset_counter_start_switch_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(peripheral_subsystem_rst_no)
  );


  logic periph_powergate_counter_start_switch_off, periph_powergate_counter_expired_switch_off;
  logic periph_powergate_counter_start_switch_on, periph_powergate_counter_expired_switch_on;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_periph_powergate_switch_off_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.periph_counters_stop.periph_switch_off_stop_bit_counter.q),
      .start_i(periph_powergate_counter_start_switch_off),
      .done_o(periph_powergate_counter_expired_switch_off),
      .hw2reg_d_o(hw2reg.periph_switch_off_counter.d),
      .hw2reg_de_o(hw2reg.periph_switch_off_counter.de),
      .hw2reg_q_i(reg2hw.periph_switch_off_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_periph_powergate_switch_on_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.periph_counters_stop.periph_switch_on_stop_bit_counter.q),
      .start_i(periph_powergate_counter_start_switch_on),
      .done_o(periph_powergate_counter_expired_switch_on),
      .hw2reg_d_o(hw2reg.periph_switch_on_counter.d),
      .hw2reg_de_o(hw2reg.periph_switch_on_counter.de),
      .hw2reg_q_i(reg2hw.periph_switch_on_counter.q)
  );

  power_manager_counter_sequence #(
      .IDLE_VALUE(SWITCH_IDLE_VALUE),
      .ONOFF_AT_RESET(SWITCH_VALUE_AT_RESET)
  ) power_manager_counter_sequence_periph_switch_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.power_gate_periph.q),
      .start_on_sequence_i (~reg2hw.power_gate_periph.q),
      .switch_ack_i (1'b1),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(periph_powergate_counter_expired_switch_off),
      .counter_expired_switch_on_i (periph_powergate_counter_expired_switch_on),

      .counter_start_switch_off_o(periph_powergate_counter_start_switch_off),
      .counter_start_switch_on_o (periph_powergate_counter_start_switch_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(peripheral_subsystem_powergate_switch_o)
  );

  logic periph_powergate_counter_start_iso_off, periph_powergate_counter_expired_iso_off;
  logic periph_powergate_counter_start_iso_on, periph_powergate_counter_expired_iso_on;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_periph_powergate_iso_off_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.periph_counters_stop.periph_iso_off_stop_bit_counter.q),
      .start_i(periph_powergate_counter_start_iso_off),
      .done_o(periph_powergate_counter_expired_iso_off),
      .hw2reg_d_o(hw2reg.periph_iso_off_counter.d),
      .hw2reg_de_o(hw2reg.periph_iso_off_counter.de),
      .hw2reg_q_i(reg2hw.periph_iso_off_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_periph_powergate_iso_on_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.periph_counters_stop.periph_iso_on_stop_bit_counter.q),
      .start_i(periph_powergate_counter_start_iso_on),
      .done_o(periph_powergate_counter_expired_iso_on),
      .hw2reg_d_o(hw2reg.periph_iso_on_counter.d),
      .hw2reg_de_o(hw2reg.periph_iso_on_counter.de),
      .hw2reg_q_i(reg2hw.periph_iso_on_counter.q)
  );

  power_manager_counter_sequence #(
    .IDLE_VALUE(ISO_IDLE_VALUE),
    .ONOFF_AT_RESET(ISO_VALUE_AT_RESET)
  ) power_manager_counter_sequence_periph_iso_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.power_gate_periph.q),
      .start_on_sequence_i (~reg2hw.power_gate_periph.q),
      .switch_ack_i (periph_switch_wait_ack),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(periph_powergate_counter_expired_iso_off),
      .counter_expired_switch_on_i (periph_powergate_counter_expired_iso_on),

      .counter_start_switch_off_o(periph_powergate_counter_start_iso_off),
      .counter_start_switch_on_o (periph_powergate_counter_start_iso_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(peripheral_subsystem_powergate_iso_o)
  );

% for bank in range(ram_numbanks):
  // --------------------------------------------------------------------------------------
  // RAM_${bank} DOMAIN
  // --------------------------------------------------------------------------------------

  logic ram_${bank}_powergate_counter_start_switch_off, ram_${bank}_powergate_counter_expired_switch_off;
  logic ram_${bank}_powergate_counter_start_switch_on, ram_${bank}_powergate_counter_expired_switch_on;

  logic ram_${bank}_subsystem_powergate_switch_ack_sync;

  sync #(
      .ResetValue(1'b0)
  ) sync_ram${bank}_ack_i (
      .clk_i,
      .rst_ni,
      .serial_i(memory_subsystem_banks_powergate_switch_ack_i[${bank}]),
      .serial_o(ram_${bank}_subsystem_powergate_switch_ack_sync)
  );

  assign hw2reg.power_gate_ram_block_${bank}_ack.de = 1'b1;
  assign hw2reg.power_gate_ram_block_${bank}_ack.d = ram_${bank}_subsystem_powergate_switch_ack_sync;

  //if you want to wait for ACK, or just bypass it
  logic ram_${bank}_switch_wait_ack;
  assign ram_${bank}_switch_wait_ack = reg2hw.ram_${bank}_wait_ack_switch_on_counter.q ? reg2hw.power_gate_ram_block_${bank}_ack.q == SWITCH_IDLE_VALUE : 1'b1;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_ram_${bank}_powergate_switch_off_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.ram_${bank}_counters_stop.ram_${bank}_switch_off_stop_bit_counter.q),
      .start_i(ram_${bank}_powergate_counter_start_switch_off),
      .done_o(ram_${bank}_powergate_counter_expired_switch_off),
      .hw2reg_d_o(hw2reg.ram_${bank}_switch_off_counter.d),
      .hw2reg_de_o(hw2reg.ram_${bank}_switch_off_counter.de),
      .hw2reg_q_i(reg2hw.ram_${bank}_switch_off_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_ram_${bank}_powergate_switch_on_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.ram_${bank}_counters_stop.ram_${bank}_switch_on_stop_bit_counter.q),
      .start_i(ram_${bank}_powergate_counter_start_switch_on),
      .done_o(ram_${bank}_powergate_counter_expired_switch_on),
      .hw2reg_d_o(hw2reg.ram_${bank}_switch_on_counter.d),
      .hw2reg_de_o(hw2reg.ram_${bank}_switch_on_counter.de),
      .hw2reg_q_i(reg2hw.ram_${bank}_switch_on_counter.q)
  );

  power_manager_counter_sequence #(
      .IDLE_VALUE(SWITCH_IDLE_VALUE),
      .ONOFF_AT_RESET(SWITCH_VALUE_AT_RESET)
  ) power_manager_counter_sequence_ram_${bank}_switch_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.power_gate_ram_block_${bank}.q),
      .start_on_sequence_i (~reg2hw.power_gate_ram_block_${bank}.q),
      .switch_ack_i (1'b1),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(ram_${bank}_powergate_counter_expired_switch_off),
      .counter_expired_switch_on_i (ram_${bank}_powergate_counter_expired_switch_on),

      .counter_start_switch_off_o(ram_${bank}_powergate_counter_start_switch_off),
      .counter_start_switch_on_o (ram_${bank}_powergate_counter_start_switch_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(memory_subsystem_banks_powergate_switch_o[${bank}])
  );

  logic ram_${bank}_powergate_counter_start_iso_off, ram_${bank}_powergate_counter_expired_iso_off;
  logic ram_${bank}_powergate_counter_start_iso_on, ram_${bank}_powergate_counter_expired_iso_on;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_ram_${bank}_powergate_iso_off_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.ram_${bank}_counters_stop.ram_${bank}_iso_off_stop_bit_counter.q),
      .start_i(ram_${bank}_powergate_counter_start_iso_off),
      .done_o(ram_${bank}_powergate_counter_expired_iso_off),
      .hw2reg_d_o(hw2reg.ram_${bank}_iso_off_counter.d),
      .hw2reg_de_o(hw2reg.ram_${bank}_iso_off_counter.de),
      .hw2reg_q_i(reg2hw.ram_${bank}_iso_off_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_ram_${bank}_powergate_iso_on_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.ram_${bank}_counters_stop.ram_${bank}_iso_on_stop_bit_counter.q),
      .start_i(ram_${bank}_powergate_counter_start_iso_on),
      .done_o(ram_${bank}_powergate_counter_expired_iso_on),
      .hw2reg_d_o(hw2reg.ram_${bank}_iso_on_counter.d),
      .hw2reg_de_o(hw2reg.ram_${bank}_iso_on_counter.de),
      .hw2reg_q_i(reg2hw.ram_${bank}_iso_on_counter.q)
  );

  power_manager_counter_sequence #(
    .IDLE_VALUE(ISO_IDLE_VALUE),
    .ONOFF_AT_RESET(ISO_VALUE_AT_RESET)
  ) power_manager_counter_sequence_ram_${bank}_iso_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.power_gate_ram_block_${bank}.q),
      .start_on_sequence_i (~reg2hw.power_gate_ram_block_${bank}.q),
      .switch_ack_i (ram_${bank}_switch_wait_ack),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(ram_${bank}_powergate_counter_expired_iso_off),
      .counter_expired_switch_on_i (ram_${bank}_powergate_counter_expired_iso_on),

      .counter_start_switch_off_o(ram_${bank}_powergate_counter_start_iso_off),
      .counter_start_switch_on_o (ram_${bank}_powergate_counter_start_iso_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(memory_subsystem_banks_powergate_iso_o[${bank}])
  );

  logic ram_${bank}_retentive_counter_start_off, ram_${bank}_retentive_counter_expired_off;
  logic ram_${bank}_retentive_counter_start_on, ram_${bank}_retentive_counter_expired_on;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_ram_${bank}_retentive_off_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.ram_${bank}_counters_stop.ram_${bank}_retentive_off_stop_bit_counter.q),
      .start_i(ram_${bank}_retentive_counter_start_off),
      .done_o(ram_${bank}_retentive_counter_expired_off),
      .hw2reg_d_o(hw2reg.ram_${bank}_retentive_off_counter.d),
      .hw2reg_de_o(hw2reg.ram_${bank}_retentive_off_counter.de),
      .hw2reg_q_i(reg2hw.ram_${bank}_retentive_off_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_ram_${bank}_retentive_on_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.ram_${bank}_counters_stop.ram_${bank}_retentive_on_stop_bit_counter.q),
      .start_i(ram_${bank}_retentive_counter_start_on),
      .done_o(ram_${bank}_retentive_counter_expired_on),
      .hw2reg_d_o(hw2reg.ram_${bank}_retentive_on_counter.d),
      .hw2reg_de_o(hw2reg.ram_${bank}_retentive_on_counter.de),
      .hw2reg_q_i(reg2hw.ram_${bank}_retentive_on_counter.q)
  );

  power_manager_counter_sequence power_manager_counter_sequence_ram_${bank}_retentive_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.set_retentive_ram_block_${bank}.q),
      .start_on_sequence_i (~reg2hw.set_retentive_ram_block_${bank}.q),
      .switch_ack_i (1'b1),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(ram_${bank}_retentive_counter_expired_off),
      .counter_expired_switch_on_i (ram_${bank}_retentive_counter_expired_on),

      .counter_start_switch_off_o(ram_${bank}_retentive_counter_start_off),
      .counter_start_switch_on_o (ram_${bank}_retentive_counter_start_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(memory_subsystem_banks_set_retentive_o[${bank}])
  );

% endfor
% for ext in range(external_domains):
  // --------------------------------------------------------------------------------------
  // EXTERNAL_SUBSYSTEM_${ext} DOMAIN
  // --------------------------------------------------------------------------------------

  logic external_${ext}_reset_counter_start_switch_off, external_${ext}_reset_counter_expired_switch_off;
  logic external_${ext}_reset_counter_start_switch_on, external_${ext}_reset_counter_expired_switch_on;

  assign hw2reg.power_gate_external_${ext}_ack.de = 1'b1;
  assign hw2reg.power_gate_external_${ext}_ack.d = external_subsystem_powergate_switch_ack_i[${ext}];

  //if you want to wait for ACK, or just bypass it
  logic external_${ext}_switch_wait_ack;
  assign external_${ext}_switch_wait_ack = reg2hw.external_${ext}_wait_ack_switch_on_counter.q ? reg2hw.power_gate_external_${ext}_ack.q == SWITCH_IDLE_VALUE : 1'b1;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_external_${ext}_reset_assert_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.external_${ext}_counters_stop.external_${ext}_reset_assert_stop_bit_counter.q),
      .start_i(external_${ext}_reset_counter_start_switch_off),
      .done_o(external_${ext}_reset_counter_expired_switch_off),
      .hw2reg_d_o(hw2reg.external_${ext}_reset_assert_counter.d),
      .hw2reg_de_o(hw2reg.external_${ext}_reset_assert_counter.de),
      .hw2reg_q_i(reg2hw.external_${ext}_reset_assert_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_external_${ext}_reset_deassert_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.external_${ext}_counters_stop.external_${ext}_reset_deassert_stop_bit_counter.q),
      .start_i(external_${ext}_reset_counter_start_switch_on),
      .done_o(external_${ext}_reset_counter_expired_switch_on),
      .hw2reg_d_o(hw2reg.external_${ext}_reset_deassert_counter.d),
      .hw2reg_de_o(hw2reg.external_${ext}_reset_deassert_counter.de),
      .hw2reg_q_i(reg2hw.external_${ext}_reset_deassert_counter.q)
  );

  power_manager_counter_sequence #(
      .IDLE_VALUE(RESET_IDLE_VALUE),
      .ONOFF_AT_RESET(RESET_VALUE_AT_RESET)
  ) power_manager_counter_sequence_external_${ext}_reset_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.power_gate_external_${ext}.q),
      .start_on_sequence_i (~reg2hw.power_gate_external_${ext}.q),
      .switch_ack_i (external_${ext}_switch_wait_ack),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(external_${ext}_reset_counter_expired_switch_off),
      .counter_expired_switch_on_i (external_${ext}_reset_counter_expired_switch_on),

      .counter_start_switch_off_o(external_${ext}_reset_counter_start_switch_off),
      .counter_start_switch_on_o (external_${ext}_reset_counter_start_switch_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(external_subsystem_rst_no[${ext}])
  );

  logic external_${ext}_powergate_counter_start_switch_off, external_${ext}_powergate_counter_expired_switch_off;
  logic external_${ext}_powergate_counter_start_switch_on, external_${ext}_powergate_counter_expired_switch_on;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_external_${ext}_powergate_switch_off_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.external_${ext}_counters_stop.external_${ext}_switch_off_stop_bit_counter.q),
      .start_i(external_${ext}_powergate_counter_start_switch_off),
      .done_o(external_${ext}_powergate_counter_expired_switch_off),
      .hw2reg_d_o(hw2reg.external_${ext}_switch_off_counter.d),
      .hw2reg_de_o(hw2reg.external_${ext}_switch_off_counter.de),
      .hw2reg_q_i(reg2hw.external_${ext}_switch_off_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_external_${ext}_powergate_switch_on_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.external_${ext}_counters_stop.external_${ext}_switch_on_stop_bit_counter.q),
      .start_i(external_${ext}_powergate_counter_start_switch_on),
      .done_o(external_${ext}_powergate_counter_expired_switch_on),
      .hw2reg_d_o(hw2reg.external_${ext}_switch_on_counter.d),
      .hw2reg_de_o(hw2reg.external_${ext}_switch_on_counter.de),
      .hw2reg_q_i(reg2hw.external_${ext}_switch_on_counter.q)
  );

  power_manager_counter_sequence #(
      .IDLE_VALUE(SWITCH_IDLE_VALUE),
      .ONOFF_AT_RESET(SWITCH_VALUE_AT_RESET)
  ) power_manager_counter_sequence_external_${ext}_switch_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.power_gate_external_${ext}.q),
      .start_on_sequence_i (~reg2hw.power_gate_external_${ext}.q),
      .switch_ack_i (1'b1),


      // counter to switch on and off signals
      .counter_expired_switch_off_i(external_${ext}_powergate_counter_expired_switch_off),
      .counter_expired_switch_on_i (external_${ext}_powergate_counter_expired_switch_on),

      .counter_start_switch_off_o(external_${ext}_powergate_counter_start_switch_off),
      .counter_start_switch_on_o (external_${ext}_powergate_counter_start_switch_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(external_subsystem_powergate_switch_o[${ext}])
  );

  logic external_${ext}_powergate_counter_start_iso_off, external_${ext}_powergate_counter_expired_iso_off;
  logic external_${ext}_powergate_counter_start_iso_on, external_${ext}_powergate_counter_expired_iso_on;

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_external_${ext}_powergate_iso_off_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.external_${ext}_counters_stop.external_${ext}_iso_off_stop_bit_counter.q),
      .start_i(external_${ext}_powergate_counter_start_iso_off),
      .done_o(external_${ext}_powergate_counter_expired_iso_off),
      .hw2reg_d_o(hw2reg.external_${ext}_iso_off_counter.d),
      .hw2reg_de_o(hw2reg.external_${ext}_iso_off_counter.de),
      .hw2reg_q_i(reg2hw.external_${ext}_iso_off_counter.q)
  );

  reg_to_counter #(
      .DW(32),
      .ExpireValue('0)
  ) reg_to_counter_external_${ext}_powergate_iso_on_i (
      .clk_i,
      .rst_ni,
      .stop_i(reg2hw.external_${ext}_counters_stop.external_${ext}_iso_on_stop_bit_counter.q),
      .start_i(external_${ext}_powergate_counter_start_iso_on),
      .done_o(external_${ext}_powergate_counter_expired_iso_on),
      .hw2reg_d_o(hw2reg.external_${ext}_iso_on_counter.d),
      .hw2reg_de_o(hw2reg.external_${ext}_iso_on_counter.de),
      .hw2reg_q_i(reg2hw.external_${ext}_iso_on_counter.q)
  );

  power_manager_counter_sequence #(
    .IDLE_VALUE(ISO_IDLE_VALUE),
    .ONOFF_AT_RESET(ISO_VALUE_AT_RESET)
  ) power_manager_counter_sequence_external_${ext}_iso_i (
      .clk_i,
      .rst_ni,

      // trigger to start the sequence
      .start_off_sequence_i(reg2hw.power_gate_external_${ext}.q),
      .start_on_sequence_i (~reg2hw.power_gate_external_${ext}.q),
      .switch_ack_i (external_${ext}_switch_wait_ack),

      // counter to switch on and off signals
      .counter_expired_switch_off_i(external_${ext}_powergate_counter_expired_iso_off),
      .counter_expired_switch_on_i (external_${ext}_powergate_counter_expired_iso_on),

      .counter_start_switch_off_o(external_${ext}_powergate_counter_start_iso_off),
      .counter_start_switch_on_o (external_${ext}_powergate_counter_start_iso_on),

      // switch on and off signal, 1 means on
      .switch_onoff_signal_o(external_subsystem_powergate_iso_o[${ext}])
  );

% endfor

endmodule : power_manager
