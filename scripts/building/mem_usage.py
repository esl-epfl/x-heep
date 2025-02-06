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



import subprocess
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


def get_readelf_output(elf_file):
    """
    Executes the readelf command on the provided ELF file with -l option to list program headers.
    """
    try:
        result = subprocess.run(['readelf', '-l', elf_file], capture_output=True, text=True)
        return result.stdout
    except Exception as e:
        print(f"Error running readelf: {e}")
        return None

def parse_program_headers(readelf_output):
    """
    Parses the output of readelf to extract program headers.
    """
    program_headers = []
    headers_started = False
    for line in readelf_output.split('\n'):
        if 'Program Headers:' in line:
            headers_started = True
        elif headers_started:
            if 'LOAD' in line:
                parts = re.split(r'\s+', line.strip())
                program_headers.append({
                    'Type': parts[0],
                    'Offset': int(parts[1], 16),
                    'VirtAddr': int(parts[2], 16),
                    'PhysAddr': int(parts[3], 16),
                    'FileSiz': int(parts[4], 16),
                    'MemSiz': int(parts[5], 16),
                    'Flg': parts[6],
                    'Align': int(parts[7], 16)
                })
            if 'Section to Segment mapping:' in line:
                break  # Stop after collecting program headers
    return program_headers

def parse_elf_output_to_regions(program_headers, section_to_segment):
    """
    Parse program headers and section-to-segment mapping to create a list of dictionaries
    describing each segment's start address, size, and type.
    """
    # Define a mapping from section names to region types
    code_sections = {'.vectors', '.init', '.text', '.eh_frame'}
    data_sections = {'.power_manager', '.rodata', '.data', '.sdata', '.sbss', '.bss', '.heap', '.stack'}
    interleaved_data_sections = {'.data_interleaved'}
    
    # List to store region dictionaries
    regions = []

    # Iterate through each program header
    for idx, ph in enumerate(program_headers):
        # Determine the type of region based on the sections it contains
        sections = section_to_segment[idx]
        region_type = 'd'  # default to data
        if any(sec in sections for sec in code_sections):
            region_type = 'C'
        elif any(sec in sections for sec in interleaved_data_sections):
            region_type = 'd'  # Special data handling like interleaved can be marked differently if needed
        
        # Create dictionary for the region
        region_dict = {
            'name': 'code' if region_type == 'C' else 'data',
            'symbol': region_type,
            'start_add': ph['VirtAddr'],
            'size_B': ph['MemSiz'],
            'end_add': ph['VirtAddr'] + ph['MemSiz']
        }
        
        # Append to the list
        regions.append(region_dict)

    return regions

import subprocess
import re

def get_readelf_output(elf_file):
    """
    Executes the readelf command on the provided ELF file with -l option to list program headers.
    """
    try:
        result = subprocess.run(['readelf', '-l', elf_file], capture_output=True, text=True)
        return result.stdout
    except Exception as e:
        print(f"Error running readelf: {e}")
        return None

def parse_program_headers(readelf_output):
    """
    Parses the output of readelf to extract program headers.
    """
    program_headers = []
    headers_started = False
    for line in readelf_output.split('\n'):
        if 'Program Headers:' in line:
            headers_started = True
        elif headers_started:
            if 'LOAD' in line:
                parts = re.split(r'\s+', line.strip())
                program_headers.append({
                    'Type': parts[0],
                    'Offset': int(parts[1], 16),
                    'VirtAddr': int(parts[2], 16),
                    'PhysAddr': int(parts[3], 16),
                    'FileSiz': int(parts[4], 16),
                    'MemSiz': int(parts[5], 16),
                    'Flg': parts[6],
                    'Align': int(parts[7], 16)
                })
            if 'Section to Segment mapping:' in line:
                break  # Stop after collecting program headers
    return program_headers

def parse_section_to_segment(readelf_output):
    """
    Parses the 'Section to Segment mapping' from the output of readelf.
    """
    mapping = {}
    capture = False
    segment_index = 0
    for line in readelf_output.split('\n'):
        if 'Section to Segment mapping:' in line:
            capture = True
        elif capture:
            if line.strip().startswith("Segment"):
                segments = re.findall(r'Segment\s+(\d+)', line)
                if segments:
                    segment_index = int(segments[0])
            else:
                sections = re.findall(r'\.\w+', line)
                if sections:
                    mapping[segment_index] = sections
                segment_index += 1
    return mapping

# Get readelf output
output = get_readelf_output('sw/build/main.elf')
program_headers = parse_program_headers(output)
section_to_segment = parse_section_to_segment(output)
regions = parse_elf_output_to_regions(program_headers, section_to_segment)


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
# regions = get_memory_regions('sw/build/main.map')
# regions['code'] = regions.pop('ram0')
# regions['data'] = regions.pop('ram1')
# regions['ildt'] = regions.pop('ram2') if num_il_banks else {'origin': regions['data']['origin'] + regions['data']['length'], 'length':0}
    
# # COMPUTE THE UTILIZATION OF CODE (max address used for code)
# text_addresses = get_addresses('sw/build/main.map', ' .text.start ', '.rodata ', )
# size_code_B = max( text_addresses  ) - regions['code']['origin']

# # COMPUTE THE UTILIZATION OF DATA (max address used for data)
# end_str = '.data_interleaved' if num_il_banks else '.comment '
# data_addresses = get_addresses('sw/build/main.map', ' *(.rodata .rodata', end_str)
# size_data_B = max( data_addresses  ) - regions['data']['origin']

# # COMPUTE THE UTILIZATION OF IL DATA (length detected for IL data)
# addr_ildt_hex, size_ildt_hex   = extract_ildt_length('sw/build/main.map')
# size_ildt_B = int(size_ildt_hex,16) if num_il_banks else 0



# # PRINT THE SUMMARY OF UTILIZATION
# print( "Region \t Start \tEnd\tSize(kB)\tUsed(kB)\tUtilz(%) ")
# print(f"Code:  \t{regions['code']['origin']/1024:5.1f}\t{(regions['code']['origin']+regions['code']['length'])/1024:5.1f}\t{regions['code']['length']/1024:5.1f}\t\t{size_code_B/1024:0.1f}\t\t{100*size_code_B/regions['code']['length']:0.1f}")
# print(f"Data:  \t{regions['data']['origin']/1024:5.1f}\t{(regions['data']['origin']+regions['data']['length'])/1024:5.1f}\t{regions['data']['length']/1024:5.1f}\t\t{size_data_B/1024:0.1f}\t\t{100*size_data_B/regions['data']['length']:0.1f}")
# if num_il_banks:
#     print(f"ILdata:\t{regions['ildt']['origin']/1024:5.1f}\t{(regions['ildt']['origin']+regions['ildt']['length'])/1024:5.1f}\t{regions['ildt']['length']/1024:5.1f}\t\t{size_ildt_B/1024:0.1f}\t\t{100*size_ildt_B/regions['ildt']['length']:0.1f}")


# DISPLAY THE UTILIZATION BY SHOWING THE BANKS 
# Cont for continuous, IntL for interleaved
# The area used by code is identified with a C
# The area used by data is identified with a d
# The utilization is shown at the end
# The granularity stands for how many Bytes each character represents
char            = '.'
address         = 0
granularity_B   = 1024
start_IL_B      = sum(bank_sizes_B[:-num_il_banks])

print("")
for bank_idx, bank in enumerate(banks):
    bank['use'] = ['-']*int((bank['size']/granularity_B))
    utilization = 0

    if bank['type'] == 'Cont':
        for piece in range(len(bank['use'])):
            address += granularity_B

            bank['use'][piece] = '-'
            for region in regions:
                if address> region['start_add'] and address <= region['end_add']:
                    bank['use'][piece] = region['symbol']
                    utilization += granularity_B
            
    if bank['type'] == "IntL":
        for piece in range(len(bank['use'])):
            address = start_IL_B + granularity_B*piece
            bank['use'][piece] = '-'
            for region in regions:
                used_by_others = (region['size_B']*(num_il_banks-1)/num_il_banks)
                if address>= region['start_add'] and address < region['end_add'] - used_by_others:
                    bank['use'][piece] = region['symbol']
                    utilization += granularity_B

    bank['use'] = ''.join([''.join(sublist) for sublist in bank['use']])
    print(bank['type'],bank_idx,bank['use'], f"\t{100*(utilization/bank['size']):0.1f}%")

