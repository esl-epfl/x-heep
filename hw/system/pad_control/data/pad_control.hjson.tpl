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

% for pad in xheep.get_padring().pad_muxed_list:
    { name:     "PAD_MUX_${pad.name.upper()}",
      desc:     "Used to mux pad ${pad.name.upper()}",
      resval:   "0x0"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "${(len(pad.pad_mux_list)-1).bit_length()-1}:0", name: "PAD_MUX_${pad.name.upper()}", desc: "Pad Mux ${pad.name.upper()} Reg" }
      ]
    }

% endfor

% if xheep.get_padring().pads_attributes != None:
% for pad in xheep.get_padring().total_pad_list:
% if pad.pad_type == 'input' or pad.pad_type == 'output' or pad.pad_type == 'inout':
% if pad.constant_attribute == False:
    { name:     "PAD_ATTRIBUTE_${pad.name.upper()}",
      desc:     "${pad.name} Attributes (Pull Up En, Pull Down En, etc. It is technology specific.",
      resval:   "${xheep.get_padring().pads_attributes['resval']}"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "${xheep.get_padring().pads_attributes['bits']}", name: "PAD_ATTRIBUTE_${pad.name.upper()}", desc: "Pad Attribute ${pad.name.upper()} Reg" }
      ]
    }
% endif
% endif
% endfor
% endif

   ]
}
