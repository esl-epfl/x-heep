// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module i2s_microphone #(
    parameter int unsigned CntMax = 32'd2048
) (
    input logic rst_ni,

    // i2s interface ports
    input logic i2s_sck_i,
    input logic i2s_ws_i,

    // output ports
    output logic i2s_sd_o
);

  logic [5:0] bit_count;
  logic curr_ws;

  always_ff @(posedge i2s_sck_i or negedge rst_ni) begin
    if (~rst_ni) begin
      bit_count <= 0;
    end else begin
      if (i2s_ws_i != curr_ws) begin
        bit_count <= 0;
      end else begin
        bit_count <= bit_count + 1;
      end
    end
  end

  always_ff @(negedge i2s_sck_i) begin
    i2s_sd_o <= bit_count[2];
  end

endmodule
