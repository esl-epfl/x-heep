// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module memory_subsystem
  import obi_pkg::*;
#(
    parameter NUM_BYTES = 2 ** 19
) (
    input logic clk_i,
    input logic rst_ni,

    input  obi_req_t  ram0_req_i,
    output obi_resp_t ram0_resp_o,
    input  obi_req_t  ram1_req_i,
    output obi_resp_t ram1_resp_o,
    input  obi_req_t  ram2_req_i,
    output obi_resp_t ram2_resp_o,
    input  obi_req_t  ram3_req_i,
    output obi_resp_t ram3_resp_o,
    input  obi_req_t  ram4_req_i,
    output obi_resp_t ram4_resp_o,
    input  obi_req_t  ram5_req_i,
    output obi_resp_t ram5_resp_o,
    input  obi_req_t  ram6_req_i,
    output obi_resp_t ram6_resp_o,
    input  obi_req_t  ram7_req_i,
    output obi_resp_t ram7_resp_o
);

  localparam int NumWords = NUM_BYTES / 4;
  localparam int AddrWidth = $clog2(NUM_BYTES);

  logic ram0_valid_q;
  logic ram1_valid_q;
  logic ram2_valid_q;
  logic ram3_valid_q;
  logic ram4_valid_q;
  logic ram5_valid_q;
  logic ram6_valid_q;
  logic ram7_valid_q;

  always_ff @(posedge clk_i or negedge rst_ni) begin : ram_valid_q
    if (!rst_ni) begin
      ram0_valid_q <= '0;
      ram1_valid_q <= '0;
      ram2_valid_q <= '0;
      ram3_valid_q <= '0;
      ram4_valid_q <= '0;
      ram5_valid_q <= '0;
      ram6_valid_q <= '0;
      ram7_valid_q <= '0;
    end else begin
      ram0_valid_q <= ram0_resp_o.gnt;
      ram1_valid_q <= ram1_resp_o.gnt;
      ram2_valid_q <= ram2_resp_o.gnt;
      ram3_valid_q <= ram3_resp_o.gnt;
      ram4_valid_q <= ram4_resp_o.gnt;
      ram5_valid_q <= ram5_resp_o.gnt;
      ram6_valid_q <= ram6_resp_o.gnt;
      ram7_valid_q <= ram7_resp_o.gnt;
    end
  end

  assign ram0_resp_o.gnt = ram0_req_i.req;
  assign ram0_resp_o.rvalid = ram0_valid_q;

  sram_wrapper #(
      .NumWords (NumWords / 8),
      .DataWidth(32'd32)
  ) ram0_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .req_i(ram0_req_i.req),
      .we_i(ram0_req_i.we),
      .addr_i(ram0_req_i.addr[AddrWidth-3-1:2]),
      .wdata_i(ram0_req_i.wdata),
      .be_i(ram0_req_i.be),
      .rdata_o(ram0_resp_o.rdata)
  );

  assign ram1_resp_o.gnt = ram1_req_i.req;
  assign ram1_resp_o.rvalid = ram1_valid_q;

  sram_wrapper #(
      .NumWords (NumWords / 8),
      .DataWidth(32'd32)
  ) ram1_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .req_i(ram1_req_i.req),
      .we_i(ram1_req_i.we),
      .addr_i(ram1_req_i.addr[AddrWidth-3-1:2]),
      .wdata_i(ram1_req_i.wdata),
      .be_i(ram1_req_i.be),
      .rdata_o(ram1_resp_o.rdata)
  );

  assign ram2_resp_o.gnt = ram2_req_i.req;
  assign ram2_resp_o.rvalid = ram2_valid_q;

  sram_wrapper #(
      .NumWords (NumWords / 8),
      .DataWidth(32'd32)
  ) ram2_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .req_i(ram2_req_i.req),
      .we_i(ram2_req_i.we),
      .addr_i(ram2_req_i.addr[AddrWidth-3-1:2]),
      .wdata_i(ram2_req_i.wdata),
      .be_i(ram2_req_i.be),
      .rdata_o(ram2_resp_o.rdata)
  );

  assign ram3_resp_o.gnt = ram3_req_i.req;
  assign ram3_resp_o.rvalid = ram3_valid_q;

  sram_wrapper #(
      .NumWords (NumWords / 8),
      .DataWidth(32'd32)
  ) ram3_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .req_i(ram3_req_i.req),
      .we_i(ram3_req_i.we),
      .addr_i(ram3_req_i.addr[AddrWidth-3-1:2]),
      .wdata_i(ram3_req_i.wdata),
      .be_i(ram3_req_i.be),
      .rdata_o(ram3_resp_o.rdata)
  );

  assign ram4_resp_o.gnt = ram4_req_i.req;
  assign ram4_resp_o.rvalid = ram4_valid_q;

  sram_wrapper #(
      .NumWords (NumWords / 8),
      .DataWidth(32'd32)
  ) ram4_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .req_i(ram4_req_i.req),
      .we_i(ram4_req_i.we),
      .addr_i(ram4_req_i.addr[AddrWidth-3-1:2]),
      .wdata_i(ram4_req_i.wdata),
      .be_i(ram4_req_i.be),
      .rdata_o(ram4_resp_o.rdata)
  );

  assign ram5_resp_o.gnt = ram5_req_i.req;
  assign ram5_resp_o.rvalid = ram5_valid_q;

  sram_wrapper #(
      .NumWords (NumWords / 8),
      .DataWidth(32'd32)
  ) ram5_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .req_i(ram5_req_i.req),
      .we_i(ram5_req_i.we),
      .addr_i(ram5_req_i.addr[AddrWidth-3-1:2]),
      .wdata_i(ram5_req_i.wdata),
      .be_i(ram5_req_i.be),
      .rdata_o(ram5_resp_o.rdata)
  );

  assign ram6_resp_o.gnt = ram6_req_i.req;
  assign ram6_resp_o.rvalid = ram6_valid_q;

  sram_wrapper #(
      .NumWords (NumWords / 8),
      .DataWidth(32'd32)
  ) ram6_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .req_i(ram6_req_i.req),
      .we_i(ram6_req_i.we),
      .addr_i(ram6_req_i.addr[AddrWidth-3-1:2]),
      .wdata_i(ram6_req_i.wdata),
      .be_i(ram6_req_i.be),
      .rdata_o(ram6_resp_o.rdata)
  );

  assign ram7_resp_o.gnt = ram7_req_i.req;
  assign ram7_resp_o.rvalid = ram7_valid_q;

  sram_wrapper #(
      .NumWords (NumWords / 8),
      .DataWidth(32'd32)
  ) ram7_i (
      .clk_i(clk_i),
      .rst_ni(rst_ni),
      .req_i(ram7_req_i.req),
      .we_i(ram7_req_i.we),
      .addr_i(ram7_req_i.addr[AddrWidth-3-1:2]),
      .wdata_i(ram7_req_i.wdata),
      .be_i(ram7_req_i.be),
      .rdata_o(ram7_resp_o.rdata)
  );

endmodule
