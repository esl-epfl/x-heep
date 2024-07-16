import subprocess
import re
import time
from tqdm import tqdm
import curses

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

num_masters = 4
num_slaves = 3
max_masters_per_slave = 2

num_channels_dma = 5
num_channels_dma_min = 1

batch_max = 4
batch_min = 1

channels_max = 4
channels_min = 1

im_h_max = 13
im_h_min = 10

im_w_max = 13
im_w_min = 10

ker_h_max = 5
ker_h_min = 3

ker_w_max = 5
ker_w_min = 3

pad_top_max = 3
pad_top_min = 1
pad_bottom_max = 3
pad_bottom_min = 1
pad_left_max = 3
pad_left_min = 1
pad_right_max = 3
pad_right_min = 1

stride_d1_max = 2
stride_d1_min = 1
stride_d2_max = 2
stride_d2_min = 1

ver_script_dir = "verification_script.py"

imcol_lib_dir = "im2col_lib.h"

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
im2col_lib_pattern_test = re.compile(r'#define TEST_EN \d+')

total_iterations = ((stride_d2_max - stride_d2_min) * (stride_d1_max - stride_d1_min) *
                    (pad_right_max - pad_right_min) * (pad_left_max - pad_left_min) *
                    (pad_bottom_max - pad_bottom_min) * (pad_top_max - pad_top_min) *
                    (ker_w_max - ker_w_min) * (ker_h_max - ker_h_min) *
                    (im_w_max - im_w_min) * (im_h_max - im_h_min) *
                    (channels_max - channels_min) * (batch_max - batch_min) *
                    (num_channels_dma - num_channels_dma_min))

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


def main(stdscr):
  
  progress_bar = tqdm(total=total_iterations, desc="Overall Progress", ncols=100, unit=" iter")

  cpu_done = 0
  iteration = 1
  execution_times = []
  start_loop_time = time.time()
  curses.curs_set(0)
  for i in range(num_channels_dma_min, num_channels_dma):
      im2col_cpu = []
      im2col_dma_2d_C = []
      im2col_spc = []

      for j in range(batch_min, batch_max):

          for k in range(channels_min, channels_max):

              for l in range(im_h_min, im_h_max):

                  for m in range(im_w_min, im_w_max):

                      for n in range(ker_h_min, ker_h_max):

                          for o in range(ker_w_min, ker_w_max):

                              for p in range(pad_top_min, pad_top_max):

                                  for q in range(pad_bottom_min, pad_bottom_max):

                                      for r in range(pad_left_min, pad_left_max):

                                          for s in range(pad_right_min, pad_right_max):

                                              for t in range(stride_d1_min, stride_d1_max):

                                                  for u in range(stride_d2_min, stride_d2_max):
                                                      start_time = time.time()

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
                                                      
                                                      mask = generate_mask(num_masters, num_slaves, max_masters_per_slave, i)

                                                      progress = (iteration)/((stride_d2_max - stride_d2_min) * (stride_d1_max - stride_d1_min) * (pad_right_max - pad_right_min) * (pad_left_max - pad_left_min) * (pad_bottom_max - pad_bottom_min) * (pad_top_max - pad_top_min) * (ker_w_max - ker_w_min) * (ker_h_max - ker_h_min) * (im_w_max - im_w_min) * (im_h_max - im_h_min) * (channels_max - channels_min) * (batch_max - batch_min) * (num_channels_dma - num_channels_dma_min)) * 100
                                                                                                          
                                                      # Replace the matched pattern with the new value
                                                      new_content = im2col_lib_pattern.sub(f'#define SPC_CH_MASK 0b{mask}', content)
                                                      
                                                      new_content = im2col_lib_pattern_test.sub(f'#define TEST_EN 1', new_content)
                                                      
                                                      if cpu_done == 1:
                                                          new_content = im2col_lib_pattern_cpu_done.sub(f'#define START_ID 3', new_content)
                                                      else:
                                                          new_content = im2col_lib_pattern_cpu_done.sub(f'#define START_ID 0', new_content)
                                                      # Write the modified content back to the file
                                                      with open(imcol_lib_dir, 'w') as file:
                                                          file.write(new_content)

                                                      # Run the verification script
                                                      subprocess.run(verification_script_com, shell=True, text=True)
                                                      result = subprocess.run(app_compile_run_com, shell=True, capture_output=True, text=True)
                                                      
                                                      pattern = re.compile(r'(\d+):(\d+):(\d+)')
                                                      err_pattern = re.compile(r'ERROR')
                                                      #print(result.stdout)

                                                      # Array to store the cycles for each test
                                                      cycles = []

                                                      file_path = "../../../build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator/uart0.log"

                                                      # Filter and extract the data
                                                      test_number = None
                                                      with open(file_path, 'r') as file:
                                                        for line in file:
                                                          match = pattern.search(line)
                                                          err_match = err_pattern.search(line)
                                                          if match:
                                                              test_number = match.group(1)
                                                              cycle_count = match.group(2)
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
                                                      
                                                      end_time = time.time()
                                                      cycle_time = end_time - start_time
                                                      execution_times.append(cycle_time)
                                                      total_time_elapsed = time.time() - start_loop_time
                                                      total_estimated_time = total_time_elapsed / progress * 100
                                                      estimated_remaining_time = total_estimated_time - total_time_elapsed
                                                      hours, remainder = divmod(estimated_remaining_time, 3600)
                                                      minutes, seconds = divmod(remainder, 60)

                                                      message = (f"SPC channels: {i}\nBatch size: {j}\nInput channels: {k}\nImage height: {l}\nImage width: {m}"
                                                            f"\nKernel height: {n}\nKernel width: {o}\nPad top: {p}\nPad bottom: {q}\n"
                                                            f"Pad left: {r}\nPad right: {s}\nStride d1: {t}\nStride d2: {u}\nRemaining time: {hours}h:{minutes}m:{seconds:.2f}s")
                                                      
                                                      stdscr.addstr(1, 0, message)
                                                      stdscr.refresh()

                                                      iteration += 1
                                                      progress_bar.update(1)
                                                      
                                                      
      if (not cpu_done):
          im2col_cpu_array.append(im2col_cpu)
          im2col_dma_2d_C_array.append(im2col_dma_2d_C)
      im2col_spc_array.append(im2col_spc)
      print(im2col_cpu)
      cpu_done = 1

  progress_bar.close()

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

  print("Data acquired!\n")

curses.wrapper(main)