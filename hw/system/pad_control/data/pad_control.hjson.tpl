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

% for pad in pad_muxed_list:
    { name:     "PAD_MUX_${pad.name.upper()}",
      desc:     "Used to mux pad ${pad.name.upper()}",
      resval:   "0x0"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "3:0", name: "PAD_MUX_${pad.name.upper()}", desc: "Pad Mux ${pad.name.upper()} Reg" }
      ]
    }

% endfor

% for pad in total_pad_list:
    { name:     "PAD_ATTRIBUTE_${pad.name.upper()}",
      desc:     "${pad.name} Attributes (Pull Up En, Pull Down En, etc. It is technology specific.",
      resval:   "0x00"
      swaccess: "rw",
      hwaccess: "hro",
      fields: [
        { bits: "7:0", name: "PAD_ATTRIBUTE_${pad.name.upper()}", desc: "Pad Attribute ${pad.name.upper()} Reg" }
      ]
    }

% endfor


   ]
}
