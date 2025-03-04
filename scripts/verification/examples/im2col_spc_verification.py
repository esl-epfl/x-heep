#
#     Copyright EPFL contributors.
#     Licensed under the Apache License, Version 2.0, see LICENSE for details.
#     SPDX-License-Identifier: Apache-2.0
#
#     Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
#                             <tommaso.terzano@gmail.com>
#
#     Info: This is a usecase of the VerifHeep tool. It generates a dataset for the im2col function 
#           and the golden result, then it runs the test on the PYNQ-Z2 board and stores the results 
#           in a file. The data can be then read and plotted using the plotter.py script.
#           
#           In order to be run, these steps has to be followed:
#           0. Set the X-Heep configuration to have 4 channels,
#              while there are no limits on the number of master ports
#           1. Run this command to include Verifheep: 
#               export PYTHONPATH="$PYTHONPATH:/<X-Heep project directory in your system>/scripts/verification"
#           2. Perform the synthesis with Vivado and to program the FPGA with the bitstream
#           3. Connect the board using GDB by running X-Heep script "make openOCD_bscan". Close Vivado to avoid conflicts
#

import re
import time
import verifheep
from tqdm import tqdm
import curses
import torch
import torch.nn.functional as F
import numpy as np

# Define the USB port to which the board is connected. Useful because the port may change
USBport = 2

# Define the parameters for the test

datatype = "uint8_t"
range_max = 255

num_masters = 4 # Number of DMA CH
num_slaves = 2 # Number of BUS master ports
max_masters_per_slave = 2 # Maximum number of DMA CH per BUS master port

batch_max = 4
batch_min = 1

channels_max = 4
channels_min = 1

im_h_max = 11
im_h_min = 10

im_w_max = 11
im_w_min = 10

ker_h_max = 5
ker_h_min = 3

ker_w_max = 5
ker_w_min = 3

pad_top_max = 2
pad_top_min = 1
pad_bottom_max = 2
pad_bottom_min = 1
pad_left_max = 2
pad_left_min = 1
pad_right_max = 2
pad_right_min = 1

stride_d1_max = 2
stride_d1_min = 1
stride_d2_max = 2
stride_d2_min = 1

# Calculate the total number of iterations
total_iterations = ((stride_d2_max - stride_d2_min) * (stride_d1_max - stride_d1_min) *
                    (pad_right_max - pad_right_min) * (pad_left_max - pad_left_min) *
                    (pad_bottom_max - pad_bottom_min) * (pad_top_max - pad_top_min) *
                    (ker_w_max - ker_w_min) * (ker_h_max - ker_h_min) *
                    (im_w_max - im_w_min) * (im_h_max - im_h_min) *
                    (channels_max - channels_min) * (batch_max - batch_min))

# Define the patterns to be used for modifying the im2col_lib.h file
spc_mask_pattern = re.compile(r'#define SPC_CH_MASK 0b\d+')
start_id_pattern = re.compile(r'#define START_ID \d+')
test_en_pattern = re.compile(r'#define TEST_EN \d+')

# Define the arrays to store the data
im2col_cpu_array = []
im2col_dma_2d_C_array = []
im2col_spc_array = []

# Function to generate the input dataset for the im2col test to be passed to the VerifHeep tool
def im2col_function(input_array, parameters):
  
    # Extract parameters
    batch_size = parameters['BATCH']
    channels = parameters['CH']
    image_height = parameters['IH']
    image_width = parameters['IW']
    top_pad = parameters['TOP_PAD']
    bottom_pad = parameters['BOTTOM_PAD']
    left_pad = parameters['LEFT_PAD']
    right_pad = parameters['RIGHT_PAD']
    stride_d1 = parameters['STRIDE_D1']
    stride_d2 = parameters['STRIDE_D2']
    (filter_height, filter_width) = (parameters['FH'], parameters['FW'])
    kernel_size = (filter_height, filter_width)

    # Convert the input array into a PyTorch tensor with the correct shape
    input_tensor = torch.tensor(input_array).view(batch_size, channels, image_height, image_width)

    dilation = 1
    # Ensure kernel_size, stride, padding, and dilation are tuples
    if isinstance(kernel_size, int):
        kernel_size = (kernel_size, kernel_size)
    if isinstance(dilation, int):
        dilation = (dilation, dilation)

    # Adjust padding format for F.pad (expects pad_left, pad_right, pad_top, pad_bottom)
    padding_format = (left_pad, right_pad, top_pad, bottom_pad)

    # Apply zero padding
    padded_input = F.pad(input_tensor, padding_format, "constant", 0)

    # Unfold the padded input tensor
    unfolded = padded_input.unfold(2, kernel_size[0], stride_d2).unfold(3, kernel_size[1], stride_d1)
    unfolded = unfolded.permute(0, 2, 3, 1, 4, 5)

    # Reshape to get the 2D tensor where each row is a flattened receptive field
    channel_dim = padded_input.size(1)
    unfolded_tensor = unfolded.contiguous().view(-1, channel_dim * kernel_size[0] * kernel_size[1]).t()

    # Convert the PyTorch tensor to a NumPy array and then to a list (simple array)
    unfolded_array = unfolded_tensor.numpy().flatten().tolist()

    return unfolded_array, ""


# Initialize the VerifHeep tool
im2colVer = verifheep.VerifHeep("pynq-z2", "../../../")

# Connect to the pynq-z2 board
print("Connecting to the board...")
serial_status = im2colVer.serialBegin(f"/dev/ttyUSB{USBport}", 9600)
if not serial_status:
    print("Error connecting to the board")
    exit(1)
else:
    print("Connected!\n")
    time.sleep(1)

# Set up the debug interface
im2colVer.setUpDeb()

def main(stdscr):

  # Initialize the progress bar
  progress_bar = tqdm(total=total_iterations, desc="Overall Progress", ncols=100, unit=" iter",
                   bar_format='{desc}: {percentage:.2f}%|{bar}| {n_fmt}/{total_fmt}')

  iteration = 1
  started = False
  counter = 10

  # Set CH0 to be the SPC channel
  mask = "0001"
  im2colVer.modifyFile("../../../sw/applications/example_im2col/im2col_lib.h", spc_mask_pattern, f'#define SPC_CH_MASK 0b{mask}')
  
  # Set the correct output format by setting the TEST_EN define
  im2colVer.modifyFile("../../../sw/applications/example_im2col/im2col_lib.h", test_en_pattern, f'#define TEST_EN 1')
                                                  
  curses.curs_set(0)
  for j in range(batch_min, batch_max):
      im2col_cpu = []
      im2col_dma_2d_C = []
      im2col_spc = []
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
                                                  
                                                  # Start the chrono
                                                  if started and counter == 0:
                                                    im2colVer.stopDeb()
                                                    im2colVer.setUpDeb()
                                                    counter = 10
                                                  elif not started:
                                                    im2colVer.setUpDeb()
                                                    started = True
                                                    counter -= 1
                                                  else:                                                      
                                                    counter -= 1
                                                  
                                                  im2colVer.chronoStart()

                                                  # Generate the input dataset and the golden result
                                                  n_patches_h = (l + p + q - n) // u + 1
                                                  n_patches_w = (m + s + r - o) // t + 1
                                                  OH = o * n * k * j # Number of rows in a column -> size of a column
                                                  OW = n_patches_h * n_patches_w # Numver of columns in a row -> size of a row
                                                  input_size = k * l * m * j
                                                  golden_size = OH * OW

                                                  parameters = {
                                                      'IH': l,
                                                      'IW': m,
                                                      'CH': k,
                                                      'BATCH': j,
                                                      'FH': n,
                                                      'FW': o,
                                                      'TOP_PAD': p,
                                                      'BOTTOM_PAD': q,
                                                      'LEFT_PAD': r,
                                                      'RIGHT_PAD': s,
                                                      'STRIDE_D1': t,
                                                      'STRIDE_D2': u
                                                  }
                                                  
                                                  im2colVer.genInputDataset(input_size, row_size=m, range_max=range_max, dataset_dir_c="../../../sw/applications/example_im2col/im2col_input.c", 
                                                                            dataset_dir="../../../sw/applications/example_im2col/im2col_input.h", parameters=parameters, dataset_name="input_image_nchw",
                                                                            datatype=datatype)
                                                  
                                                  im2colVer.genGoldenResult(im2col_function, golden_size, parameters, row_size=OW, golden_dir="../../../sw/applications/example_im2col/im2col_golden.h", 
                                                                            golden_dir_c="../../../sw/applications/example_im2col/im2col_golden.c", input_dataset_dir="../../../sw/applications/example_im2col/im2col_input.c",
                                                                            golden_name="golden_im2col_nchw",
                                                                            output_datatype=datatype)
                                                                                                  
                                                  im2colVer.modifyFile("../../../sw/applications/example_im2col/im2col_lib.h", start_id_pattern, f'#define START_ID 0')
                                                  
                                                  # Launch the test
                                                  im2colVer.launchTest("example_im2col", input_size=j*k*l*m)

                                                  # Format the parameters of the current run and store them for plots
                                                  for test in im2colVer.results:
                                                      string = f'CH_SPC: 1, B: {j}, C: {k}, H: {l}, W: {m}, FH: {n}, FW: {o}, PT: {p}, PB: {q}, PL: {r}, PR: {s}, S1: {t}, S2: {u}, cycles: {test["Cycles"]}'
                                                      
                                                      if int(test["ID"]) == 0:
                                                          im2col_cpu.append(string)
                                                      elif int(test["ID"]) == 1:
                                                          im2col_dma_2d_C.append(string)
                                                      elif int(test["ID"]) == 2:
                                                          im2col_spc.append(string)
                                                  
                                                  # Stop the chrono and calculate the remaining time of the verification
                                                  im2colVer.clearResults()
                                                  im2colVer.chronoStop()
                                                  time_rem = im2colVer.chronoExecutionEst(((stride_d2_max - stride_d2_min) * (stride_d1_max - stride_d1_min) * (pad_right_max - pad_right_min) * (pad_left_max - pad_left_min) * (pad_bottom_max - pad_bottom_min) * (pad_top_max - pad_top_min) * (ker_w_max - ker_w_min) * (ker_h_max - ker_h_min) * (im_w_max - im_w_min) * (im_h_max - im_h_min) * (channels_max - channels_min) * (batch_max - batch_min)))
                                                  
                                                  # Update the progress bar
                                                  message = (
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

      im2col_cpu_array.append(im2col_cpu)
      im2col_dma_2d_C_array.append(im2col_dma_2d_C)
      im2col_spc_array.append(im2col_spc)

  # Stop the debug interface and close the progress bar
  im2colVer.stopDeb()
  progress_bar.close()

  # Write the data to a file
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