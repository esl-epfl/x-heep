import subprocess
import re
import matplotlib.pyplot as plt
import pandas as pd

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

num_channels_dma = 5

batch_max = 4

channels_max = 4

im_h_max = 11

im_w_max = 11

ker_h_max = 3

ker_w_max = 3

pad_top_max = 2
pad_bottom_max = 2
pad_left_max = 2
pad_right_max = 2

stride_d1_max = 2
stride_d2_max = 2

ver_script_dir = "/home/ubuntu/Desktop/Xheep/Source/x-heep/sw/applications/example_im2col/verification_script.py"

imcol_lib_dir = "/home/ubuntu/Desktop/Xheep/Source/x-heep/sw/applications/example_im2col/im2col_lib.h"

verification_script_com = "python verification_script.py"

app_compile_run_com = " cd ../../../ ; make app PROJECT=example_im2col ; cd build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator/ ; ./Vtestharness +firmware=../../../sw/build/main.hex ; cd ../../../ "

ver_patterns = {
    'image_height': r'(image_height\s*=\s*)\d+',
    'image_width': r'(image_width\s*=\s*)\d+',
    'channels': r'(channels\s*=\s*)\d+',
    'batch': r'(batch\s*=\s*)\d+',
    'filter_height': r'(filter_height\s*=\s*)\d+',
    'filter_width': r'(filter_width\s*=\s*)\d+',
    'top_pad': r'(top_pad\s*=\s*)\d+',
    'bottom_pad': r'(bottom_pad\s*=\s*)\d+',
    'left_pad': r'(left_pad\s*=\s*)\d+',
    'right_pad': r'(right_pad\s*=\s*)\d+',
    'stride_d1': r'(stride_d1\s*=\s*)\d+',
    'stride_d2': r'(stride_d2\s*=\s*)\d+'
}

im2col_lib_pattern = re.compile(r'#define SPC_CH_MASK 0b\d+')
im2col_lib_pattern_cpu_done = re.compile(r'#define START_ID \d+')

im2col_cpu_array = []
im2col_dma_2d_C_array = []
im2col_spc_array = []

def generate_mask(num_masters, num_slaves, max_masters_per_slave, num_channels):
    # Initialize the mask with all zeros
    mask = [0] * num_masters

    # Calculate the number of crossbars needed
    crossbars = []
    master_index = 0
    for _ in range(num_slaves):
        if master_index >= num_masters:
            break
        end_index = min(master_index + max_masters_per_slave, num_masters)
        crossbars.append(list(range(master_index, end_index)))
        master_index = end_index

    # Reverse the crossbars to start allocation from the rightmost master
    crossbars.reverse()
    for cb in crossbars:
        cb.reverse()

    # Generate the mask for the specified number of channels
    used_channels = set()
    crossbar_index = 0

    while len(used_channels) < num_channels:
        for cb in crossbars:
            if len(used_channels) >= num_channels:
                break
            for master in cb:
                if master not in used_channels:
                    used_channels.add(master)
                    mask[master] = 1
                    break

    return ''.join(map(str, mask))

def modify_parameters(file_path, modifications, patterns):
    # Read the contents of the file
    with open(file_path, 'r') as file:
        content = file.read()
    
    # Modify the content
    for param, new_value in modifications.items():
        if param in patterns:
            content = re.sub(patterns[param], r'\g<1>{}'.format(new_value), content)
    
    # Write the modified content back to the file
    with open(file_path, 'w') as file:
        file.write(content)

cpu_done = 1

for i in range(4, num_channels_dma):
    print("Number of channels used by SPC", i)
    im2col_cpu = []
    im2col_dma_2d_C = []
    im2col_spc = []

    for j in range(1, batch_max):

        for k in range(1, channels_max):

            for l in range(10, im_h_max):

                for m in range(10, im_w_max):

                    for n in range(2, ker_h_max):

                        for o in range(2, ker_w_max):

                            for p in range(1, pad_top_max):

                                for q in range(1, pad_bottom_max):

                                    for r in range(1, pad_left_max):

                                        for s in range(1, pad_right_max):

                                            for t in range(1, stride_d1_max):

                                                for u in range(1, stride_d2_max):
                                                    print("Batch size: ", j)
                                                    print("Number of input channels: ", k)
                                                    print("Image height: ", l)
                                                    print("Image width: ", m)
                                                    print("Kernel height: ", n)
                                                    print("Kernel width: ", o)
                                                    print("Pad top: ", p)
                                                    print("Pad bottom: ", q)
                                                    print("Pad left: ", r)
                                                    print("Pad right: ", s)
                                                    print("Stride d1: ", t)
                                                    print("Stride d2: ", u)


                                                    print("_______________________\n\n")
                                                    progress = (u + t*stride_d1_max + s*stride_d2_max*stride_d1_max + r*pad_right_max*stride_d2_max*stride_d1_max + q*pad_left_max*pad_right_max*stride_d2_max*stride_d1_max + p*pad_bottom_max*pad_left_max*pad_right_max*stride_d2_max*stride_d1_max + o*pad_top_max*pad_bottom_max*pad_left_max*pad_right_max*stride_d2_max*stride_d1_max + n*ker_w_max*pad_top_max*pad_bottom_max*pad_left_max*pad_right_max*stride_d2_max*stride_d1_max + m*ker_h_max*ker_w_max*pad_top_max*pad_bottom_max*pad_left_max*pad_right_max*stride_d2_max*stride_d1_max + l*im_w_max*ker_h_max*ker_w_max*pad_top_max*pad_bottom_max*pad_left_max*pad_right_max*stride_d2_max*stride_d1_max + k*im_h_max*im_w_max*ker_h_max*ker_w_max*pad_top_max*pad_bottom_max*pad_left_max*pad_right_max*stride_d2_max*stride_d1_max + j*channels_max*im_h_max*im_w_max*ker_h_max*ker_w_max*pad_top_max*pad_bottom_max*pad_left_max*pad_right_max*stride_d2_max*stride_d1_max + i*batch_max*channels_max*im_h_max*im_w_max*ker_h_max*ker_w_max*pad_top_max*pad_bottom_max*pad_left_max*pad_right_max*stride_d2_max*stride_d1_max)/(batch_max*channels_max*im_h_max*im_w_max*ker_h_max*ker_w_max*pad_top_max*pad_bottom_max*pad_left_max*pad_right_max*stride_d2_max*stride_d1_max*num_channels_dma) * 100 
                                                    print("", progress, "%")
                                                    print("_______________________\n")
                                                    
                                                    ver_modifications = {
                                                        'image_height': l,
                                                        'image_width': m,
                                                        'channels': k,
                                                        'batch': j,
                                                        'filter_height': n,
                                                        'filter_width': o,
                                                        'top_pad': p,
                                                        'bottom_pad': q,
                                                        'left_pad': r,
                                                        'right_pad': s,
                                                        'stride_d1': t,
                                                        'stride_d2': u
                                                    }

                                                    modify_parameters(ver_script_dir, ver_modifications, patterns=ver_patterns)
                                                    with open(imcol_lib_dir, 'r') as file:
                                                        content = file.read()
                                                    
                                                    mask = generate_mask(4, 2, 2, i)

                                                    # Replace the matched pattern with the new value
                                                    new_content = im2col_lib_pattern.sub(f'#define SPC_CH_MASK 0b{mask}', content)
                                                    if cpu_done == 1:
                                                        new_content = im2col_lib_pattern_cpu_done.sub(f'#define START_ID 3', new_content)
                                                    
                                                    # Write the modified content back to the file
                                                    with open(imcol_lib_dir, 'w') as file:
                                                        file.write(new_content)

                                                    # Run the verification script
                                                    subprocess.run(verification_script_com, shell=True, text=True)
                                                    result = subprocess.run(app_compile_run_com, shell=True, capture_output=True, text=True)
                                                    
                                                    pattern = re.compile(r'im2col NCHW test (\d+) executed')
                                                    pattern_2 = re.compile(r'Total number of cycles: \[(\d+)\]')
                                                    err_pattern = re.compile(r'ERROR')
                                                    err_pattern_small = re.compile(r'error')
                                                    #print(result.stdout)

                                                    # Array to store the cycles for each test
                                                    cycles = []

                                                    file_path = "/home/ubuntu/Desktop/Xheep/Source/x-heep/build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator/uart0.log"

                                                    # Filter and extract the data
                                                    test_number = None
                                                    with open(file_path, 'r') as file:
                                                      for line in file:
                                                        match = pattern.search(line)
                                                        match_2 = pattern_2.search(line)
                                                        err_match = err_pattern.search(line)
                                                        err_match_small = err_pattern_small.search(line)
                                                        if match:
                                                            test_number = match.group(1)
                                                        elif match_2:
                                                            cycle_count = match_2.group(1)
                                                            cycles.append((test_number, cycle_count))
                                                        elif err_match or err_match_small:
                                                            print("ERROR FOUND")
                                                            break
                                                    
                                                    for test, cycle in cycles:
                                                        string = f"CH_SPC: {i}, B: {j}, C: {k}, H: {l}, W: {m}, FH: {n}, FW: {o}, PT: {p}, PB: {q}, PL: {r}, PR: {s}, S1: {t}, S2: {u}, cycles: {cycle}"
                                                        
                                                        if int(test) == 0:
                                                            im2col_cpu.append(string)
                                                        elif int(test) == 2:
                                                            im2col_dma_2d_C.append(string)
                                                        elif int(test) == 3:
                                                            im2col_spc.append(string)
    im2col_cpu_array.append(im2col_cpu)
    im2col_dma_2d_C_array.append(im2col_dma_2d_C)
    im2col_spc_array.append(im2col_spc)

    cpu_done = 1

with open('im2col_data.txt', 'w') as file:
    file.write("im2col_cpu:\n")
    for value in im2col_cpu_array:
        file.write(f"{value}\n")
    file.write("\n")
    
    file.write("im2col_dma_2d_C:\n")
    for value in im2col_dma_2d_C_array:
        file.write(f"{value}\n")
    file.write("\n")
    
    file.write("im2col_spc:\n")
    for value in im2col_spc_array:
        file.write(f"{value}\n")
    file.write("\n")

# Elaborate the data

parsed_1ch_CPU = parse_data(im2col_cpu_array[0])
parsed_1ch_DMA = parse_data(im2col_dma_2d_C_array[0])
parsed_1ch_spc = parse_data(im2col_spc_array[0])
parsed_2ch_spc = parse_data(im2col_spc_array[1])
parsed_3ch_spc = parse_data(im2col_spc_array[2])

add_loop_size(parsed_1ch_CPU)
add_loop_size(parsed_1ch_DMA)
add_loop_size(parsed_1ch_spc)
add_loop_size(parsed_2ch_spc)
add_loop_size(parsed_3ch_spc)

df_1ch_CPU = pd.DataFrame(parsed_1ch_CPU)
df_1ch_DMA = pd.DataFrame(parsed_1ch_DMA)
df_1ch_spc = pd.DataFrame(parsed_1ch_spc)
df_2ch_spc = pd.DataFrame(parsed_2ch_spc)
df_3ch_spc = pd.DataFrame(parsed_3ch_spc)

# Plot the data
plt.figure(figsize=(10, 6))
plt.scatter(df_1ch_CPU['loop_size'], df_1ch_CPU['cycles'], color='blue')
plt.scatter(df_1ch_DMA['loop_size'], df_1ch_DMA['cycles'], color='red')
plt.scatter(df_1ch_spc['loop_size'], df_1ch_spc['cycles'], color='green')
plt.scatter(df_2ch_spc['loop_size'], df_2ch_spc['cycles'], color='orange')
plt.scatter(df_3ch_spc['loop_size'], df_3ch_spc['cycles'], color='purple')
plt.title('Loop Size vs Cycles')
plt.xlabel('Loop Size')
plt.ylabel('Cycles')
plt.grid(True)
plt.show()

