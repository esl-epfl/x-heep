// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_ring (
% for pad in pad_list:
${pad.pad_ring_io_interface}
${pad.pad_ring_ctrl_interface}
% endfor

% for external_pad in external_pad_list:
${external_pad.pad_ring_io_interface}
${external_pad.pad_ring_ctrl_interface}
% endfor

% if pads_attributes != None:
    input logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][${pads_attributes['bits']}] pad_attributes_i,
% else:
    // here just for simplicity
    /* verilator lint_off UNUSED */
    input logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][0:0] pad_attributes_i,
% endif
% if pads_extra_inputs != None:
    input logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][${pads_extra_inputs['bits']}] pad_extra_inputs_i,
% else:
   /* verilator lint_off UNUSED */
    input logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][0:0] pad_extra_inputs_i,
% endif
% if pads_extra_outputs != None:
    output logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][${pads_extra_outputs['bits']}] pad_extra_outputs_o
% else:
   /* verilator lint_off UNDRIVEN */
    output logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][0:0] pad_extra_outputs_o
% endif

);

% for pad in pad_list:
${pad.pad_ring_instance}
% endfor

% for external_pad in external_pad_list:
${external_pad.pad_ring_instance}
% endfor

endmodule  // pad_ring
