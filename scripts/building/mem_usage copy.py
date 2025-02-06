# Copyright EPFL contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
#
# Author: Juan Sapriza <juan.sapriza@epfl.ch>
#
# Info: This script parses the generated main.map and core_v_mini_mcu_pkg.sv files to 
# display the usage of the different memory banks of the generated MCU for code (text) and data. 
# The script considers the possibility of having interleaved (IL) memory banks at the end of the
# continuous memory banks. In the IL banks, data is distributed homogeneously across banks (although
# this does not necessarily need to be the case). 
# The code extracts the number and size of the memory banks from the MCU package. 
# Then extracts the regions (code and data) from the main.map file -- i.e. where code and data can 
# be stored.  
# Later extracts the utilization of those regions by looking for the addresses in which text and data
# has been written in the main.map file. 
# For the IL data (ildt) only the length is extracted, for simplicity. We assume an homogeneous distribution.


import re

def get_banks_and_sizes(mcu_pkg_size):
    """
    Parses the core_v_mini_mcu_pkg.sv file to extract the count of memory banks and their sizes. 
    It looks for the definitions:
    localparam int unsigned NUM_BANKS = 5;
    localparam int unsigned NUM_BANKS_IL = 2;

    To obtain the total and IL count. 

    Later looks for 
    localparam logic [31:0] RAM0_SIZE = 32'h00008000;
    To extract the size of each. 
    They are all assumed to be contiguous. 

    Parameters:
    mcu_pkg_size - path of the .sv file, relative to the location from which this script is called (e.g. the Makefile)

    Returns: 
    num_banks       - Total count of memory banks
    num_il_banks    - How many of those banks are IL
    sizes_B         - Size in bytes of each bank
    """
    num_banks = 0
    num_il_banks = 0
    sizes_B = []
    try:
        with open(mcu_pkg_size, 'r') as file:
            for line in file:
                if "NUM_BANKS =" in line:
                    num_banks = int(line.split('=')[1].strip().strip(';'))
                elif "NUM_BANKS_IL =" in line:
                    num_il_banks = int(line.split('=')[1].strip().strip(';'))
                else: 
                    match = re.search(r"RAM(\d+)_SIZE = 32'h([0-9A-Fa-f]+);", line)
                    if match:
                        size_B      = int(match.group(2), 16)
                        sizes_B.append(size_B)               
    except FileNotFoundError:
        print("File not found. Please check the path and try again.")
    return num_banks, num_il_banks, sizes_B


def get_memory_regions(map_path):
    """
    Parses the main.map file to obtain the origin and length of each region. 
    These are called ram0 (code), ram1 (data) and ram2 (IL data) - but that does not necessarily 
    correspond with an index of memory banks. 

    The origin and size of each are defined in the configs/*.hjson files.

    Parameters:
    map_path - path of the .map file, relative to the location from which this script is called (e.g. the Makefile)

    Returns: 
    regions - Dictionary with the regions found
    """
    regions = {}
    try:
        with open(map_path, 'r') as file:
            collect = False
            for line in file:
                if "Name" in line and "Origin" in line and "Length" in line:
                    collect = True
                    continue
                if collect:
                    if line.strip() == '':
                        collect = False  # Stop collecting when a blank line is encountered
                        continue
                    parts = line.split()
                    if len(parts) >= 4:
                        name = parts[0]
                        origin = int(parts[1], 16)
                        length = int(parts[2], 16)
                        attributes = parts[3]
                        regions[name] = {'origin': origin, 'length': length, 'attributes': attributes}
    except FileNotFoundError:
        print("File not found. Please check the path and try again.")
    return regions

def get_addresses(map_path, start_str, end_str):
    """
    Parses the main.map file to obtain an array of addresses of each .text or data section.
    This corresponds with the space used for each.
    It looks between the start_str and the end_str.  

    Parameters:
    map_path - path of the .map file, relative to the location from which this script is called (e.g. the Makefile)
    start_str - A string that indicates the beginning of the section
    end_str - A string that indicates the end of the section
    
    Returns: 
    addresses - A list of addresses where code is stored
    """
    pattern = re.compile(r'0x[0-9A-Fa-f]{16}\b')
    addresses = []
    start = False
    with open(map_path, 'r') as file:
        for line in file:
            if start:
                if line.startswith(end_str): break
                matches = pattern.findall(line)
                if matches:
                    addresses.extend(matches)
            elif line.startswith(start_str): 
                start = True
    addresses = [ int(a,16) for a in addresses]
    return addresses

def extract_ildt_length(map_path):
    """
    Parses the main.map file to obtain the length of the IL section

    Parameters:
    map_path - path of the .map file, relative to the location from which this script is called (e.g. the Makefile)
        
    Returns: 
    ildt_address    - The start address of the IL section
    ildt_length     - The length of the data stored there 
    """
    pattern = re.compile(r'\.data_interleaved\s+0x[0-9A-Fa-f]+\s+0x[0-9A-Fa-f]+')
    ildt_length     = None
    ildt_address    = None

    with open(map_path, 'r') as file:
        content = file.read()  # Read the whole file into a single string
        matches = pattern.findall(content)
        if matches:
            last_match = matches[-1]  # Get the last match
            parts = last_match.split()
            ildt_length = parts[-1]  # Extract the last hex number from the last match
            ildt_address = parts[-2]
    return ildt_address, ildt_length



# OBTAIN THE NUMBER AND SIZE OF THE BANKS
num_banks, num_il_banks, bank_sizes_B = get_banks_and_sizes('hw/core-v-mini-mcu/include/core_v_mini_mcu_pkg.sv')
total_size_B = sum(bank_sizes_B)
print(f"Total space: {total_size_B/1024:0.1f} kB = Continuous:",[int(s/1024) for s in bank_sizes_B[:num_banks-num_il_banks]],"kB + Interleaved:", [int(s/1024) for s in bank_sizes_B[-num_il_banks:]], "kB")

# CONVERT THE BANKS INTO A LIST OF DICTIONARIES
banks = []
for i in range(num_banks):
    bank = {
        'type'  : "Cont" if i<(num_banks-num_il_banks) else "IntL",
        'size'  : bank_sizes_B[i],
    }
    banks.append(bank)

# GET THE MEMORY REGIONS FOR CODE AND DATA, TRANSLATE ramx to code, data, IL
# If there are no IL banks, create an entry with length 0
regions = get_memory_regions('sw/build/main.map')
regions['code'] = regions.pop('ram0')
regions['data'] = regions.pop('ram1')
regions['ildt'] = regions.pop('ram2') if num_il_banks else {'origin': regions['data']['origin'] + regions['data']['length'], 'length':0}
    
# COMPUTE THE UTILIZATION OF CODE (max address used for code)
text_addresses = get_addresses('sw/build/main.map', ' .text.start ', '.rodata ', )
size_code_B = max( text_addresses  ) - regions['code']['origin']

# COMPUTE THE UTILIZATION OF DATA (max address used for data)
end_str = '.data_interleaved' if num_il_banks else '.comment '
data_addresses = get_addresses('sw/build/main.map', ' *(.rodata .rodata', end_str)
size_data_B = max( data_addresses  ) - regions['data']['origin']

# COMPUTE THE UTILIZATION OF IL DATA (length detected for IL data)
addr_ildt_hex, size_ildt_hex   = extract_ildt_length('sw/build/main.map')
size_ildt_B = int(size_ildt_hex,16) if num_il_banks else 0

# PRINT THE SUMMARY OF UTILIZATION
print( "Region \t Start \tEnd\tSize(kB)\tUsed(kB)\tUtilz(%) ")
print(f"Code:  \t{regions['code']['origin']/1024:5.1f}\t{(regions['code']['origin']+regions['code']['length'])/1024:5.1f}\t{regions['code']['length']/1024:5.1f}\t\t{size_code_B/1024:0.1f}\t\t{100*size_code_B/regions['code']['length']:0.1f}")
print(f"Data:  \t{regions['data']['origin']/1024:5.1f}\t{(regions['data']['origin']+regions['data']['length'])/1024:5.1f}\t{regions['data']['length']/1024:5.1f}\t\t{size_data_B/1024:0.1f}\t\t{100*size_data_B/regions['data']['length']:0.1f}")
if num_il_banks:
    print(f"ILdata:\t{regions['ildt']['origin']/1024:5.1f}\t{(regions['ildt']['origin']+regions['ildt']['length'])/1024:5.1f}\t{regions['ildt']['length']/1024:5.1f}\t\t{size_ildt_B/1024:0.1f}\t\t{100*size_ildt_B/regions['ildt']['length']:0.1f}")


# DISPLAY THE UTILIZATION BY SHOWING THE BANKS 
# Cont for continuous, IntL for interleaved
# The area used by code is identified with a C
# The area used by data is identified with a d
# The utilization is shown at the end
# The granularity stands for how many Bytes each character represents
char = '.'
address = 0
granularity_B = 1024

# Switch these two lines to choose if to display the correct start of the data or the
# "de facto" start, displaced by the code (in case the code size exceeds the designated length
data_start_B = max(regions['data']['origin'],regions['code']['origin'] + size_code_B)
data_start_B =  regions['data']['origin']

print("")

for i, bank in enumerate(banks):
    bank['use'] = ['-']*int((bank['size']/granularity_B))
    utilization = 0

    if bank['type'] == 'Cont':
        for piece in range(len(bank['use'])):
            address += granularity_B
            if address <= regions['code']['origin'] + size_code_B:
                if address <= regions['code']['origin'] + regions['code']['length']:
                    bank['use'][piece] = 'C'
                else:
                    bank['use'][piece] = '!'
                utilization += granularity_B

            elif address >= data_start_B  and address <= data_start_B + size_data_B:
                if address <= data_start_B + regions['data']['length']:
                    bank['use'][piece] = 'd'
                else:
                    bank['use'][piece] = '!'
                utilization += granularity_B
            else:
                bank['use'][piece] = '-'

    if bank['type'] == "IntL":
        size_per_il_bank = int((size_ildt_B/num_il_banks)/granularity_B)
        for piece in range(size_per_il_bank):
            bank['use'][piece] = 'd'
        for piece in range(size_per_il_bank, len(bank['use'])):
            bank['use'][piece] = '-'


    bank['use'] = ''.join([''.join(sublist) for sublist in bank['use']])
    print(bank['type'],i,bank['use'], f"\t{100*(utilization/bank['size']):0.1f}%")

