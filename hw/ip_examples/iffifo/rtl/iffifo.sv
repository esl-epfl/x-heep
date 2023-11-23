// Copyright 2023 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
// Date: 18.10.2023

module iffifo #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic,
    localparam int WIDTH = 32,
    localparam int DEPTH = 4,
    localparam int DEPTHw = $clog2(DEPTH + 1)
) (
    input logic clk_i,
    input logic rst_ni,
    input reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    // DMA slots
    output logic iffifo_in_ready_o,
    output logic iffifo_out_valid_o,

    // Interrupt lines
    output logic iffifo_int_o
);

  import iffifo_reg_pkg::*;

  iffifo_reg2hw_t reg2hw;
  iffifo_hw2reg_t hw2reg;

  logic [WIDTH-1:0] fifout;
  logic [DEPTHw-1:0] occupancy;
  logic empty, full, reached, available;

  assign hw2reg.fifo_out.d = fifout + 1;

  // Status (full/empty/watermark) reporting circuitry
  assign empty = (occupancy == 0);
  assign available = !empty;
  assign hw2reg.status.empty.de = 1;
  assign hw2reg.status.empty.d = empty;
  assign hw2reg.status.available.de = 1;
  assign hw2reg.status.available.d = available;
  assign hw2reg.status.full.de = 1;
  assign hw2reg.status.full.d = full;
  assign hw2reg.status.reached.de = 1;
  assign hw2reg.status.reached.d = reached;
  assign hw2reg.occupancy.de = 1;
  assign hw2reg.occupancy.d = {{32 - DEPTHw{1'b0}}, occupancy};
  assign reached = ({{32 - DEPTHw{1'b0}}, occupancy} >= reg2hw.watermark.q);

  // Interrupts circuitry
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin

      iffifo_int_o <= 0;

    end else begin

      // Interrupts firing
      if (reached & reg2hw.interrupts.q) begin
        iffifo_int_o <= 1;
      end

      // Interrupts assertion
      if (reg2hw.interrupts.qe) begin
        iffifo_int_o <= 0;
      end

    end
  end

  iffifo_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) iffifo_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg2hw,
      .hw2reg,
      .reg_req_i,
      .reg_rsp_o,
      .devmode_i(1'b0)
  );

  prim_fifo_sync #(
      .Width(WIDTH),
      .Depth(DEPTH),
      .Pass(1'b0),
      .OutputZeroIfEmpty(1'b0)
  ) iffifo_fifo_i (

      .clk_i,
      .rst_ni,
      .clr_i(1'b0),

      // From DMA (output) to FIFO (input, TX)
      .wvalid_i(reg2hw.fifo_in.qe),
      .wready_o(iffifo_in_ready_o),
      .wdata_i (reg2hw.fifo_in.q),

      // From FIFO (output) to DMA (input, RX)
      .rvalid_o(iffifo_out_valid_o),
      .rready_i(reg2hw.fifo_out.re),
      .rdata_o (fifout),

      .full_o (full),
      .depth_o(occupancy)

  );

endmodule : iffifo

