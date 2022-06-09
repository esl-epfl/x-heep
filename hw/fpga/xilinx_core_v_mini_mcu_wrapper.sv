// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module xilinx_core_v_mini_mcu_wrapper #(
    parameter PULP_XPULP = 0,
    parameter FPU        = 0,
    parameter PULP_ZFINX = 0
) (
    input  logic clk_i,
    input  logic rst_ni,

    input  logic jtag_tck_i,
    input  logic jtag_tms_i,
    input  logic jtag_trst_ni,
    input  logic jtag_tdi_i,
    output logic jtag_tdo_o,

    input  logic uart_rx_i,
    output logic uart_tx_o,

    input  logic fetch_enable_i,
    output logic exit_value_o,
    output logic exit_valid_o
);

  logic             clk_gen;
  logic      [31:0] exit_value;


  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
      .clk_125MHz (clk_i),
      .clk_out1_0 (clk_gen)
  );

  core_v_mini_mcu core_v_mini_mcu_i (
      .clk_i          (clk_gen),
      .rst_ni         (rst_ni),
      .jtag_tck_i     (jtag_tck_i),
      .jtag_tms_i     (jtag_tms_i),
      .jtag_trst_ni   (jtag_trst_ni),
      .jtag_tdi_i     (jtag_tdi_i),
      .jtag_tdo_o     (jtag_tdo_o),
      .uart_rx_i      (uart_rx_i),
      .uart_tx_o      (uart_tx_o),
      .fetch_enable_i (fetch_enable_i),
      .exit_value_o   (exit_value),
      .exit_valid_o   (exit_valid_o)
  );

  assign exit_value_o = exit_value[0];


endmodule
