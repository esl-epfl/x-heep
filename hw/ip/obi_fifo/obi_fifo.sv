// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

//This module relies on the fact that the variable latency XBAR does not rise a new REQ if the previous one has not been granted


module obi_fifo
  import obi_pkg::*;
(
    input logic clk_i,
    input logic rst_ni,

    input  obi_req_t  producer_req_i,
    output obi_resp_t producer_resp_o,

    output obi_req_t  consumer_req_o,
    input  obi_resp_t consumer_resp_i
);

  typedef struct packed {
    logic        we;
    logic [3:0]  be;
    logic [31:0] addr;
    logic [31:0] wdata;
  } obi_data_req_t;

  typedef enum logic {
    CONSUMER_REQ,
    WAIT_CONSUMER_GNT
  } consumer_req_fsm_e;

  obi_data_req_t producer_data_req, consumer_data_req;
  consumer_req_fsm_e state_n, state_q;

  assign {producer_data_req.we, producer_data_req.be, producer_data_req.addr, producer_data_req.wdata} =
          {
    producer_req_i.we, producer_req_i.be, producer_req_i.addr, producer_req_i.wdata
  };

  assign {consumer_req_o.we, consumer_req_o.be, consumer_req_o.addr, consumer_req_o.wdata} = {
    consumer_data_req.we, consumer_data_req.be, consumer_data_req.addr, consumer_data_req.wdata
  };

  logic fifo_req_full, fifo_req_empty, fifo_req_push, fifo_req_pop;
  logic fifo_resp_full, fifo_resp_empty, fifo_resp_push, fifo_resp_pop;

  assign producer_resp_o.gnt = producer_req_i.req & !fifo_req_full;
  assign fifo_req_push       = producer_req_i.req & !fifo_req_full;
  assign fifo_req_pop        = !fifo_req_empty && state_q == CONSUMER_REQ;

  always_comb begin
    state_n = state_q;
    consumer_req_o.req = 1'b0;
    case (state_q)

      CONSUMER_REQ: begin
        if (fifo_req_pop) begin
          consumer_req_o.req = 1'b1;
          if (!consumer_resp_i.gnt) state_n = WAIT_CONSUMER_GNT;
        end
      end

      WAIT_CONSUMER_GNT: begin
        consumer_req_o.req = 1'b1;
        if (consumer_resp_i.gnt) state_n = CONSUMER_REQ;
      end

    endcase

  end

  fifo_v3 #(
      .DEPTH(1),
      .dtype(obi_data_req_t)
  ) obi_req_fifo_i (
      .clk_i,
      .rst_ni,
      .flush_i(1'b0),
      .testmode_i(1'b0),
      .full_o(fifo_req_full),
      .empty_o(fifo_req_empty),
      .usage_o(),
      //togliere .req da qui se no rimane ad 1, fare logica per driver
      .data_i(producer_data_req),
      .push_i(fifo_req_push),
      //togliere .req da qui se no rimane ad 1, fare logica per driver
      .data_o(consumer_data_req),
      .pop_i(fifo_req_pop)
  );

  //todo add asserts - it cannot be full as we are popping all the time
  assign fifo_resp_push = consumer_resp_i.rvalid & !fifo_resp_full;
  assign fifo_resp_pop = !fifo_resp_empty;
  assign producer_resp_o.rvalid = fifo_resp_pop;

  fifo_v3 #(
      .DEPTH(1),
      .dtype(logic [31:0])
  ) obi_resp_fifo_i (
      .clk_i,
      .rst_ni,
      .flush_i(1'b0),
      .testmode_i(1'b0),
      .full_o(fifo_resp_full),
      .empty_o(fifo_resp_empty),
      .usage_o(),
      .data_i(consumer_resp_i.rdata),
      .push_i(fifo_resp_push),
      // grant is given above
      .data_o(producer_resp_o.rdata),
      .pop_i(fifo_resp_pop)
  );

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (~rst_ni) begin
      state_q <= CONSUMER_REQ;
    end else begin
      state_q <= state_n;
    end
  end



endmodule
