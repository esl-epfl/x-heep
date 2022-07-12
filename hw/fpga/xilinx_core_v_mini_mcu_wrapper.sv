// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module xilinx_core_v_mini_mcu_wrapper
  import obi_pkg::*;
  import reg_pkg::*;
#(
    parameter EXT_XBAR_NMASTER = 0,
    parameter EXT_NINTERRUPT   = 0
) (
    input logic clk_i,
    input logic rst_ni,

    input  logic jtag_tck_i,
    input  logic jtag_tms_i,
    input  logic jtag_trst_ni,
    input  logic jtag_tdi_i,
    output logic jtag_tdo_o,

    input  logic uart_rx_i,
    output logic uart_tx_o,

    input  logic gpio_i,
    output logic gpio_o,

    input  logic fetch_enable_i,
    output logic exit_value_o,
    output logic exit_valid_o
);

  logic        clk_gen;
  logic [31:0] exit_value;
  logic [31:0] gpio_in;
  logic [31:0] gpio_out;

  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
      .clk_125MHz(clk_i),
      .clk_out1_0(clk_gen)
  );

  core_v_mini_mcu core_v_mini_mcu_i (
      .clk_i(clk_gen),
      .rst_ni(rst_ni),
      .jtag_tck_i(jtag_tck_i),
      .jtag_tms_i(jtag_tms_i),
      .jtag_trst_ni(jtag_trst_ni),
      .jtag_tdi_i(jtag_tdi_i),
      .jtag_tdo_o(jtag_tdo_o),
      .ext_xbar_master_req_i(),
      .ext_xbar_master_resp_o(),
      .ext_xbar_slave_req_o(),
      .ext_xbar_slave_resp_i(),
      .ext_peripheral_slave_req_o(),
      .ext_peripheral_slave_resp_i(),
      .uart_rx_i(uart_rx_i),
      .uart_tx_o(uart_tx_o),
      .intr_vector_ext_i(),
      .gpio_i(gpio_in),
      .gpio_o(gpio_out),
      .gpio_en_o(),
      .fetch_enable_i(fetch_enable_i),
      .exit_value_o(exit_value),
      .exit_valid_o(exit_valid_o)
  );

  assign exit_value_o = exit_value[0];
  assign gpio_in[0] = gpio_i;
  assign gpio_in[31:1] = '0;
  assign gpio_o = gpio_out[0];

endmodule
