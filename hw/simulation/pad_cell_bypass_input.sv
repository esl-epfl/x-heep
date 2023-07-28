// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

/* verilator lint_off UNUSED */
module pad_cell_bypass_input #(
    parameter PADATTR = 16,
    //do not touch these parameters
    parameter PADATTR_RND = PADATTR == 0 ? 1 : PADATTR
) (
    input logic pad_in_i,
    input logic pad_oe_i,
    output logic pad_out_o,
    inout logic pad_io,
    input logic [PADATTR_RND-1:0] pad_attributes_i
);

  // when ported to another technology, they remain like this
  assign pad_out_o = pad_io;

endmodule
