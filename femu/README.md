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

1. Download the Linux image version v3.0.1 using the following link:

http://www.pynq.io/board.html

2. Follow the procedure explained in the following linked page to flash the downloaded image to the SD card:

https://pynq.readthedocs.io/en/v2.2.1/appendix.html#writing-the-sd-card

3. Insert the SD into the Pynq-Z2 board and make sure the booting jumper is on the SD position

4. Power-on the board and wait for Linux to boot

## Connect to Linux running on the Pynq-Z2

Connect your Linux-based PC to the Pynq-Z2 board using an Ethernet cable and run the following command:

```
ssh -X xilinx@board_ip
```

## Install OpenOCD

1. Clone the OpenOCD repository usign the following link:

https://github.com/openocd-org/openocd

2. Run the following commands to install it:

```
sudo ./bootstrap
sudo ./configure --enable-sysfsgpio
sudo make
sudo make install
```

## Copy the x-heep folder to the Pynq-Z2 baord

Copy the x-heep/femu/arm/ folder from your PC to the home directory of the Pynq-Z2 board with the following command:

```
scp -r x-heep/femu/arm/ xilinx@board_ip:/~
```

## Enable UART1 on Linux

Enter the arm/uart_enable/ folder on and run the following commands:

```
sudo su
mkdir -p /configfs
./uart_enable.sh
exit
```

## Prepare the needed shells

You need to use 5 shells:

1. First shell - Run virtual flash app on the Pynq-Z2 board - browse to the arm/virtual_flash/ folder and execute the following commands to compile and run the required application:

```
sudo make clean
sudo make
sudo ./virtual_flash
```

This app allocates a buffer into the off-chip DDR memory of the Pynq-Z2 board and stores its physical base address to the hijacker PL peripheral.

2. Second shell - Run the following commands on the Pynq-Z2 board to get the stdout from the app running on x-heep:

```
sudo apt-get install screen
sudo screen /dev/ttyPS1 115200
```

3. Third shell - Run OpenOCD on the Pynq-Z2 board - browse to the arm/openocd_cfg/ folder and run the following command:

```
sudo openocd -f ./gpio_bitbang.cfg
```

4. Fourth shell - Compile x-heep app on your PC - browse to the x-heep/sw/ folder and run the following command:

```
make clean applications/example_virtual_flash/example_virtual_flash.hex TARGET=pynq-z2
```

With this command you compile a sample RISC-V based application that uses the allocated DDR-based virtual memory. You may want to change this command with the name of your own application.

5. Fifth shell - Run GDB on your PC - browse to the x-heep/sw/applications/example_virtual_flash/ folder and run the following command to connect GDB:

```
sudo /path_to_your_riscv_toolchain/bin/riscv32-unknown-elf-gdb
```

Use the following GDB commands to connect to OpenOCD running on the Pynq-Z2 board and run your application:

```
(GDB) target remote board_ip:3333
(GDB) load example_virtual_flash.elf
(GDB) cont
```

Go back to the first shell and press ENTER to end the application and dump the content of the virtual flash to a file. Then, run the following commands:

```
xxd dump.txt dump
nano dump
```

Check if your result corresponds to the following lines:

```
00000000: 0000 0000 0000 0001 0000 0002 0000 0003  ................
00000010: 0000 0004 0000 0005 0000 0006 0000 0007  ................
00000020: 0000 0008 0000 0009 0000 000a 0000 000b  ................
00000030: 0000 000c 0000 000d 0000 000e 0000 000f  ................
00000040: 0000 0010 0000 0011 0000 0012 0000 0013  ................
00000050: 0000 0014 0000 0015 0000 0016 0000 0017  ................
00000060: 0000 0018 0000 0019 0000 001a 0000 001b  ................
00000070: 0000 001c 0000 001d 0000 001e 0000 001f  ................
00000080: 0000 0000 0000 0000 0000 0000 0000 0000  ................
```
