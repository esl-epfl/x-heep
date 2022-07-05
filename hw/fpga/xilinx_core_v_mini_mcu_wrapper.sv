// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module xilinx_core_v_mini_mcu_wrapper #(
    parameter PULP_XPULP = 0,
    parameter FPU        = 0,
    parameter PULP_ZFINX = 0
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

    input  logic fetch_enable_i,
    output logic exit_value_o,
    output logic exit_valid_o,

    inout wire [3:0] spi_flash_dio,
    output wire spi_flash_cs,
    output wire spi_flash_clk
);

  logic        clk_gen;
  logic [31:0] exit_value;

  logic [ 3:0] spi_flash_di;
  logic [ 3:0] spi_flash_do;
  logic [ 3:0] spi_flash_oe;


  // IOBUF: bidirectional pads for the data pins of the QSPI flash
  genvar i;
  generate
    for (i = 1; i <= 3; i = i + 1) begin : generate_block_identifier
      IOBUF qspi_iobuf (
          .T (spi_flash_oe[i]),
          .I (spi_flash_do[i]),
          .O (spi_flash_di[i]),
          .IO(spi_flash_dio[i])
      );
    end
  endgenerate


  xilinx_clk_wizard_wrapper xilinx_clk_wizard_wrapper_i (
      .clk_125MHz(clk_i),
      .clk_out1_0(clk_gen)
  );

  core_v_mini_mcu core_v_mini_mcu_i (
      .clk_i         (clk_gen),
      .rst_ni        (rst_ni),
      .jtag_tck_i    (jtag_tck_i),
      .jtag_tms_i    (jtag_tms_i),
      .jtag_trst_ni  (jtag_trst_ni),
      .jtag_tdi_i    (jtag_tdi_i),
      .jtag_tdo_o    (jtag_tdo_o),
      .uart_rx_i     (uart_rx_i),
      .uart_tx_o     (uart_tx_o),
      .fetch_enable_i(fetch_enable_i),

      .flash_csb_o(spi_flash_cs),
      .flash_clk_o(spi_flash_clk),

      .flash_io0_oe_o(spi_flash_oe[0]),
      .flash_io1_oe_o(spi_flash_oe[1]),
      .flash_io2_oe_o(spi_flash_oe[2]),
      .flash_io3_oe_o(spi_flash_oe[3]),

      .flash_io0_do_o(spi_flash_do[0]),
      .flash_io1_do_o(spi_flash_do[1]),
      .flash_io2_do_o(spi_flash_do[2]),
      .flash_io3_do_o(spi_flash_do[3]),

      .flash_io0_di_i(spi_flash_di[0]),
      .flash_io1_di_i(spi_flash_di[1]),
      .flash_io2_di_i(spi_flash_di[2]),
      .flash_io3_di_i(spi_flash_di[3]),

      .exit_value_o(exit_value),
      .exit_valid_o(exit_valid_o)
  );

  assign exit_value_o = exit_value[0];


endmodule
