// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

/* verilator lint_off UNUSED */
/* verilator lint_off MULTIDRIVEN */

module memory_subsystem
  import obi_pkg::*;
#(
    parameter NUM_BANKS = 2
) (
    input logic clk_i,
    input logic rst_ni,

    // Clock-gating signal
    input logic [NUM_BANKS-1:0] clk_gate_en_i,

    input  obi_req_t  [NUM_BANKS-1:0] ram_req_i,
    output obi_resp_t [NUM_BANKS-1:0] ram_resp_o,

    input logic [core_v_mini_mcu_pkg::NUM_BANKS-1:0] set_retentive_i
);

  localparam int NumWords = 32 * 1024 / 4;
  localparam int AddrWidth = $clog2(32 * 1024);

  logic [NUM_BANKS-1:0] ram_valid_q;
  // Clock-gating
  logic [NUM_BANKS-1:0] clk_cg;
  for (genvar i = 0; i < NUM_BANKS; i++) begin : gen_sram

    tc_clk_gating clk_gating_cell_i (
        .clk_i,
        .en_i(~clk_gate_en_i[i]),
        .test_en_i(1'b0),
        .clk_o(clk_cg[i])
    );

    always_ff @(posedge clk_cg[i] or negedge rst_ni) begin
      if (!rst_ni) begin
        ram_valid_q[i] <= '0;
      end else begin
        ram_valid_q[i] <= ram_resp_o[i].gnt;
      end
    end

    assign ram_resp_o[i].gnt = ram_req_i[i].req;
    assign ram_resp_o[i].rvalid = ram_valid_q[i];

    //Fixed to 8KWords per bank (32KB)
    sram_wrapper #(
        .NumWords (NumWords),
        .DataWidth(32'd32)
    ) ram_i (
        .clk_i(clk_cg[i]),
        .rst_ni(rst_ni),
        .req_i(ram_req_i[i].req),
        .we_i(ram_req_i[i].we),
        .addr_i(ram_req_i[i].addr[AddrWidth-1:2]),
        .wdata_i(ram_req_i[i].wdata),
        .be_i(ram_req_i[i].be),
        .set_retentive_i(set_retentive_i[i]),
        .rdata_o(ram_resp_o[i].rdata)
    );

  end

endmodule
