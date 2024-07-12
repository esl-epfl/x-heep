import subprocess
import re
import time
import serial
import pexpect
import threading
import queue

# Configura la porta seriale
ser = serial.Serial(
    port='/dev/ttyUSB2',  # Sostituisci con il tuo dispositivo seriale
    baudrate=9600,        # Sostituisci con il baud rate appropriato
    timeout=1             # Tempo di timeout in secondi
)

# Step 1: Avvia OpenOCD in un processo separato
openocd_cmd = ['openocd', '-f', './tb/core-v-mini-mcu-pynq-z2-esl-programmer.cfg']
openocd_proc = subprocess.Popen(openocd_cmd)

# Attendi che OpenOCD si avvii completamente
time.sleep(2)

# Step 2: Avvia GDB in un altro processo
gdb_cmd = ['$RISCV/bin/riscv32-unknown-elf-gdb', './sw/build/main.elf']
gdb = pexpect.spawn(' '.join(gdb_cmd))

# Coda per memorizzare le linee lette dalla porta seriale
serial_queue = queue.Queue()

gdb.expect('(gdb)')
gdb.sendline('target remote localhost:3333')
gdb.expect('(gdb)')

def RunGdb():
  gdb.sendline('load')

  gdb.expect('(gdb)')
  gdb.sendline('b _exit')

  gdb.expect('(gdb)')
  gdb.sendline('continue')

  try:
    gdb.expect('Breakpoint', timeout=600)
  except pexpect.TIMEOUT:
    print("Timeout! Program didn't answer in time, exiting...")
    gdb.terminate()
    openocd_proc.terminate()
    exit(1)

def StopGdb():
  gdb.sendcontrol('c')

  # Chiudi il processo GDB
  gdb.terminate()

  # Chiudi il log
  gdb.logfile.close()

  # Termina il processo OpenOCD
  openocd_proc.terminate()

def SerialReceiver(serial_queue):
  try:
      received = False
      lines = []
      while not received:
          # Leggi una riga di dati dalla porta seriale
          line = ser.readline().decode('utf-8').rstrip()
          serial_queue.put(line)
          if line:
              print(f"Ricevuto: {line}")
              if line == "END":
                received = True
                return
  except KeyboardInterrupt:
      print("Interruzione da tastiera. Chiudo la porta seriale.")
  finally:
      ser.close()

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
                                                    start_time = time.time()

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
                                                    print("Mask: ", mask)
                                                    print("CPU done: ", cpu_done)
                                                    
                                                    print("_______________________\n\n")
                                                    iteration += 1
                                                    progress = (iteration)/((stride_d2_max - stride_d2_min) * (stride_d1_max - stride_d1_min) * (pad_right_max - pad_right_min) * (pad_left_max - pad_left_min) * (pad_bottom_max - pad_bottom_min) * (pad_top_max - pad_top_min) * (ker_w_max - ker_w_min) * (ker_h_max - ker_h_min) * (im_w_max - im_w_min) * (im_h_max - im_h_min) * (channels_max - channels_min) * (batch_max - batch_min) * (num_channels_dma - num_channels_dma_min)) * 100
                                                    
                                                    print("Progress: {:.2f}%".format(progress))

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
                                                    
                                                    # Crea e avvia i thread per GDB e la lettura seriale
                                                    gdb_thread = threading.Thread(target=RunGdb)
                                                    serial_thread = threading.Thread(target=SerialReceiver, args=(serial_queue,))

                                                    gdb_thread.start()
                                                    serial_thread.start()

                                                    # Aspetta che entrambi i thread terminino
                                                    gdb_thread.join()
                                                    serial_thread.join()

                                                    # Recupera tutte le linee lette dalla porta seriale
                                                    lines = []
                                                    while not serial_queue.empty():
                                                        lines.append(serial_queue.get())

                                                    pattern = re.compile(r'ID:(\d+)')
                                                    pattern_2 = re.compile(r'c:(\d+)')
                                                    err_pattern = re.compile(r'ERROR')
                                                    err_pattern = re.compile(r'ERROR', re.IGNORECASE)

                                                    # Array per memorizzare i cicli per ogni test
                                                    cycles = []

                                                    # Filtra ed estrae i dati
                                                    test_number = None
                                                    for line in lines:
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
                                                    
                                                    print(cycles)

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
                                                    average_time = sum(execution_times) / len(execution_times)
                                                    total_time_elapsed = time.time() - start_loop_time
                                                    total_estimated_time = total_time_elapsed / progress * 100
                                                    estimated_remaining_time = total_estimated_time - total_time_elapsed
                                                    hours, remainder = divmod(estimated_remaining_time, 3600)
                                                    minutes, seconds = divmod(remainder, 60)

                                                    print(f"Cycle time: {cycle_time:.2f}s")
                                                    print(f"Average time: {average_time:.2f}s")
                                                    print(f"Remaining time: {hours}h:{minutes}m:{seconds:.2f}s")

                                                    print("_______________________\n")
    if (not cpu_done):
        im2col_cpu_array.append(im2col_cpu)
        im2col_dma_2d_C_array.append(im2col_dma_2d_C)
    im2col_spc_array.append(im2col_spc)
    print(im2col_cpu)
    cpu_done = 1

StopGdb()

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