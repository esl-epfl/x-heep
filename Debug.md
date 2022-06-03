## Prerequisite

1. Install the required linux tools:

```
$ sudo apt install pkg-config libusb-1.0-0-dev
```

You need at least gcc>10, so in case you do not have it:

```
$ sudo apt install gcc-10 g++-10
$ sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 --slave /usr/bin/g++ g++ /usr/bin/g++-10 --slave /usr/bin/gcov gcov /usr/bin/gcov-10
```

2. Install openOCD

Download `openOCD` from https://sourceforge.net/projects/openocd/files/openocd/
Version 0.11.0-rc2.
After extracting the files,

```
$ cd openocd-0.11.0-rc2
$ ./configure --enable-ftdi --enable-remote-bitbang --prefix=/home/yourusername/tools/openocd && make
$ make install
```

Add to `PATH` `openOCD`:

```
$ export PATH=/home/yourusername/tools/openocd/bin:$PATH
```
## TODOs

As today the testbench does not have a way to select whether to boot from JTAG or not, so the CPU starts executing the program even before openOCD connects,
and so the simulation ends. Use a software which containts a `while(1)` to be sure the simulation is still in place once openOCD and GDB connects.
This will be replaced.

## Simulating

You need 3 shells to do this job.

Now we are going to Simulate debugging with core-v-mini-mcu.
In this setup, OpenOCD communicates with the remote bitbang server by means of DPIs.
The remote bitbang server is simplemented in the folder ./hw/vendor/pulp_platform_pulpissimo/rtl/tb/remote_bitbang and it will be compiled using fusesoc.

### Verilator

To simulate your application with Questasim using the remote_bitbang server, you need to compile you system adding the flag `use_jtag_dpi`:

```
$ fusesoc --cores-root . run --no-export --target=sim --tool=verilator --setup --build --flag use_jtag_dpi openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log
```

then, go to your target system built folder

```
$ cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
```

and type to run your compiled software:

```
$ ./Vtestharness +firmware=../../../sw/applications/hello_world/hello_world.hex"
```

### Questasim

To simulate your application with Questasim using the remote_bitbang server, you need to compile you system adding the flag `use_jtag_dpi`:

```
$ fusesoc --cores-root . run --no-export --target=sim --tool=modelsim --setup --build --flag use_jtag_dpi openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log
```

then, go to your target system built folder

```
$ cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim/
```

and type to run your compiled software:

```
$ make run PLUSARGS="c firmware=../../../sw/applications/hello_world/hello_world.hex"
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
/home/yourusername/tools/riscv/bin/riscv32-unknown-elf-gdb ./sw/applications/hello_world/hello_world.elf
```

Once GDB starts, do the following 3 commands:
```
(gdb) set remotetimeout 2000
(gdb) target remote localhost:3333
(gdb) load
```

Keep in mind that this takes a lot of time due to the simulation time.

The output of GDB should be something like:

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

Now you can play with `gdb`

e.g: Ask for the content of register `a0`

```
(gdb) i r a0
```
