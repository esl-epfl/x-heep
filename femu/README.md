# Building the platform
Start in x-heep's main folder

## Generating the files
1. First activate the conda environment with:

```
conda activate core-v-mini-mcu
```

2. Generate the hdl files by calling:

```
make femu-gen PAD_CFG=femu/pad_cfg.hjson
```

__NOTE__: you can customize the mcu-gen process by providing the MEMORY_BANKS - CPU - BUS parameters to the above command

## Synthesize with Vivado

1. Run the synthesis by calling:

```
make vivado-fpga FPGA_BOARD=pynq-z2-arm-emulation
```


