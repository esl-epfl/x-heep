#
# DMA subsystem configuration
#

import os
import copy
import subprocess
import hjson

current_dir = os.getcwd()

print("Generting DMA registers RTL...")

with open(current_dir + "/mcu_cfg.hjson", "r") as file:
  mcu_cfg = hjson.load(file, use_decimal=True)
  
with open(current_dir + "/hw/ip/dma/data/dma.hjson", "r") as file:
  dma_registers_cfg = hjson.load(file, use_decimal=True)

# Generating DMA subsystem configuration masks
dma_ch_count = mcu_cfg["ao_peripherals"]["dma"]["num_channels"]
dma_fifo_depth = mcu_cfg["ao_peripherals"]["dma"]["fifo_depth"]
dma_addr_mode = mcu_cfg["ao_peripherals"]["dma"]["addr_mode_en"]
dma_subaddr_mode = mcu_cfg["ao_peripherals"]["dma"]["subaddr_mode_en"]
dma_hw_fifo_mode = mcu_cfg["ao_peripherals"]["dma"]["hw_fifo_mode_en"]
dma_zero_padding = mcu_cfg["ao_peripherals"]["dma"]["zero_padding_en"]

# Generating DMA registers depending on the configuration
dma_data_dir = current_dir + "/hw/ip/dma/data/dma_gen.hjson"
padding_params = {"PAD_TOP", "PAD_BOTTOM", "PAD_RIGHT", "PAD_LEFT"}

dma_registers_cfg_cpy = copy.deepcopy(dma_registers_cfg)
dma_ch_registers = dma_registers_cfg_cpy['registers']

if dma_addr_mode == "no":
    dma_ch_registers = [d for d in dma_ch_registers if d.get('name') != "ADDR_PTR"]

if dma_zero_padding == "no":
    dma_ch_registers = [d for d in dma_ch_registers if d.get('name') not in padding_params]

dma_registers_cfg_cpy['registers'] = dma_ch_registers

with open(dma_data_dir, "w") as file:
    hjson.dump(dma_registers_cfg_cpy, file)

command = f'cd {current_dir}/hw/ip/dma/ && source dma_gen.sh'
subprocess.run(["bash", "-c", command])

print("Generted DMA registers RTL!")
