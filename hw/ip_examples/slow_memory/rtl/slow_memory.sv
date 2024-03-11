// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module slow_memory #(
    parameter int unsigned NumWords = 32'd1024,  // Number of Words in data array
    parameter int unsigned DataWidth = 32'd32,  // Data signal width
    // DEPENDENT PARAMETERS, DO NOT OVERWRITE!
    parameter int unsigned AddrWidth = (NumWords > 32'd1) ? $clog2(NumWords) : 32'd1
) (
    input  logic                 clk_i,    // Clock
    input  logic                 rst_ni,   // Asynchronous reset active low
    // input ports
    input  logic                 req_i,    // request
    input  logic                 we_i,     // write enable
    input  logic [AddrWidth-1:0] addr_i,   // request address
    input  logic [         31:0] wdata_i,  // write data
    input  logic [          3:0] be_i,     // write byte enable
    // output ports
    output logic                 gnt_o,    // memory is ready
    output logic [         31:0] rdata_o,  // read data
    output logic                 rvalid_o  // read data is valid
);


  logic                 mem_req_q;
  logic                 mem_we_q;
  logic [AddrWidth-1:0] mem_addr_q;
  logic [         31:0] mem_wdata_q;
  logic [          3:0] mem_be_q;

  logic                 mem_req_n;
  logic                 mem_we_n;
  logic [AddrWidth-1:0] mem_addr_n;
  logic [         31:0] mem_wdata_n;
  logic [          3:0] mem_be_n;

  logic                 mem_req;
  logic                 mem_we;
  logic [AddrWidth-1:0] mem_addr;
  logic [         31:0] mem_wdata;
  logic [          3:0] mem_be;

  logic rvalid_n, rvalid_q;
  logic [4:0] counter_n, counter_q;

  typedef enum logic {
    READY,
    WAIT_RVALID
  } slow_memory_fsm_e;

  slow_memory_fsm_e state_q, state_n;

  int random1, random2;

  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_
    if (~rst_ni) begin
      counter_q <= '0;
      rvalid_q <= 1'b0;
      state_q <= READY;
      mem_req_q <= 1'b0;
      mem_we_q <= 1'b0;
      mem_addr_q <= '0;
      mem_wdata_q <= '0;
      mem_be_q <= '0;
    end else begin
      random1 <= $random();
      random2 <= $random();
      counter_q <= counter_n;
      rvalid_q <= rvalid_n;
      state_q <= state_n;
      mem_req_q <= mem_req_n;
      mem_we_q <= mem_we_n;
      mem_addr_q <= mem_addr_n;
      mem_wdata_q <= mem_wdata_n;
      mem_be_q <= mem_be_n;
    end
  end


  always_comb begin
    gnt_o       = 1'b0;
    rvalid_o    = rvalid_q;
    state_n     = state_q;
    counter_n   = counter_q - 1;
    rvalid_n    = rvalid_q;
    mem_req     = '0;
    mem_we      = '0;
    mem_addr    = '0;
    mem_wdata   = '0;
    mem_be      = '0;
    mem_req_n   = mem_req_q;
    mem_we_n    = mem_we_q;
    mem_addr_n  = mem_addr_q;
    mem_wdata_n = mem_wdata_q;
    mem_be_n    = mem_be_q;
    unique case (state_q)

      READY: begin
        rvalid_n = 1'b0;
        if (req_i) begin
          gnt_o = random1[0];
          if (gnt_o) begin
            state_n     = WAIT_RVALID;
            counter_n   = random2[4:0] + 1;
            mem_req_n   = req_i;
            mem_we_n    = we_i;
            mem_addr_n  = addr_i;
            mem_wdata_n = wdata_i;
            mem_be_n    = be_i;
          end
        end
      end

      WAIT_RVALID: begin
        if (counter_q == 1) begin
          mem_req = mem_req_q;
          mem_we = mem_we_q;
          mem_addr = mem_addr_q;
          mem_wdata = mem_wdata_q;
          mem_be = mem_be_q;
          state_n = READY;
          rvalid_n = 1'b1;
        end
      end
    endcase

  end

  tc_sram #(
      .NumWords (NumWords),
      .DataWidth(DataWidth),
      .NumPorts (32'd1)
  ) tc_ram_i (
      .clk_i  (clk_i),
      .rst_ni (rst_ni),
      .req_i  (mem_req),
      .we_i   (mem_we),
      .addr_i (mem_addr),
      .wdata_i(mem_wdata),
      .be_i   (mem_be),
      // output ports
      .rdata_o(rdata_o)
  );


endmodule
