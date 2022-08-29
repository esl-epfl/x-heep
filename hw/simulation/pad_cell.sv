// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_cell #(
    parameter string PadType = "inout"  //inout, input, output
) (
    input  logic pad_in_i,
    input  logic pad_oe_i,
    output logic pad_out_o,

    inout logic pad_io
);

  logic pad;

  generate

    if (PadType == "inout") begin

      assign pad_out_o = pad_io;
      assign pad_io = pad;

      always_comb begin
        if (pad_oe_i == 1'b1) begin
          pad = pad_in_i;
        end else begin
          pad = 1'bz;
        end
      end

    end else if (PadType == "input") begin

      assign pad_out_o = pad_io;
      assign pad_io = pad;
      assign pad = 1'bz;

`ifndef SYNTHESIS
      // Check that you never want to drive an input PAD
      always_comb
      begin
        if (pad_oe_i != 1'b0) begin
          $error("%t: input PAD OE equal to 1", $time);
          $stop;
        end
      end
`endif

    end else if (PadType == "output") begin

      assign pad_out_o = 1'b0;
      assign pad_io = pad;
      assign pad = pad_in_i;

    end
  endgenerate


endmodule
