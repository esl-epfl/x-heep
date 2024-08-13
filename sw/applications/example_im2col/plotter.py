import matplotlib.pyplot as plt
import pandas as pd
import ast
import numpy as np

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
        entry['loop_size'] = entry['C'] * entry['B'] * entry['H'] * entry['W']
    return data

# Read and parse data from file
def read_data(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    
    im2col_cpu = []
    im2col_dma_2d_C = []
    im2col_spc = []

    counter = 0
    
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
parsed_2ch_spc = parse_data(ast.literal_eval("'"+im2col_spc_array[1]+"'"))
parsed_3ch_spc = parse_data(ast.literal_eval("'"+im2col_spc_array[2]+"'"))
parsed_4ch_spc = parse_data(ast.literal_eval("'"+im2col_spc_array[3]+"'"))

add_loop_size(parsed_1ch_CPU)
add_loop_size(parsed_1ch_DMA)
add_loop_size(parsed_1ch_spc)
add_loop_size(parsed_2ch_spc)
add_loop_size(parsed_3ch_spc)
add_loop_size(parsed_4ch_spc)

df_1ch_CPU = pd.DataFrame(parsed_1ch_CPU)
df_1ch_DMA = pd.DataFrame(parsed_1ch_DMA)
df_1ch_spc = pd.DataFrame(parsed_1ch_spc)
df_2ch_spc = pd.DataFrame(parsed_2ch_spc)
df_3ch_spc = pd.DataFrame(parsed_3ch_spc)
df_4ch_spc = pd.DataFrame(parsed_4ch_spc)

# Plot the data
plt.figure(0, figsize=(12, 8))

# Scatter plots
plt.scatter(df_1ch_CPU['loop_size'], df_1ch_CPU['cycles'], color='blue', label='1ch CPU', alpha=1)
plt.scatter(df_1ch_DMA['loop_size'], df_1ch_DMA['cycles'], color='red', label='1ch DMA', alpha=1)
plt.scatter(df_1ch_spc['loop_size'], df_1ch_spc['cycles'], color='green', label='1ch SPC', alpha=1)
plt.scatter(df_2ch_spc['loop_size'], df_2ch_spc['cycles'], color='orange', label='2ch SPC', alpha=1)
plt.scatter(df_3ch_spc['loop_size'], df_3ch_spc['cycles'], color='purple', label='3ch SPC', alpha=1)
plt.scatter(df_4ch_spc['loop_size'], df_4ch_spc['cycles'], color='cyan', label='4ch SPC', alpha=1)

# Trendline plots
p_cpu = np.polyfit(df_1ch_CPU['loop_size'],df_1ch_CPU['cycles'],  1)
trendline_cpu = np.polyval(p_cpu, df_1ch_CPU['loop_size'])

p_dma = np.polyfit(df_1ch_DMA['loop_size'], df_1ch_DMA['cycles'], 1)
trendline_dma = np.polyval(p_dma, df_1ch_DMA['loop_size'])

p_spc = np.polyfit(df_1ch_spc['loop_size'], df_1ch_spc['cycles'], 1)
trendline_spc = np.polyval(p_spc, df_1ch_spc['loop_size'])

p_2ch_spc = np.polyfit(df_2ch_spc['loop_size'], df_2ch_spc['cycles'], 1)
trendline_spc_2ch = np.polyval(p_2ch_spc, df_2ch_spc['loop_size'])

p_3ch_spc = np.polyfit(df_3ch_spc['loop_size'], df_3ch_spc['cycles'], 1)
trendline_spc_3ch = np.polyval(p_3ch_spc, df_3ch_spc['loop_size'])

p_4ch_spc = np.polyfit(df_4ch_spc['loop_size'], df_4ch_spc['cycles'], 1)
trendline_spc_4ch = np.polyval(p_4ch_spc, df_4ch_spc['loop_size'])

plt.plot(df_1ch_CPU['loop_size'], trendline_cpu, color='blue', linestyle='-', alpha=0.6)
plt.plot(df_1ch_DMA['loop_size'], trendline_dma, color='red', linestyle='-', alpha=0.6)
plt.plot(df_1ch_spc['loop_size'], trendline_spc, color='green', linestyle='-', alpha=0.6)
plt.plot(df_2ch_spc['loop_size'], trendline_spc_2ch, color='orange', linestyle='-', alpha=0.6)
plt.plot(df_3ch_spc['loop_size'], trendline_spc_3ch, color='purple', linestyle='-', alpha=0.6)
plt.plot(df_4ch_spc['loop_size'], trendline_spc_4ch, color='cyan', linestyle='-', alpha=0.6)


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
