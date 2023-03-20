// Copyright 2022 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Author: Thomas Benz    <tbenz@ethz.ch>


/// DMA transaction id generator. Increases the transaction id on every request.
module idma_transfer_id_gen #(
    parameter int unsigned  IdWidth = -1
) (
    input  logic                clk_i,
    input  logic                rst_ni,
    // new request is pushed
    input  logic                issue_i,
    // request is popped
    input  logic                retire_i,
    // next id is
    output logic [IdWidth-1:0]  next_o,
    // last id completed is
    output logic [IdWidth-1:0]  completed_o
);

    //--------------------------------------
    // counters
    //--------------------------------------
    logic [IdWidth-1:0] next_d, next_q, completed_d, completed_q;

    // count up on events
    always_comb begin : proc_next_id
        // default
        next_d = next_q;
        // overflow
        if (next_q == '1) begin
            if (issue_i)
                next_d = 'h2;
            else
                next_d = 'h1;
        // request
        end else begin
            if (issue_i)
                next_d = 'h1 + next_q;
        end
    end

    always_comb begin : proc_next_completed
        // default
        completed_d = completed_q;
        // overflow
        if (completed_q == '1) begin
            if (retire_i)
                completed_d = 'h2;
            else
                completed_d = 'h1;
        // request
        end else begin
            if (retire_i)
                completed_d = 'h1 + completed_q;
        end
    end

    // assign outputs
    assign next_o      = next_q;
    assign completed_o = completed_q;

    //--------------------------------------
    // state
    //--------------------------------------
    always_ff @(posedge clk_i or negedge rst_ni) begin : proc_id_gen
        if(~rst_ni) begin
            next_q      <= 2;
            completed_q <= 1;
        end else begin
            next_q      <= next_d;
            completed_q <= completed_d;
        end
    end

endmodule : idma_transfer_id_gen
