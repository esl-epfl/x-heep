#
#     Copyright EPFL contributors.
#     Licensed under the Apache License, Version 2.0, see LICENSE for details.
#     SPDX-License-Identifier: Apache-2.0
#
#     Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
#                             <tommaso.terzano@gmail.com>
#
#     Info: Plotter script for im2col spc verification data.
#

import matplotlib.pyplot as plt
import pandas as pd
import ast
import numpy as np

# Function to parse data from string to dictionary
def parse_data(data):
    results = []
    for item in data:
        # Split each item by ', ' to get key-value pairs
        pairs = item.split(', ')
        # Create a dictionary from the pairs
        result_dict = {}
        for pair in pairs:
            key, value = pair.split(': ')
            # Convert value to integer
            result_dict[key] = int(value)
        results.append(result_dict)
    return results

# Function to add loop_size field
def add_loop_size(data):
    for entry in data:
        # Calculate the number of patches, i.e. the number the filter can fit along one dimension during convolution
        n_patches_h = int((entry['H'] - entry['FH']  + entry['PT'] + entry['PB']) / entry['S2']) + 1
        n_patches_w = int((entry['W'] - entry['FW']  + entry['PR'] + entry['PL']) / entry['S1']) + 1
        # Calculate the dimensions of the output matrix
        OH = entry['FW'] * entry['FH'] * entry['C'] * entry['B'] # Number of rows in a column -> size of a column
        OW = n_patches_h * n_patches_w # Number of columns in a row -> size of a row
        entry['loop_size'] = OH*OW
    return data

# Read and parse data from file
def read_data(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    
    im2col_cpu = []
    im2col_dma_2d_C = []
    im2col_spc = []
    
    current_section = None
    for line in lines:
        line = line.strip()
        if line.startswith("im2col_cpu:"):
            current_section = im2col_cpu
        elif line.startswith("im2col_dma_2d_C:"):
            current_section = im2col_dma_2d_C
        elif line.startswith("im2col_spc:"):
            current_section = im2col_spc
        elif line:
            if current_section is not None:
                current_section.append(line.strip("[]'"))

    return im2col_cpu, im2col_dma_2d_C, im2col_spc

# Path to the file containing the data
file_path = 'im2col_data.txt'

# Read data from the file
im2col_cpu_array, im2col_dma_2d_C_array, im2col_spc_array = read_data(file_path)

# Elaborate the data
parsed_1ch_CPU = parse_data(ast.literal_eval("'"+im2col_cpu_array[0]+"'"))
parsed_1ch_DMA = parse_data(ast.literal_eval("'"+im2col_dma_2d_C_array[0]+"'"))
parsed_1ch_spc = parse_data(ast.literal_eval("'"+im2col_spc_array[0]+"'"))

add_loop_size(parsed_1ch_CPU)
add_loop_size(parsed_1ch_DMA)
add_loop_size(parsed_1ch_spc)

df_1ch_CPU = pd.DataFrame(parsed_1ch_CPU)
df_1ch_DMA = pd.DataFrame(parsed_1ch_DMA)
df_1ch_spc = pd.DataFrame(parsed_1ch_spc)

# Plot the data
plt.figure(0, figsize=(12, 8))

# Scatter plots
plt.scatter(df_1ch_CPU['loop_size'], df_1ch_CPU['cycles'], color='blue', label='1ch CPU', alpha=1)
plt.scatter(df_1ch_DMA['loop_size'], df_1ch_DMA['cycles'], color='red', label='1ch DMA', alpha=1)
plt.scatter(df_1ch_spc['loop_size'], df_1ch_spc['cycles'], color='green', label='1ch SPC', alpha=1)

# Trendline plots
p_cpu = np.polyfit(df_1ch_CPU['loop_size'],df_1ch_CPU['cycles'],  1)
trendline_cpu = np.polyval(p_cpu, df_1ch_CPU['loop_size'])

p_dma = np.polyfit(df_1ch_DMA['loop_size'], df_1ch_DMA['cycles'], 1)
trendline_dma = np.polyval(p_dma, df_1ch_DMA['loop_size'])

p_spc = np.polyfit(df_1ch_spc['loop_size'], df_1ch_spc['cycles'], 1)
trendline_spc = np.polyval(p_spc, df_1ch_spc['loop_size'])

plt.plot(df_1ch_CPU['loop_size'], trendline_cpu, color='blue', linestyle='-', alpha=0.6)
plt.plot(df_1ch_DMA['loop_size'], trendline_dma, color='red', linestyle='-', alpha=0.6)
plt.plot(df_1ch_spc['loop_size'], trendline_spc, color='green', linestyle='-', alpha=0.6)


# Title and labels
plt.title('Loop Size vs Cycles')
plt.xlabel('Loop Size')
plt.ylabel('Cycles')
plt.grid(True)

# Legend
plt.legend()

# Save plot
plt.savefig('plot.png')  

# Show plot
plt.show()
