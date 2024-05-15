# Copyright 2024 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
# 
# Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
# 
# Info: Derived from Occamy: 
#   https://github.com/pulp-platform/snitch/blob/master/hw/system/occamy/src/occamy_cfg.hjson\n
#   Configuration file generator for Multichannel Multidimensional Smart DMA.

import hjson

def gen_hjson(mcu_cfg_path, m2s_dma_path, output_file_path=None):
    # Read the Hjson from the input file
    with open(mcu_cfg_path, 'r') as file:
        data = hjson.load(file)

    # License header
    license = ("/*\n"
    " * Copyright 2024 EPFL\n"
    " * Solderpad Hardware License, Version 2.1, see LICENSE.md for details.\n"
    " * SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1\n"
    " *\n"
    " * Author: Tommaso Terzano <tommaso.terzano@epfl.ch>\n"
    " *  \n"
    " * Info: \n"
    " *     Derived from Occamy: https://github.com/pulp-platform/snitch/blob/master/hw/system/occamy/src/occamy_cfg.hjson\n"
    " *     DMA channels configuration for M2S DMA system.\n"
    "*/\n\n")

    # New dictionary
    new_data = dict()

    channels = dict()

    # Get the base offset and channel number from the m2s_dma configuration
    base_offset = int(data['ao_peripherals']['m2s_dma']['offset'].strip().rstrip(','), 16)
    channel_number = int(data['ao_peripherals']['m2s_dma']['channel_number'].strip().rstrip(','), 16)
    
    # Iterate over the number of channels and add dma_chx entries
    for i in range(channel_number):
        dma_key = f"dma_ch{i}"
        new_dma_entry = {
            "offset": hex(base_offset + 0x100 * (i + 1)),  # Starting from the next block after m2s_dma
            "length": hex(256),
            "path": "./hw/ip/dma/data/dma.hjson"
        }
        
        new_dma_entry['offset'] = '0x' + new_dma_entry['offset'][2:].zfill(8)
        new_dma_entry['length'] = '0x' + new_dma_entry['length'][2:].zfill(8)
        
        channels[dma_key] = new_dma_entry

    new_data["dma_channels"] = channels
    # Write the updated Hjson
    with open(m2s_dma_path, 'w') as file:
        file.write(license)
        hjson.dump(new_data, file, indent=2)

if __name__ == "__main__":
    input_hjson_file_path = "././mcu_cfg.hjson"
    output_hjson_file_path = "././m2s_dma_cfg.hjson"
    gen_hjson(input_hjson_file_path, output_hjson_file_path)
