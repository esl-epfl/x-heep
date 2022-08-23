// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

/* verilator lint_off UNUSED */

`include "common_cells/assertions.svh"

module power_manager #(
    parameter type reg_req_t = logic,
    parameter type reg_rsp_t = logic
) (
    input clk_i,
    input rst_ni,

    // Bus Interface
    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    // Power gate core signal
    input  logic rv_timer_irq_i,
    input  logic core_sleep_i,
    output logic power_gate_core_o,
    output logic cpu_subsystem_rst_no
);

  import power_manager_reg_pkg::*;

  power_manager_reg2hw_t reg2hw;

  logic [31:0] curr_cnt, next_cnt;

  assign power_gate_core_o = 1'b0;

  typedef enum logic [1:0] {
    IDLE,
    PW_OFF_RST_ON,
    PW_ON_RST_ON,
    PW_ON_RST_OFF
  } fsm_state;

  fsm_state curr_state, next_state;

  power_manager_reg_top #(
      .reg_req_t(reg_req_t),
      .reg_rsp_t(reg_rsp_t)
  ) power_manager_reg_top_i (
      .clk_i,
      .rst_ni,
      .reg_req_i,
      .reg_rsp_o,
      .reg2hw,
      .devmode_i(1'b1)
  );

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
        // power_gate_core_o = 1'b0;
        cpu_subsystem_rst_no = 1'b1;

        if (reg2hw.power_gate_core.q == 1'b1 && core_sleep_i == 1'b1) begin
          next_state = PW_OFF_RST_ON;
        end
      end

      PW_OFF_RST_ON: begin
        // power_gate_core_o = 1'b1;
        cpu_subsystem_rst_no = 1'b0;

        if (rv_timer_irq_i == 1'b1) begin
          next_state = PW_ON_RST_ON;
        end
      end

      PW_ON_RST_ON: begin
        // power_gate_core_o = 1'b0;
        cpu_subsystem_rst_no = 1'b0;

        if (curr_cnt == 32'd20) begin
          next_state = PW_ON_RST_OFF;
          next_cnt   = 32'd0;
        end else begin
          next_cnt = curr_cnt + 32'd1;
        end
      end

      PW_ON_RST_OFF: begin
        // power_gate_core_o = 1'b0;
        cpu_subsystem_rst_no = 1'b1;

        if (reg2hw.power_gate_core.q == 1'b0) begin
          next_state = IDLE;
        end
      end

      default: begin
        // power_gate_core_o = 1'b0;
        cpu_subsystem_rst_no = 1'b1;
        next_state = IDLE;
        next_cnt = 32'd0;
      end

    endcase

  end

endmodule : power_manager
