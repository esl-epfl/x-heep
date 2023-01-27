# Build the platform
Start in the x-heep main folder

## Generate the files
1. Activate the conda environment with:

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

Open the Vivado project and use the Hardware Manager to program the bitstream to the Zynq 7020 of the Pynq-Z2 board

## Flash the SD card with our pre-compiled Linux image

1. Download our Linux image using the following link:

   --- link available soon ---

2. Follow the procedure explained in the following linked page to flash the downloaded image to the SD card:

   https://pynq.readthedocs.io/en/v2.2.1/appendix.html#writing-the-sd-card

3. Insert the SD into the Pynq-Z2 board and make sure the booting jumper is on the SD position

4. Power-on the board and wait for Linux to boot

__NOTE__: the provided Linux image contains a pre-installed version of the RISC-V toolchain and OpenOCD

## Connect to Linux running on the Pynq-Z2

Connect your Linux-based PC to the Pynq-Z2 board using an Ethernet cable and run the following command:

```
ssh -X xilinx@192.168.2.99
```

## Copy the needed folder to the Pynq-Z2 baord

Copy the x-heep repository folder from your PC to the home directory of the Pynq-Z2 board

## Enable UART1 on Linux

Enter the x-heep/femu/arm/script/ folder on and run the following command:

```
source ./uart_enable.sh
```

## Prepare the needed shells

You need to use 4 shells:

1. First shell - Run virtual flash app: browse to the x-heep/femu/arm/virtual_flash/ folder and execute the following commands to compile and run the required application:

```
sudo make clean
sudo make
sudo ./virtual_flash
```

This app allocates a buffer into the off-chip DDR memory of the Pynq-Z2 board and stores its physical base address to the hijacker PL peripheral

2. Second shell - Compile x-heep app: browse to the x-heep/sw/ folder and run the following command:

```
make clean applications/example_virtual_flash/example_virtual_flash.hex
```

With this command you compile an sample RISC-V based application that uses the allocated DDR-based virtual memory. You may want to change this command with the name of your own application.

3. Third shell - Run OpenOCD: browse to the x-heep/femu/arm/openocd_cfg/ folder and run the following command:

```
sudo openocd -f ./gpio_bitbang.cfg
```

4. Fourth shell - Run GDB: browse to the x-heep/sw/ folder and run the following command to connect GDB:

```
/path_to_your_riscv_toolchain/riscv/bin/riscv32-unknown-elf-gdb ./applications/example_virtual_flash/example_virtual_flash.elf
```

Use GDB commands to interact with the x-heep architecture and run/debug your application.
