# Linux-FEMU

In this version, the X-HEEP architecture is implemented on the programmable logic (PL) side of the Zynq 7020 chip and Linux is run on the ARM-based processing system (PS) side of the same chip. The X-HEEP JTAG signals are driven by the PS GPIO peripheral and are used to program and debug the architecture, while the X-HEEP UART is connected to the PS UART peripheral and is used to get the stdout of the program running on the architecture. Moreover, the X-HEEP flash SPI is connected to the ARM AXI bus passing through an SPI to AXI bridge: this allows virtualising the flash memory into the on-board DDR RAM memory. In this way, the program running on the architecture can read the external virtualised flash as it was a real flash memory, i.e., this virtualisation is totally transparent to the X-HEEP code.

Go through the following steps to build and use our Linux-FEMU version.

## Build the platform

Start in the x-heep main folder.

## Generate the files

1. Activate the conda environment with:

```
conda activate core-v-mini-mcu
```

2. Generate the hdl files by calling:

```
make linux-femu-gen PAD_CFG=linux_femu/pad_cfg.hjson
```

__NOTE__: you can customize the mcu-gen process by providing the MEMORY_BANKS - CPU - BUS parameters to the above command.

## Flash the SD card with a Linux image

1. Download the Linux image (version v3.0.1) using the following link:

`http://www.pynq.io/board.html`

2. Follow the procedure explained in the following linked page to flash the downloaded image to the SD card:

`https://pynq.readthedocs.io/en/v2.2.1/appendix.html#writing-the-sd-card`

3. Insert the SD into the Pynq-Z2 board and make sure the booting bridge is on the SD position.

4. Power-on the board and wait for Linux to boot (a row of 4 leds should turn on).

## Create the Vivado project and generate the bitstream

1. Starting in the x-heep main folder, run the following command to create the Vivado project and generate the bitstream:

```
make vivado-fpga FPGA_BOARD=pynq-z2-arm-emulation
```

2. Open the project x-heep/build/openhwgroup.org_systems_core-v-mini-mcu_0/pynq-z2-arm-emulation-vivado/openhwgroup.org_systems_core-v-mini-mcu_0.xpr with Vivado and use the Hardware Manager to program the bitstream to the Zynq 7020 of the Pynq-Z2 board.

## Connect to Linux running on the Pynq-Z2 board

Connect your Linux-based PC to the Pynq-Z2 board through Ethernet and run the following command:

```
ssh -X xilinx@board_ip
```

The default password is: xilinx

If you need additional documentation on how to connect your PC to the Pynq-Z2 board, use the following link:

`https://pynq.readthedocs.io/en/v2.6.1/getting_started/pynq_z2_setup.html`

## Install OpenOCD on the Pynq-Z2 board

1. Clone the OpenOCD repository usign the following link:

`https://github.com/openocd-org/openocd`

2. Run the following commands from the OpneOCD main folder to install it:

```
sudo ./bootstrap
sudo ./configure --enable-sysfsgpio
sudo make
sudo make install
```

## Copy the x-heep folder to the Pynq-Z2 baord

Copy the x-heep/linux_femu/arm/ folder from your PC to the home directory of the Pynq-Z2 board with the following command:

```
sudo scp -r x-heep/linux_femu/arm/ xilinx@board_ip:~
```

## Enable UART1 on Linux on the Pynq-Z2 baord

Enter the arm/uart_enable/ folder and run the following commands:

```
sudo su
mkdir -p /configfs
./uart_enable.sh
exit
```

These commands enable the UART1 on the ARM system side of the Zynq 7020 on the Pynq-Z2 board. UART1 is used to receive the stdout of the code running on the HEEP architecture implemented on the PL side of the chip.

## Prepare the needed shells

You need to use 5 shells (the first, second and third shells will run on the Pynq-Z2 board, while the fourth and fifth shells will run on your PC):

1. First shell - run virtual flash app on the Pynq-Z2 board - browse to the arm/virtual_flash/ folder and execute the following commands to compile and run the required application:

```
sudo make clean
sudo make
sudo ./virtual_flash
```

This app allocates a buffer into the off-chip DDR memory of the Pynq-Z2 board and stores its physical base address to the hijacker PL peripheral.

2. Second shell - run the following commands on the Pynq-Z2 board to get the stdout from the app running on x-heep:

```
sudo apt-get install screen
sudo screen /dev/ttyPS1 115200
```

3. Third shell - run openocd on the Pynq-Z2 board - browse to the arm/openocd_cfg/ folder and run the following command:

```
sudo openocd -f ./gpio_bitbang.cfg
```

4. Fourth shell - compile HEEP app on your PC - browse to the x-heep/sw/ folder and run the following command:

```
make clean applications/example_virtual_flash/example_virtual_flash.hex TARGET=pynq-z2
```

With this command you compile a sample RISC-V based application that uses the allocated DDR-based virtual memory. You may want to change this command with the name of your own application.

5. Fifth shell - run GDB on your PC - browse to the x-heep/sw/applications/example_virtual_flash/ folder and run the following command to connect GDB:

```
sudo /path_to_your_riscv_toolchain/bin/riscv32-unknown-elf-gdb
```

Use the following GDB commands to connect to OpenOCD running on the Pynq-Z2 board and run your application:

```
(gdb) target remote board_ip:3333
(gdb) load example_virtual_flash.elf
(gdb) cont
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

__NOTE__: to run other applications, press the hard reset button on the Pynq-Z2 board (BTN3) and re-connect OpneOCD and GDB.
