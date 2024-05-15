# Copyright EPFL contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

import hjson

# hjson file from which the register data is stored
output_file = "data/m2s_dma.hjson"

# hjson file from which the peripherals info are taken
m2s_dma_cfg_file = "../../../mcu_cfg.hjson"

license_header = "// Copyright EPFL contributors.\n" + \
"// Licensed under the Apache License, Version 2.0, see LICENSE for details.\n" + \
"// SPDX-License-Identifier: Apache-2.0\n"

def format_hjson(file_path, size):
    
    try:
        # Read the contents of the file
        with open(file_path, 'r') as file:
            content = hjson.load(file)
        
        # Update the bits for TRANSACTION_IFR
        for register in content['registers']:
            if register['name'] == 'TRANSACTION_IFR':
                for field in register['fields']:
                    if field['name'] == 'EN':
                        field['bits'] = f"{size - 1}:0"

        # Update the bits for WINDOW_IFR
        for register in content['registers']:
            if register['name'] == 'WINDOW_IFR':
                for field in register['fields']:
                    if field['name'] == 'EN':
                        field['bits'] = f"{size - 1}:0"

        # Write the updated content back to the file
        with open(file_path, 'w') as file:
            # Write the license header back
            file.writelines(license_header)
            hjson.dump(content, file, ensure_ascii=False)
        
    except FileNotFoundError:
        print(f"The file {file_path} does not exist.")
    except Exception as e:
        print(f"An error occurred: {str(e)}")


if __name__ == "__main__":

    with open(m2s_dma_cfg_file) as f:
        data = hjson.load(f)
        
    channel_number = int(data['ao_peripherals']['m2s_dma']['channel_number'].strip().rstrip(','), 16)

    format_hjson(output_file, channel_number)
