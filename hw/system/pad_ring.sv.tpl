// Copyright 2022 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

module pad_ring (
${xheep.get_pad_manager().make_root_io_ports(internal=True)}
##% for pad in pad_list:
##${pad.pad_ring_io_interface}
##${pad.pad_ring_ctrl_interface}
##% endfor

##% for external_pad in external_pad_list:
##${external_pad.pad_ring_io_interface}
##${external_pad.pad_ring_ctrl_interface}
##% endfor

${xheep.get_rh().get_node_ports(xheep.get_pad_manager().get_pad_ring_node())}
% if xheep.get_pad_manager().get_attr_bits() != 0:
    input logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][${xheep.get_pad_manager().get_attr_bits()}-1:0] pad_attributes_i
% else:
    // here just for simplicity
    /* verilator lint_off UNUSED */
    input logic [core_v_mini_mcu_pkg::NUM_PAD-1:0][0:0] pad_attributes_i
% endif


);

##% for pad in pad_list:
##${pad.pad_ring_instance}
##% endfor
##
##% for external_pad in external_pad_list:
##${external_pad.pad_ring_instance}
##% endfor

${xheep.get_pad_manager().make_pad_cells(xheep.get_rh())}

endmodule  // pad_ring
