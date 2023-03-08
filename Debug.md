## Prerequisite

1. Install the required linux tools:

```
sudo apt install pkg-config libftdi1-2 libusb-1.0-4
```

You need at least gcc>10, so in case you do not have it:

```
sudo apt install gcc-10 g++-10
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 --slave /usr/bin/g++ g++ /usr/bin/g++-10 --slave /usr/bin/gcov gcov /usr/bin/gcov-10
```

2. Install openOCD

Download `openOCD` from https://sourceforge.net/projects/openocd/files/openocd/
Version 0.11.0-rc2.
After extracting the files,

```
cd openocd-0.11.0-rc2
./configure --enable-ftdi --enable-remote-bitbang --prefix=/home/$USER/tools/openocd && make
make install
```

Add to `PATH` `openOCD`:

```
export PATH=/home/$USER/tools/openocd/bin:$PATH
```
## Simulating

You need 3 shells to do this job.

Now we are going to Simulate debugging with core-v-mini-mcu.
In this setup, OpenOCD communicates with the remote bitbang server by means of DPIs.
The remote bitbang server is simplemented in the folder ./hw/vendor/pulp_platform_pulpissimo/rtl/tb/remote_bitbang and it will be compiled using fusesoc.

### Verilator

To simulate your application with Questasim using the remote_bitbang server, you need to compile you system adding the flag `use_jtag_dpi`:

```
make verilator-sim FUSESOC_FLAGS="--flag use_jtag_dpi"
```

then, go to your target system built folder

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
```

and type to run your compiled software:

```
./Vtestharness +firmware=../../../sw/build/main.hex +openOCD=true
```

### Questasim

To simulate your application with Questasim using the remote_bitbang server, you need to compile you system adding the flag `use_jtag_dpi`:

```
make questasim-sim FUSESOC_FLAGS="--flag=use_jtag_dpi"
```

then, go to your target system built folder

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim/
```

and type to run your compiled software:

```
make run PLUSARGS="c firmware=../../../sw/build/main.hex"
```
### Remote bitbang server started

Once the Verilator or Questasim simulation started, the remote bitbang server should print in your shell:

```
JTAG remote bitbang server is ready
Listening on port 4567
Attempting to accept client socket
Accepted successfully.
```

In another shell, connect OpenOCD:

```
export JTAG_VPI_PORT=4567
openocd -f ./tb/core-v-mini-mcu.cfg
```

OpenOCD will connect to the DPI remote bitbang server and it will start initialize the MCU.
Check the waveform of the JTAG on Modelsim if you like.

In a 3rd shell, conenct gdb as:

```
$RISCV/bin/riscv32-unknown-elf-gdb ./sw/build/main.elf
```

Once `gdb` starts, do the following 3 commands:
```
(gdb) set remotetimeout 2000
(gdb) target remote localhost:3333
(gdb) load
```

Keep in mind that this takes a lot of time due to the simulation time.

The output of `gdb` should be something like:

```
Loading section .vectors, size 0x100 lma 0x0
Loading section .init, size 0x48 lma 0x180
Loading section .text, size 0xfd4 lma 0x1c8
Loading section .rodata, size 0x128 lma 0x8000
Loading section .data, size 0x60 lma 0x8128
Loading section .sdata, size 0x10 lma 0x8188
Start address 0x00000180, load size 4788
Transfer rate: 67 bytes/sec, 798 bytes/write.
```

`gdb` automatically set the `program counter` to start from `_start`, check with:

```
(gdb) i r pc
```
It should give you `0x180`.

Now you can play with `gdb`

e.g: Ask for the content of register `a0`

```
(gdb) i r a0
```
or just run the entire execution with the continue command and then check the `uart0.log` to see the printed hello world string:

```
(gdb) continue
```

You can also run all the gdb steps by running:
```
make gdb_connect MAINFILE=<main_file_name_of_the_project_that WAS_built WITHOUT EXTENSION>
```

## Debugging on FPGA

We can use either the `Digilet HS2` cable with the `FT232HQ` [chip](https://www.ftdichip.com/Support/Documents/TechnicalNotes/TN_100_USB_VID-PID_Guidelines.pdf) which has Vendor ID `0x0403` and Product ID `0x6014`, or the EPFL Programmer (described in
[ProgramFlash](./ProgramFlash.md)) which has the `FT4232H` which has Vendor ID `0x0403` and Product ID `0x6011`.

Connect the HS2 cable to the FPGA or the EPFL PRogrammer.

For the HS2 Cable, follow the next Section.
If you want to install the `FT4232H` chip, follow the guide at [ProgramFlash](./ProgramFlash.md).

### HS2 Cable Install


If you execute `lsusb`, you should see something like:

```
Bus 001 Device 004: ID 0403:6014 Future Technology Devices International, Ltd FT232H Single HS USB-UART/FIFO IC
```

and by executing `dmesg --time-format iso | grep FTDI` you should see something like:

```
2022-06-09T10:53:07,139277+02:00 usbserial: USB Serial support registered for FTDI USB Serial Device
2022-06-09T10:53:07,139325+02:00 ftdi_sio 1-1.4:1.0: FTDI USB Serial Device converter detected
2022-06-09T10:53:07,139919+02:00 usb 1-1.4: FTDI USB Serial Device converter now attached to ttyUSB0
```
The date and time will of course be different, and the tty device number may also be different (ttyUBS1, ttyUSB2, etc).


### Run openOCD

Now run `openOCD` with the its the configuration file specific for the HS2 cable:

```
openocd -f ./tb/core-v-mini-mcu-nexsys-hs2.cfg
```

or with the EPFL Programmer using this command:

```
openocd -f ./tb/core-v-mini-mcu-pynq-z2-esl-programmer.cfg
```

or with the EPFL Programmer also using this other command (**strongly recommended**):

```
make openOCD_epflp
```


If you get this error:

```
libusb_open() failed with LIBUSB_ERROR_ACCESS
```

For the HS2 cable, create a file called `60-hs2.rules` in the folder `/etc/udev/rules.d/` (you need sudo permits).

Write the following text in the created file to describe the attributes of the FT232HQ chip:

```
# HS2
ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6014", MODE="664", GROUP="plugdev"
```

Otherwise for the EPFL Programmer, follow [ProgramFlash](./ProgramFlash.md).

You may also need to run `sudo usermod -a -G plugdev yourusername` and restart the utility with `sudo udevadm control --reload`.
