// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module power_manager_counter_sequence #(
    //value that is set after reset
    parameter logic IDLE_VALUE = 1'b1,
    //value that is set during reset
    parameter logic ONOFF_AT_RESET = 1'b1
) (
    input logic clk_i,
    input logic rst_ni,

    // trigger to start the sequence
    input logic start_off_sequence_i,
    input logic start_on_sequence_i,
    // if true, wait for ack after counter expires
    input logic switch_ack_i,

    // counter to switch on and off signals
    input logic counter_expired_switch_off_i,
    input logic counter_expired_switch_on_i,

    output logic counter_start_switch_off_o,
    output logic counter_start_switch_on_o,

    // switch on and off signal, IDLE_VALUE means on
    output logic switch_onoff_signal_o

);

  typedef enum logic [2:0] {
    RESET,
    IDLE,
    WAIT_SWITCH_OFF_COUNTER,
    SWITCH_OFF,
    WAIT_SWITCH_ON_COUNTER,
    WAIT_ACK,
    SWITCH_ON
  } sequence_fsm_state;

  sequence_fsm_state sequence_curr_state, sequence_next_state;

  logic switch_onoff_signal_d, switch_onoff_signal_q;

  // FSM seq logic
  always_ff @(posedge clk_i or negedge rst_ni) begin : proc_
    if (~rst_ni) begin
      sequence_curr_state   <= RESET;
      switch_onoff_signal_q <= ONOFF_AT_RESET;
    end else begin
      sequence_curr_state   <= sequence_next_state;
      switch_onoff_signal_q <= switch_onoff_signal_d;
    end
  end

  assign switch_onoff_signal_o = switch_onoff_signal_q;

  // FSM comb logic
  always_comb begin : power_manager_counter_sequence_fsm

    sequence_next_state = sequence_curr_state;
    counter_start_switch_off_o = 1'b0;
    counter_start_switch_on_o = 1'b0;
    switch_onoff_signal_d = switch_onoff_signal_q;

    unique case (sequence_curr_state)

      RESET: begin
        switch_onoff_signal_d = ONOFF_AT_RESET;
        sequence_next_state   = IDLE;
      end

      IDLE: begin
        switch_onoff_signal_d = IDLE_VALUE;
        if (start_off_sequence_i) begin
          counter_start_switch_off_o = 1'b1;
          sequence_next_state = WAIT_SWITCH_OFF_COUNTER;
        end
      end

      WAIT_SWITCH_OFF_COUNTER: begin
        switch_onoff_signal_d = IDLE_VALUE;
        sequence_next_state   = counter_expired_switch_off_i ? SWITCH_OFF : WAIT_SWITCH_OFF_COUNTER;
        sequence_next_state   = counter_expired_switch_off_i ? SWITCH_OFF : WAIT_SWITCH_OFF_COUNTER;
      end

      SWITCH_OFF: begin
        switch_onoff_signal_d = ~IDLE_VALUE;
        if (start_on_sequence_i) begin
          counter_start_switch_on_o = 1'b1;
          sequence_next_state = WAIT_SWITCH_ON_COUNTER;
        end
      end

      WAIT_SWITCH_ON_COUNTER: begin
        switch_onoff_signal_d = ~IDLE_VALUE;
        if (counter_expired_switch_on_i) begin
          sequence_next_state = switch_ack_i ? SWITCH_ON : WAIT_ACK;
        end else begin
          sequence_next_state = WAIT_SWITCH_ON_COUNTER;
        end
      end

      WAIT_ACK: begin
        switch_onoff_signal_d = ~IDLE_VALUE;
        sequence_next_state   = switch_ack_i ? SWITCH_ON : WAIT_ACK;
      end

      SWITCH_ON: begin
        switch_onoff_signal_d = IDLE_VALUE;
        sequence_next_state   = IDLE;
      end


      default: begin
        sequence_next_state = IDLE;
      end

    endcase

  end

endmodule
