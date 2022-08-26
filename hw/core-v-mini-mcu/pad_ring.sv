// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_ring
  import spi_host_reg_pkg::*;
(
    inout logic clk_i,
    inout logic rst_ni,

    inout logic boot_select_i,
    inout logic execute_from_flash_i,

    inout logic jtag_tck_i,
    inout logic jtag_tms_i,
    inout logic jtag_trst_ni,
    inout logic jtag_tdi_i,
    inout logic jtag_tdo_o,

    inout logic uart_rx_i,
    inout logic uart_tx_o,

    inout logic exit_valid_o,

    inout logic [31:0] gpio_io,

    inout logic [3:0] spi_sd_io,
    inout logic [spi_host_reg_pkg::NumCS-1:0] spi_csb_o,
    inout logic spi_sck_o,

    inout logic i2c_scl_io,
    inout logic i2c_sda_io,

    output logic clk_o,
    output logic rst_no,

    output logic boot_select_o,
    output logic execute_from_flash_o,

    output logic jtag_tck_o,
    output logic jtag_tms_o,
    output logic jtag_trst_no,
    output logic jtag_tdi_o,
    input  logic jtag_tdo_i,

    output logic uart_rx_o,
    input  logic uart_tx_i,

    input logic exit_valid_i,

    input  logic [31:0] gpio_out_i,
    input  logic [31:0] gpio_oe_i,
    output logic [31:0] gpio_in_o,

    input logic spi_sck_i,
    input logic spi_sck_en_i,

    input logic [spi_host_reg_pkg::NumCS-1:0] spi_csb_i,
    input logic [spi_host_reg_pkg::NumCS-1:0] spi_csb_oe_i,

    input  logic [3:0] spi_sd_i,
    input  logic [3:0] spi_sd_oe_i,
    output logic [3:0] spi_sd_o,


    input logic i2c_scl_i,
    input logic i2c_sda_i,

    input logic i2c_scl_oe_i,
    input logic i2c_sda_oe_i,

    output logic i2c_scl_o,
    output logic i2c_sda_o

);

  pad_cell pad_cell_clk_i (
      .pad_in_i(1'b0),
      .pad_oe_i(1'b0),
      .pad_out_o(clk_o),
      .pad_io(clk_i)
  );

  pad_cell pad_cell_rst_i (
      .pad_in_i(1'b0),
      .pad_oe_i(1'b0),
      .pad_out_o(rst_no),
      .pad_io(rst_ni)
  );

  pad_cell pad_cell_boot_sel_i (
      .pad_in_i(1'b0),
      .pad_oe_i(1'b0),
      .pad_out_o(boot_select_o),
      .pad_io(boot_select_i)
  );

  pad_cell pad_cell_exec_flash_i (
      .pad_in_i(1'b0),
      .pad_oe_i(1'b0),
      .pad_out_o(execute_from_flash_o),
      .pad_io(execute_from_flash_i)
  );

  pad_cell pad_cell_tck_i (
      .pad_in_i(1'b0),
      .pad_oe_i(1'b0),
      .pad_out_o(jtag_tck_o),
      .pad_io(jtag_tck_i)
  );

  pad_cell pad_cell_tms_i (
      .pad_in_i(1'b0),
      .pad_oe_i(1'b0),
      .pad_out_o(jtag_tms_o),
      .pad_io(jtag_tms_i)
  );

  pad_cell pad_cell_trst_i (
      .pad_in_i(1'b0),
      .pad_oe_i(1'b0),
      .pad_out_o(jtag_trst_no),
      .pad_io(jtag_trst_ni)
  );

  pad_cell pad_cell_tdi_i (
      .pad_in_i(1'b0),
      .pad_oe_i(1'b0),
      .pad_out_o(jtag_tdi_o),
      .pad_io(jtag_tdi_i)
  );

  pad_cell pad_cell_tdo_i (
      .pad_in_i(jtag_tdo_i),
      .pad_oe_i(1'b1),
      .pad_out_o(),
      .pad_io(jtag_tdo_o)
  );

  pad_cell pad_cell_uart_rx_i (
      .pad_in_i(1'b0),
      .pad_oe_i(1'b0),
      .pad_out_o(uart_rx_o),
      .pad_io(uart_rx_i)
  );

  pad_cell pad_cell_uart_tx_i (
      .pad_in_i(uart_tx_i),
      .pad_oe_i(1'b1),
      .pad_out_o(),
      .pad_io(uart_tx_o)
  );

  pad_cell pad_cell_exit_valid_i (
      .pad_in_i(exit_valid_i),
      .pad_oe_i(1'b1),
      .pad_out_o(),
      .pad_io(exit_valid_o)
  );


  genvar i;
  generate
    for (i = 0; i < 32; i++) begin
      pad_cell pad_cell_gpio_i (
          .pad_in_i(gpio_out_i[i]),
          .pad_oe_i(gpio_oe_i[i]),
          .pad_out_o(gpio_in_o[i]),
          .pad_io(gpio_io[i])
      );
    end
  endgenerate


  pad_cell pad_cell_spi_sck_i (
      .pad_in_i(spi_sck_i),
      .pad_oe_i(spi_sck_en_i),
      .pad_out_o(),
      .pad_io(spi_sck_o)
  );

  genvar k;
  generate
    for (k = 0; k < spi_host_reg_pkg::NumCS; k++) begin
      pad_cell pad_cell_spi_csb_i (
          .pad_in_i(spi_csb_i[k]),
          .pad_oe_i(spi_csb_oe_i[k]),
          .pad_out_o(),
          .pad_io(spi_csb_o[k])
      );
    end
  endgenerate

  genvar j;
  generate
    for (j = 0; j < 4; j++) begin
      pad_cell pad_cell_sd_i (
          .pad_in_i(spi_sd_i[j]),
          .pad_oe_i(spi_sd_oe_i[j]),
          .pad_out_o(spi_sd_o[j]),
          .pad_io(spi_sd_io[j])
      );
    end
  endgenerate

  pad_cell pad_cell_i2c_scl_i (
      .pad_in_i(i2c_scl_i),
      .pad_oe_i(i2c_scl_oe_i),
      .pad_out_o(i2c_scl_o),
      .pad_io(i2c_scl_io)
  );

  pad_cell pad_cell_i2c_sda_i (
      .pad_in_i(i2c_sda_i),
      .pad_oe_i(i2c_sda_oe_i),
      .pad_out_o(i2c_sda_o),
      .pad_io(i2c_sda_io)
  );


endmodule  // core_v_mini_mcu
