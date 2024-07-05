# Run on FPGA

This project offers two different X-HEEP implementetions on Xilinx FPGAs, called Standalone and FEMU.

## Standalone

### Set-up
In this version, the X-HEEP architecture is implemented on the programmable logic (PL) side of the FPGA, and its input/output are connected to the available headers on the FPGA board.

Two FPGA boards are supported: the Xilinx Pynq-z2 and Nexys-A7-100t.

1. Make sure you have the FPGA board files installed in your Vivado.
> For example, for the Pynq-Z2 board, use the documentation provided at the following [link](https://pynq.readthedocs.io/en/v2.5/overlay_design_methodology/board_settings.html) to download and install them.

2. Make sure you set up the Vivado environments by running
   ```
   source <vivado-installation-path>/settings64.sh
   ```
   > We recommend adding this command to your `.bashrc`

3. Install the Xilinx cable drivers.
* Follow the [instructions for Linux](https://docs.amd.com/api/khub/documents/6EIhov6ruoilhq8zq7bXBA/content?Ft-Calling-App=ft%2Fturnkey-portal&Ft-Calling-App-Version=4.3.26#G4.262534)
* Restart your PC

### Running

To build and program the bitstream for your FPGA with vivado, type:

```
make vivado-fpga FPGA_BOARD=pynq-z2
```

or

```
make vivado-fpga FPGA_BOARD=nexys-a7-100t
```

or add the flag `use_bscane_xilinx` to use the native Xilinx scanchain:

```
make vivado-fpga FPGA_BOARD=pynq-z2 FUSESOC_FLAGS=--flag=use_bscane_xilinx
```

To program the bitstream, open Vivado,

```
open --> Hardware Manager --> Open Target --> Autoconnect --> Program Device
```

and choose the file `openhwgroup.org_systems_core-v-mini-mcu_0.bit`.

Or simply type:

```
bash vivado-fpga-pgm FPGA_BOARD=pynq-z2
```

or

```
make vivado-fpga-pgm FPGA_BOARD=nexys-a7-100t
```

To run SW, follow the [Debug](./Debug.md) guide to load the binaries with the HS2 cable over JTAG,
or follow the [ExecuteFromFlash](./ExecuteFromFlash.md) guide if you have a FLASH attached to the FPGA.

Do not forget that the `pynq-z2` board requires you to have the ethernet cable attached to the board while running.

For example, if you want to run your application using flash_exec, do as follow:
compile your application, e.g. `make app PROJECT=example_matfadd TARGET=pynq-z2 ARCH=rv32imfc LINKER=flash_exec`
and then follow the [ExecuteFromFlash](./ExecuteFromFlash.md) to program the flash and set the boot buttons on the FPGA correctly.
To look at the output of your printf, run in another terminal:
`picocom -b 9600 -r -l --imap lfcrlf /dev/ttyUSB2`
Please be sure to use the right `ttyUSB` number (you can discover it with `dmesg --time-format iso | grep FTDI` for example).

## FPGA EMUlation Platform (FEMU)

In this version, the X-HEEP architecture is implemented on the programmable logic (PL) side of the Xilinx Zynq-7020 chip on the Pynq-Z2 board and Linux is run on the ARM-based processing system (PS) side of the same chip.

NOTE: This platform is not part of this repository, but you can access it with the following link: [FEMU](https://github.com/esl-epfl/x-heep-femu-sdk).
