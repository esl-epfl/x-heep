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

    inout logic clk_i,
    inout logic rst_i,

    //visibility signals
    output logic rst_led,
    output logic clk_led,
    output logic clk_out,

    inout logic boot_select_i,
    inout logic execute_from_flash_i,

    inout logic jtag_tck_i,
    inout logic jtag_tms_i,
    inout logic jtag_trst_ni,
    inout logic jtag_tdi_i,
    inout logic jtag_tdo_o,

    inout logic uart_rx_i,
    inout logic uart_tx_o,

    inout logic [31:0] gpio_io,

    output logic exit_value_o,
    inout  logic exit_valid_o,

    inout logic [3:0] spi_sd_io,
    inout logic spi_csb_o,
    inout logic spi_sck_o,

    inout logic i2c_scl_io,
    inout logic i2c_sda_io

);

  wire                                clk_gen;
  logic [                       31:0] exit_value;
  logic [spi_host_reg_pkg::NumCS-1:0] spi_csb;
  wire                                rst_n;
  logic [ CLK_LED_COUNT_LENGTH - 1:0] clk_count;

  // low active reset
  assign rst_n = !rst_i;

  // reset LED for debugging
  OBUF xilinx_rst_led_i (
      .I(rst_n),
      .O(rst_led)
  );

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
  OBUF xilinx_clk_gen_i (
      .I(clk_gen),
      .O(clk_out)
  );

  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
      .clk_125MHz(clk_i),
      .clk_out1_0(clk_gen)
  );

  core_v_mini_mcu core_v_mini_mcu_i (

      .clk_i (clk_gen),
      .rst_ni(rst_n),

      .jtag_tck_i  (jtag_tck_i),
      .jtag_tms_i  (jtag_tms_i),
      .jtag_trst_ni(jtag_trst_ni),
      .jtag_tdi_i  (jtag_tdi_i),
      .jtag_tdo_o  (jtag_tdo_o),

      .ext_xbar_master_req_i('0),
      .ext_xbar_master_resp_o(),
      .ext_xbar_slave_req_o(),
      .ext_xbar_slave_resp_i('0),
      .ext_peripheral_slave_req_o(),
      .ext_peripheral_slave_resp_i('0),

      .uart_rx_i(uart_rx_i),
      .uart_tx_o(uart_tx_o),

      .intr_vector_ext_i('0),

      .gpio_io(gpio_io),

      .execute_from_flash_i(execute_from_flash_i),
      .boot_select_i(boot_select_i),

      .spi_sd_io(spi_sd_io),
      .spi_csb_o(spi_csb),
      .spi_sck_o(spi_sck_o),

      .exit_value_o(exit_value),
      .exit_valid_o(exit_valid_o),

      .i2c_scl_io,
      .i2c_sda_io
  );

  assign exit_value_o = exit_value[0];
  assign spi_csb_o = spi_csb[0];

endmodule
