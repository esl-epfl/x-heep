// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_cell_input #(
    parameter PADATTR = 16,
    parameter EXTRA_INPUTS = 4,
    parameter EXTRA_OUTPUTS = 4,
    parameter core_v_mini_mcu_pkg::pad_side_e SIDE = core_v_mini_mcu_pkg::TOP,
    //do not touch these parameters
    parameter PADATTR_RND = PADATTR == 0 ? 1 : PADATTR,
    parameter EXTRA_INPUTS_RND = EXTRA_INPUTS == 0 ? 1 : EXTRA_INPUTS,
    parameter EXTRA_OUTPUTS_RND = EXTRA_OUTPUTS == 0 ? 1 : EXTRA_OUTPUTS
) (
    input logic pad_in_i,
    input logic pad_oe_i,
    output logic pad_out_o,
    inout logic pad_io,
    input logic [PADATTR_RND-1:0] pad_attributes_i,
    input logic [EXTRA_INPUTS_RND-1:0] pad_extra_inputs_i,
    output logic [EXTRA_OUTPUTS_RND-1:0] pad_extra_outputs_o
);

  assign pad_out_o = pad_io;

endmodule
