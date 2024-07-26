import subprocess
import re
import time
import serial
import pexpect
import threading
import queue
import verifheep_lib
from tqdm import tqdm
import curses

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

app_compile_run_com = " cd ../../../ ; make app PROJECT=example_im2col TARGET=pynq_z2 "

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

cpu_done = 0
iteration = 0
execution_times = []
start_loop_time = time.time()

im2colVer = verifheep_lib.VerifHeep("pynq-z2", "../../../")

print("Connecting to the board...")
serial_status = im2colVer.serialBegin("/dev/ttyUSB2", 9600)
if not serial_status:
    print("Error connecting to the board")
    exit()
else:
    print("Connected!\n")
    time.sleep(1)
im2colVer.setUpDeb()

total_iterations = ((stride_d2_max - stride_d2_min) * (stride_d1_max - stride_d1_min) *
                    (pad_right_max - pad_right_min) * (pad_left_max - pad_left_min) *
                    (pad_bottom_max - pad_bottom_min) * (pad_top_max - pad_top_min) *
                    (ker_w_max - ker_w_min) * (ker_h_max - ker_h_min) *
                    (im_w_max - im_w_min) * (im_h_max - im_h_min) *
                    (channels_max - channels_min) * (batch_max - batch_min) *
                    (num_channels_dma - num_channels_dma_min))

def main(stdscr):
  progress_bar = tqdm(total=total_iterations, desc="Overall Progress", ncols=100, unit=" iter",
                    bar_format='{desc}: {percentage:.2f}%|{bar}| {n_fmt}/{total_fmt}')

  cpu_done = 0
  iteration = 1
  curses.curs_set(0)
  for i in range(num_channels_dma_min, num_channels_dma):
      print("_______________________\n\r")
      print("Number of channels used by SPC\n\r", i)
      print("_______________________\n\n\r")
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
                                                      
                                                      im2colVer.chronoStart()
                                                      
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
                                                      
                                                      # Launch the test
                                                      im2colVer.launchTest("example_im2col", input_size=j*k*l*m)

                                                      for test in im2colVer.results:
                                                          string = f'CH_SPC: {i}, B: {j}, C: {k}, H: {l}, W: {m}, FH: {n}, FW: {o}, PT: {p}, PB: {q}, PL: {r}, PR: {s}, S1: {t}, S2: {u}, cycles: {test["Cycles"]}'
                                                          
                                                          if int(test["ID"]) == 0:
                                                              im2col_cpu.append(string)
                                                          elif int(test["ID"]) == 2:
                                                              im2col_dma_2d_C.append(string)
                                                          elif int(test["ID"]) == 3:
                                                              im2col_spc.append(string)
                                                      
                                                      im2colVer.clearResults()
                                                      im2colVer.chronoStop()
                                                      time_rem = im2colVer.chronoExecutionEst(((stride_d2_max - stride_d2_min) * (stride_d1_max - stride_d1_min) * (pad_right_max - pad_right_min) * (pad_left_max - pad_left_min) * (pad_bottom_max - pad_bottom_min) * (pad_top_max - pad_top_min) * (ker_w_max - ker_w_min) * (ker_h_max - ker_h_min) * (im_w_max - im_w_min) * (im_h_max - im_h_min) * (channels_max - channels_min) * (batch_max - batch_min) * (num_channels_dma - num_channels_dma_min)))
                                                      
                                                      message = (
                                                          f"SPC channels:  {i:>5}\n"
                                                          f"Batch size:    {j:>5}\n"
                                                          f"Input channels:{k:>5}\n"
                                                          f"Image height:  {l:>5}\n"
                                                          f"Image width:   {m:>5}\n"
                                                          f"Kernel height: {n:>5}\n"
                                                          f"Kernel width:  {o:>5}\n"
                                                          f"Pad top:       {p:>5}\n"
                                                          f"Pad bottom:    {q:>5}\n"
                                                          f"Pad left:      {r:>5}\n"
                                                          f"Pad right:     {s:>5}\n"
                                                          f"Stride d1:     {t:>5}\n"
                                                          f"Stride d2:     {u:>5}\n"
                                                          f"Remaining time:{time_rem['hours']:>2}h:{time_rem['minutes']:>2}m:{time_rem['seconds']:.2f}s\n"
                                                      )

                                                      iteration += 1
                                                      progress_bar.update(1)
                                                      
                                                      stdscr.addstr(1, 0, message)
                                                      stdscr.refresh()

      if (not cpu_done):
          im2col_cpu_array.append(im2col_cpu)
          im2col_dma_2d_C_array.append(im2col_dma_2d_C)
      im2col_spc_array.append(im2col_spc)
      print(im2col_cpu)
      cpu_done = 1

  im2colVer.stopAll()
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
main()