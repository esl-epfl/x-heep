// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module femu
  import obi_pkg::*;
  import reg_pkg::*;
#(
    parameter PULP_XPULP           = 0,
    parameter FPU                  = 0,
    parameter PULP_ZFINX           = 0,
    parameter EXT_XBAR_NMASTER = 0,
    parameter CLK_LED_COUNT_LENGTH = 27
) (
    inout logic clk_in,
    inout logic rst_i,

    //visibility signals
    output logic rst_led,
    output logic clk_led,
    output logic clk_out,

    inout logic boot_select_i,
    inout logic execute_from_flash_i,

    inout logic [29:0] gpio_io,

    output logic exit_value_o,
    inout  logic exit_valid_o,

    //inout logic [3:0] spi_sd_io,
    inout logic spi_sd_0_io,
    inout logic spi_sd_1_io,
    inout logic spi_sd_2_io,
    inout logic spi_sd_3_io,
    inout logic spi_csb_o,
    inout logic spi_sck_o,

    inout logic spi2_sd_0_io,
    inout logic spi2_sd_1_io,
    inout logic spi2_sd_2_io,
    inout logic spi2_sd_3_io,
    inout logic [1:0] spi2_csb_o,
    inout logic spi2_sck_o,

    inout logic i2c_scl_io,
    inout logic i2c_sda_io

);

  import core_v_mini_mcu_pkg::*;

  // PM signals
  logic cpu_subsystem_powergate_switch;
  logic cpu_subsystem_powergate_switch_ack;
  logic cpu_subsystem_powergate_iso;
  logic cpu_subsystem_rst_n;
  logic peripheral_subsystem_powergate_switch;
  logic peripheral_subsystem_powergate_switch_ack;
  logic peripheral_subsystem_powergate_iso;
  logic peripheral_subsystem_rst_n;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_switch_ack;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_powergate_iso;
  logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] memory_subsystem_banks_set_retentive;

  // PAD controller
  reg_req_t pad_req;
  reg_rsp_t pad_resp;
  logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][7:0] pad_attributes;
  logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][3:0] pad_muxes;

  logic rst_ngen;

  //input, output pins from core_v_mini_mcu
% for pad in total_pad_list:
${pad.internal_signals}
% endfor
  
  wire                               clk_gen;
  logic [                      31:0] exit_value;
  wire                               rst_n;
  logic [CLK_LED_COUNT_LENGTH - 1:0] clk_count;

  // low active reset
  assign rst_n   = !rst_i;

  // reset LED for debugging
  assign rst_led = rst_n;

  // counter to blink an LED
  assign clk_led = clk_count[CLK_LED_COUNT_LENGTH-1];

  always_ff @(posedge clk_gen or negedge rst_n) begin : clk_count_process
    if (!rst_n) begin
      clk_count <= '0;
    end else begin
      clk_count <= clk_count + 1;
    end
  end

  // clock output for debugging
  assign clk_out = clk_gen;

  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
      .clk_125MHz(clk_in),
      .clk_out1_0(clk_gen)
  );

  logic clk_i;
  assign clk_i = clk_gen;
  
  core_v_mini_mcu #(
    .PULP_XPULP(PULP_XPULP),
    .FPU(FPU),
    .PULP_ZFINX(PULP_ZFINX),
    .EXT_XBAR_NMASTER(EXT_XBAR_NMASTER)
  ) core_v_mini_mcu_i (

    .rst_ni(rst_ngen),
% for pad in pad_list:
${pad.core_v_mini_mcu_bonding}
% endfor
    .intr_vector_ext_i('0),
    .ext_xbar_master_req_i('0),
    .ext_xbar_master_resp_o(),
    .ext_xbar_slave_req_o(),
    .ext_xbar_slave_resp_i('0),
    .ext_peripheral_slave_req_o(),
    .ext_peripheral_slave_resp_i('0),
    .external_subsystem_powergate_switch_o(),
    .external_subsystem_powergate_switch_ack_i(),
    .external_subsystem_powergate_iso_o(),
    .external_subsystem_rst_no(),
    .external_ram_banks_set_retentive_o(),
    .exit_value_o(exit_value),
    .pad_req_o(pad_req),
    .pad_resp_i(pad_resp),
    .cpu_subsystem_powergate_switch_o(cpu_subsystem_powergate_switch),
    .cpu_subsystem_powergate_switch_ack_i(cpu_subsystem_powergate_switch_ack),
    .peripheral_subsystem_powergate_switch_o(peripheral_subsystem_powergate_switch),
    .peripheral_subsystem_powergate_switch_ack_i(peripheral_subsystem_powergate_switch_ack),
    .memory_subsystem_banks_powergate_switch_o(memory_subsystem_banks_powergate_switch),
    .memory_subsystem_banks_powergate_switch_ack_i(memory_subsystem_banks_powergate_switch_ack)
  );

  pad_ring pad_ring_i (
% for pad in total_pad_list:
${pad.pad_ring_bonding_bonding}
% endfor
    .pad_attributes_i(pad_attributes)
  );

${pad_constant_driver_assign}

${pad_mux_process}

  pad_control #(
      .reg_req_t(reg_pkg::reg_req_t),
      .reg_rsp_t(reg_pkg::reg_rsp_t),
      .NUM_PAD  (core_v_mini_mcu_pkg::NUM_PAD)
  ) pad_control_i (
      .clk_i(clk_in_x),
      .rst_ni(rst_ngen),
      .reg_req_i(pad_req),
      .reg_rsp_o(pad_resp),
      .pad_attributes_o(pad_attributes),
      .pad_muxes_o(pad_muxes)
  );

  rstgen rstgen_i (
    .clk_i(clk_in_x),
    .rst_ni(rst_n),
    .test_mode_i(1'b0),
    .rst_no(rst_ngen),
    .init_no()
  );


  assign exit_value_o = exit_value[0];
endmodule  // x_heep_system
