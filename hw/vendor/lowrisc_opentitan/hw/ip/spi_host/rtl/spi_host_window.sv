// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Module to manage TX FIFO window for Serial Peripheral Interface (SPI) host IP.
//

`include "common_cells/assertions.svh"

module spi_host_window #(
  parameter type reg_req_t = logic,
  parameter type reg_rsp_t = logic
)(
  input  clk_i,
  input  rst_ni,
  input  reg_req_t          win_i,
  output reg_rsp_t          win_o,
  output logic [31:0]       tx_data_o,
  output logic [3:0]        tx_be_o,
  output logic              tx_valid_o,
  input        [31:0]       rx_data_i,
  output logic              rx_ready_o
);

  localparam int AW=spi_host_reg_pkg::BlockAw;
  localparam int DW=32;

  logic [AW-1:0] addr;

  // Only support reads/writes to the data fifo window
  logic win_error;
  assign win_error = (tx_valid_o || rx_ready_o) &&
                     (addr != spi_host_reg_pkg::SPI_HOST_DATA_OFFSET);

  // Check that our regbus data is 32 bit wide
`ASSERT_INIT(RegbusIs32Bit, $bits(win_i.wdata) == 32)

  // We are already a regbus, so no stateful adapter should be needed here
  // TODO @(paulsc, zarubaf): check this assumption!
  // Request
  assign tx_valid_o   = win_i.valid & win_i.write;    // write-enable
  assign rx_ready_o   = win_i.valid & ~win_i.write;   // read-enable
  assign addr         = win_i.addr;
  assign tx_data_o    = win_i.wdata;
  assign tx_be_o      = win_i.wstrb;
  // Response: always ready, else over/underflow error reported in regfile
  assign win_o.rdata  = rx_data_i;
  assign win_o.error  = win_error;
  assign win_o.ready  = 1'b1;

endmodule : spi_host_window
