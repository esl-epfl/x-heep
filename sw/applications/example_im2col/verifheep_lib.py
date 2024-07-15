#
#     Copyright EPFL contributors.
#     Licensed under the Apache License, Version 2.0, see LICENSE for details.
#     SPDX-License-Identifier: Apache-2.0
#
#     Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
#                             <tommaso.terzano@gmail.com>
#
#     Info: This library can be used to implement a software-based verification of X-Heep 
#           on a FPGA target, using gdb and OpenOCD. 
#           Some useful information:
#           1) By default, this is the format of the data that the SW testbench should produce:
#              - 1st line: "S"
#              - 2nd line to n line: "<ID>:<cycles>:<outcome>"
#              - n+1 line: "&"
#           

import subprocess
import re
import time
import serial
import pexpect
import threading
import queue

class VerifHeep:
    def __init__(self, target, xheep_dir):
        self.target = target
        if target != 'pynq-z2':
            raise Exception(f'Target {target} not supported. Choose one among:\n- "pynq-z2"\n')
        self.xheep_dir = xheep_dir
        self.results = []
    
    def setUpSynth(self, mem_banks=6, cpu="cv32e40px", bus="1toN"):
        mcu_gen_cmd = f"cd {self.xheep_dir} ; make mcu-gen MEMORY_BANKS={mem_banks} CPU={cpu} BUS={bus}"
        subprocess.run(mcu_gen_cmd, shell=True, capture_output=True, text=True)
        if ("ERROR" in mcu_gen_cmd.stderr) or ("error" in mcu_gen_cmd.stderr):
            print(mcu_gen_cmd.stderr)
            exit(1)
    
    def launchSynth(self, scan_chains_en=0):
        synth_cmd = f"cd {self.xheep_dir} ; make vivado-fpga FPGA_BOARD={self.target}"
        if scan_chains_en:
            synth_cmd += " FUSESOC_FLAGS=--flag=use_bscane_xilinx"
        result_synth = subprocess.run(synth_cmd, shell=True, capture_output=True, text=True)
        if ("ERROR" in result_synth.stderr) or ("error" in result_synth.stderr):
            print(result_synth.stderr)
            exit(1)
    
    def serialBegin(self, port, baudrate):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        self.serial_queue = queue.Queue()

    def setUpDeb(self):
        openocd_cmd = ['openocd', '-f', f'./tb/core-v-mini-mcu-{self.target}-esl-programmer.cfg']
        self.openocd_proc = subprocess.Popen(openocd_cmd)     
        time.sleep(2)
        gdb_cmd = ['$RISCV/bin/riscv32-unknown-elf-gdb', './sw/build/main.elf']
        self.gdb = pexpect.spawn(' '.join(gdb_cmd))
        self.gdb.expect('(gdb)')
        self.gdb.sendline('target remote localhost:3333')
        self.gdb.expect('(gdb)')

    def launchTest(self, example_name, input_size=0, pattern=r'(\d+):(\d+):(\d+)', en_timeout_term=0):
        # Set up the serial communication thread and attach it the serial queue
        self.serial_thread = threading.Thread(target=SerialReceiver, args=(self.serial_queue,))

        # Compile the application
        app_compile_run_com = f"cd {self.xheep_dir} ; make app PROJECT={example_name} TARGET={self.target}"
        result_compilation = subprocess.run(app_compile_run_com, shell=True, capture_output=True, text=True)
        if ("Error" in result_compilation.stderr) or ("error" in result_compilation.stderr):
            print(result_compilation.stderr)
            return
        
        # Run the testbench with gdb
        self.gdb.sendline('load')
        self.gdb.expect('(gdb)')

        # Set a breakpoint at the exit and wait for it
        self.gdb.sendline('b _exit')
        self.gdb.expect('(gdb)')
        self.gdb.sendline('continue')
        try:
          self.gdb.expect('Breakpoint', timeout=600)
        except pexpect.TIMEOUT:
          print("Timeout! Program didn't answer in time, exiting...")
          self.gdb.terminate()
          self.openocd_proc.terminate()
          if en_timeout_term:
            exit(1)
          return
        
        # Wait for serial to finish
        self.serial_thread.join()

        # Recover the lines
        lines = []
        while not self.serial_queue.empty():
            lines.append(self.serial_queue.get())

        # Analyse the results
        pattern = re.compile(pattern)  
        test_id = None
        for line in lines:
            match = pattern.search(line)
            if match:
                test_id = match.group(1)
                cycle_count = match.group(2)
                outcome = match.group(3)
                self.results.append({ "ID" : test_id, "Cycles": cycle_count, "Outcome": outcome, "Input size": input_size })

    def dumpResults(self, filename="results.txt"):
        with open(filename, 'w') as f:
            for result in self.results:
                f.write(result + '\n')
    
    def stopAll(self):
        self.gdb.sendcontrol('c')
        self.gdb.terminate()
        self.gdb.logfile.close()
        self.openocd_proc.terminate()
    
    def chronoStart(self):
        self.start_time = time.time()
    
    def chronoStop(self):
        self.end_time = time.time()
        self.it_times.append(self.end_time - self.start_time)
        return self.end_time - self.start_time
    
    def chronoExecutionEst(self, loop_size):
        avg_duration = sum(self.it_times) / len(self.it_times)
        remaining_it = loop_size - len(self.it_times)
        remaining_time_raw = remaining_it * avg_duration
        remaining_time = []
        remaining_time["hours"], remainder = divmod(remaining_time_raw, 3600)
        remaining_time["minutes"], remaining_time["seconds"] = divmod(remainder, 60)
        return remaining_time

def SerialReceiver(ser, serial_queue, endword="&"):
  try:
      received = False
      lines = []
      while not received:
          # Leggi una riga di dati dalla porta seriale
          line = ser.readline().decode('utf-8').rstrip()
          serial_queue.put(line)
          if line:
              print(f"Ricevuto: {line}")
              if line == endword:
                received = True
                return
  except KeyboardInterrupt:
      print("Interruzione da tastiera. Chiudo la porta seriale.")
  finally:
      ser.close()