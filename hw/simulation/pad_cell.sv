// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_cell #(
) (
    input  logic pad_in_i,
    input  logic pad_oe_i,
    output logic pad_out_o,

    inout logic pad_io
);

  logic pad;

  assign pad_out_o = pad_io;
  assign pad_io = pad;

  always_comb begin
    if (pad_oe_i == 1'b1) begin
      pad = pad_in_i;
    end else begin
      pad = 1'bz;
    end
  end

endmodule
