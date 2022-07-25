// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module xilinx_core_v_mini_mcu_wrapper
  import obi_pkg::*;
  import reg_pkg::*;
#(
    parameter PULP_XPULP           = 0,
    parameter FPU                  = 0,
    parameter PULP_ZFINX           = 0,
    parameter CLK_LED_COUNT_LENGTH = 27
) (
    input  logic clk_i,
    input  logic rst_i,
    output logic rst_led,
    output logic clk_led,
    output logic clk_out,
    input  logic jtag_tck_i,
    input  logic jtag_tms_i,
    input  logic jtag_trst_ni,
    input  logic jtag_tdi_i,
    output logic jtag_tdo_o,

    input  logic uart_rx_i,
    output logic uart_tx_o,

    inout logic [31:0] gpio_io,

    input  logic fetch_enable_i,
    input  logic boot_select_i,
    output logic exit_value_o,
    output logic exit_valid_o,


    inout logic [3:0] spi_sd_io,
    output logic spi_csb_o,
    output logic spi_sck_o
);

  logic                               clk_gen;
  logic [                       31:0] exit_value;
  logic [spi_host_reg_pkg::NumCS-1:0] spi_csb;
  logic                               rst_ni;
  logic [ CLK_LED_COUNT_LENGTH - 1:0] clk_count;

  // low active reset
  assign rst_ni  = !rst_i;
  // reset LED
  assign rst_led = rst_ni;

  // counter to blink an LED
  assign clk_led = clk_count[CLK_LED_COUNT_LENGTH-1];

  always_ff @(posedge clk_gen or negedge rst_ni) begin : clk_count_process
    if (!rst_ni) begin
      clk_count <= '0;
    end else begin
      clk_count <= clk_count + 1;
    end
  end

  // clock output for debugging
  assign clk_out = clk_gen;

  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
      .clk_125MHz(clk_i),
      .clk_out1_0(clk_gen)
  );

  core_v_mini_mcu core_v_mini_mcu_i (

      .clk_i (clk_gen),
      .rst_ni(rst_ni),

      .jtag_tck_i  (jtag_tck_i),
      .jtag_tms_i  (jtag_tms_i),
      .jtag_trst_ni(rst_ni),
      .jtag_tdi_i  (jtag_tdi_i),
      .jtag_tdo_o  (jtag_tdo_o),

      .ext_xbar_master_req_i(),
      .ext_xbar_master_resp_o(),
      .ext_xbar_slave_req_o(),
      .ext_xbar_slave_resp_i(),
      .ext_peripheral_slave_req_o(),
      .ext_peripheral_slave_resp_i(),

      .uart_rx_i(uart_rx_i),
      .uart_tx_o(uart_tx_o),

      .intr_vector_ext_i(),

      .gpio_io(gpio_io),

      .fetch_enable_i(fetch_enable_i),
      .boot_select_i (boot_select_i),

      .spi_sd_io(spi_sd_io),
      .spi_csb_o(spi_csb),
      .spi_sck_o(spi_sck_o),

      .exit_value_o(exit_value),
      .exit_valid_o(exit_valid_o)
  );

  assign exit_value_o = exit_value[0];
  assign spi_csb_o = spi_csb[0];

endmodule
