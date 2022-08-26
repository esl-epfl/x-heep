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

  IOBUF qspi_iobuf (
      .T (~pad_oe_i),
      .I (pad_in_i),
      .O (pad_out_o),
      .IO(pad_io)
  );

endmodule
