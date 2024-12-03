// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
{ name: "pad_control",
  clock_primary: "clk_i",
  bus_interfaces: [
    { protocol: "reg_iface", direction: "device" }
  ],
  regwidth: "32",
  registers: [

% for pad, num in xheep.get_pad_manager().iterate_muxed_pad_index_with_num():
    { name:     "PAD_MUX_${pad}",
      desc:     "Used to mux pad ${pad}",
      resval:   "0x0"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "${(num-1).bit_length()-1}:0", name: "PAD_MUX_${pad}", desc: "Pad Mux ${pad} Reg" }
      ]
    }

% endfor

% if xheep.get_pad_manager().get_attr_bits() != 0:
% for pad in xheep.get_pad_manager().iterate_pad_index():
    { name:     "PAD_ATTRIBUTE_${pad}",
      desc:     "${pad} Attributes (Pull Up En, Pull Down En, etc. It is technology specific.",
      resval:   "${pads_attributes['resval']}"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "${xheep.get_pad_manager().get_attr_bits()-1}:0", name: "PAD_ATTRIBUTE_${pad}", desc: "Pad Attribute ${pad} Reg" }
      ]
    }

% endfor
% endif

   ]
}
