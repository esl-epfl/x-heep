import subprocess
import re
import matplotlib.pyplot as plt

num_channels_dma = 4

batch_max = 2

channels_max = 2

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

im2col_lib_pattern = re.compile(r'#define SPC_CH_NUM \d+')

im2col_cpu_array = []
im2col_dma_2d_C_array = []
im2col_spc_array = []

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


for i in range(1, num_channels_dma):
    print("Number of channels used by SPC", i)
    im2col_cpu = []
    im2col_dma_2d_C = []
    im2col_spc = []

    for j in range(1, batch_max):
        print("Batch size: ", j)

        for k in range(1, channels_max):
            print("Number of input channels: ", k)

            for l in range(10, im_h_max):
                print("Image height: ", l)

                for m in range(10, im_w_max):
                    print("Image width: ", m)

                    for n in range(2, ker_h_max):
                        print("Kernel height: ", n)

                        for o in range(2, ker_w_max):
                            print("Kernel width: ", o)

                            for p in range(1, pad_top_max):
                                print("Pad top: ", p)

                                for q in range(1, pad_bottom_max):
                                    print("Pad bottom: ", q)

                                    for r in range(1, pad_left_max):
                                        print("Pad left: ", r)

                                        for s in range(1, pad_right_max):
                                            print("Pad right: ", s)

                                            for t in range(1, stride_d1_max):
                                                print("Stride d1: ", t)

                                                for u in range(1, stride_d2_max):
                                                    print("Stride d2: ", u)
                                                    
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

                                                    # Replace the matched pattern with the new value
                                                    new_content = im2col_lib_pattern.sub(f'#define SPC_CH_NUM {i}', content)

                                                    # Write the modified content back to the file
                                                    with open(imcol_lib_dir, 'w') as file:
                                                        file.write(new_content)

                                                    # Run the verification script
                                                    subprocess.run(verification_script_com, shell=True, text=False)
                                                    result = subprocess.run(app_compile_run_com, shell=True, capture_output=True, text=True)
                                                    
                                                    pattern = re.compile(r'im2col NCHW test (\d+) executed')
                                                    pattern_2 = re.compile(r'Total number of cycles: \[(\d+)\]')
                                                    err_pattern = re.compile(r'ERROR')
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
                                                        if match:
                                                            test_number = match.group(1)
                                                        elif match_2:
                                                            cycle_count = match_2.group(1)
                                                            cycles.append((test_number, cycle_count))
                                                        elif err_match:
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