// Copyright 2023 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51
//
// Hayate Okuhara <hayate.okuhara@unibo.it>

/// Generates 4 phase shifted clocks out of one faster clock
module hyperbus_clk_gen (
    input  logic clk_i,     // input clock
    input  logic rst_ni,
    output logic clk0_o,    // have the input clock - 0deg phase shift
    output logic clk90_o,   // have the input clock - 90deg phase shift
    output logic clk180_o,  // have the input clock - 180deg phase shift
    output logic clk270_o,   // have the input clock - 270deg phase shift
    output logic rst_no
);


    logic r_clk0_o;
    logic r_clk90_o;
    logic r_clk180_o;
    logic r_clk270_o;

    logic  s_clk0_n;

    assign clk0_o = r_clk0_o;
    assign clk90_o = r_clk90_o;
    assign clk180_o = r_clk180_o;
    assign clk270_o = r_clk270_o;
    assign rst_no = rst_ni;

    tc_clk_inverter i_clk0_inverter (
                     .clk_i (r_clk0_o),
                     .clk_o (s_clk0_n)
                     );


    always_ff @(posedge clk_i or negedge rst_ni) begin
        if(~rst_ni) begin
            r_clk0_o   <= 0;
            r_clk180_o <= 1;
        end else begin
            r_clk0_o <= s_clk0_n;
            r_clk180_o <= r_clk90_o;
        end
    end

    always_ff @(negedge clk_i or negedge rst_ni) begin
        if (~rst_ni) begin
            r_clk90_o  <= 0;
            r_clk270_o <= 1;
        end else begin
            r_clk90_o  <= r_clk0_o;
            r_clk270_o <= r_clk180_o;
        end
    end

endmodule
