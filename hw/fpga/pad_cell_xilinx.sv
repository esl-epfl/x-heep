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

  generate

    if (PadType == "inout") begin

      IOBUF xilinx_iobuf_i (
          .T (~pad_oe_i),
          .I (pad_in_i),
          .O (pad_out_o),
          .IO(pad_io)
      );

    end else if (PadType == "input") begin

      assign pad_out_o = pad_io;

    end else if (PadType == "output") begin

      assign pad_out_o = 1'b0;
      assign pad_io = pad_in_i;

    end
  endgenerate



endmodule
