// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module memory_subsystem
  import obi_pkg::*;
#(
    parameter NUM_BYTES = 2 ** 16
) (
    input logic clk_i,
    input logic rst_ni,

    input  obi_req_t  ram0_req_i,
    output obi_resp_t ram0_resp_o,
    input  obi_req_t  ram1_req_i,
    output obi_resp_t ram1_resp_o
);

  localparam int NumWords = NUM_BYTES / 4;
  localparam int AddrWidth = $clog2(NUM_BYTES);

  logic ram0_valid_q, ram1_valid_q;

  always_ff @(posedge clk_i or negedge rst_ni) begin : ram_valid_q
    if (!rst_ni) begin
      ram0_valid_q <= '0;
      ram1_valid_q <= '0;
    end else begin
      ram0_valid_q <= ram0_resp_o.gnt;
      ram1_valid_q <= ram1_resp_o.gnt;
    end
  end


  assign ram0_resp_o.gnt = ram0_req_i.req;
  assign ram0_resp_o.rvalid = ram0_valid_q;

  //16Kwords per bank (64KB)
  sram_wrapper #(
      .NumWords (NumWords / 2),
      .DataWidth(32'd32)
  ) ram0_i (
      .clk_i  (clk_i),
      .rst_ni (rst_ni),
      .req_i  (ram0_req_i.req),
      .we_i   (ram0_req_i.we),
      .addr_i (ram0_req_i.addr[AddrWidth-1-1:2]),
      .wdata_i(ram0_req_i.wdata),
      .be_i   (ram0_req_i.be),
      // output ports
      .rdata_o(ram0_resp_o.rdata)
  );

  assign ram1_resp_o.gnt = ram1_req_i.req;
  assign ram1_resp_o.rvalid = ram1_valid_q;

  //16Kwords per bank (64KB)
  sram_wrapper #(
      .NumWords (NumWords / 2),
      .DataWidth(32'd32)
  ) ram1_i (
      .clk_i  (clk_i),
      .rst_ni (rst_ni),
      .req_i  (ram1_req_i.req),
      .we_i   (ram1_req_i.we),
      .addr_i (ram1_req_i.addr[AddrWidth-1-1:2]),
      .wdata_i(ram1_req_i.wdata),
      .be_i   (ram1_req_i.be),
      // output ports
      .rdata_o(ram1_resp_o.rdata)
  );

endmodule
