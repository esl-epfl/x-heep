## Prerequisite

1. Install the required apt tools:

```
$ sudo apt install pkg-config libusb-1.0-0-dev
```

2. Install openOCD

Download  `openOCD` from https://sourceforge.net/projects/openocd/files/openocd/
Version 0.11.0-rc2.
After extracting the files,

```
$ cd openocd-0.11.0-rc2
$ ./configure --enable-ftdi --enable-remote-bitbang --prefix=/home/yourusername/tools/openocd && make
$ make install
```

3. Compile the remote_bitbang Server

(TODO: this should be done with FuseSoc)

```
$ cd tb/remote_bitbang
$ make all
```

You need at least gcc>10, so in case you do not have it:

```
$ sudo apt install gcc-10 g++-10
$ sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 --slave /usr/bin/g++ g++ /usr/bin/g++-10 --slave /usr/bin/gcov gcov /usr/bin/gcov-10
```
## Simulating

You need 3 shells to do this job.

Now we are going to Simulate debugging with core-v-mini-mcu.
In this setup, OpenOCD communicates with the remote bitbang server by means of DPIs.
The remote bitbang server is simplemented in the folder ./tb/remote_bitbang that has been previously compiled.

### Questasim

Compile the model as describe in the main Guide, then:

```
$ cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim
$ vsim -64 -gui tb_top_vopt +firmware=../../../sw/applications/hello_world/hello_world.hex -sv_lib ../../../tb/remote_bitbang/librbs
$ run -all
```

Now the remote bitbang server should print in your shell:

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
/home/username/tools/riscv/bin/riscv32-unknown-elf-gdb ./sw/applications/hello_world/hello_world.elf
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

Now you can play with [gdb](

e.g:

```
(gdb) info registers
```
