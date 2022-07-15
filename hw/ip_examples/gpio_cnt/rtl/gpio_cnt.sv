// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module gpio_cnt #(
    parameter int unsigned CntMax = 32'd2048
) (
    input logic clk_i,
    input logic rst_ni,

    // input ports
    input logic gpio_i,

    // output ports
    output logic gpio_o
);

  logic [31:0] curr_cnt, next_cnt;

  typedef enum logic [1:0] {
    IDLE,
    COUNT,
    SET_OUT
  } fsm_state;

  fsm_state curr_state, next_state;

  // FSM seq logic
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_
    if (~rst_ni) begin
      curr_state <= IDLE;
      curr_cnt   <= 32'd0;
    end else begin
      curr_state <= next_state;
      curr_cnt   <= next_cnt;
    end
  end

  // FSM comb logic
  always_comb begin

    next_state = curr_state;
    next_cnt   = curr_cnt;

    unique case (curr_state)

      IDLE: begin
        gpio_o = 1'b0;

        if (gpio_i == 1'b1) begin
          next_state = COUNT;
        end
      end

      COUNT: begin
        gpio_o = 1'b0;

        if (curr_cnt == CntMax) begin
          next_state = SET_OUT;
          next_cnt   = 32'd0;
        end else begin
          next_cnt = curr_cnt + 32'd1;
        end
      end

      SET_OUT: begin
        gpio_o = 1'b1;

        if (gpio_i == 1'b0) begin
          next_state = IDLE;
        end
      end

      default: begin
        gpio_o = 1'b0;
        next_state = IDLE;
        next_cnt = 32'd0;
      end

    endcase

  end

endmodule
