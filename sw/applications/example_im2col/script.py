import re

# Array to store the cycles for each test
cycles = []

file_path = "/home/ubuntu/Desktop/Xheep/Source/x-heep/build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator/uart0.log"
pattern = re.compile(r'im2col NCHW test (\d+) executed')
pattern_2 = re.compile(r'Total number of cycles: \[(\d+)\]')
err_patterm = re.compile(r'ERROR')
                                                    
# Filter and extract the data
with open(file_path, 'r') as file:
  for line in file:
      print(line)
      match = pattern.search(line)
      match_2 = pattern_2.search(line)
      err_match = err_patterm.search(line)
      if match:
          test_number = match.group(0)
      elif match_2:
          cycle_count = match_2.group(0)
          cycles.append((test_number, cycle_count))
      elif err_match:
          print("ERROR FOUND")
          break

for test, cycle in cycles:
    print(f"Test {test}: {cycle} cycles")