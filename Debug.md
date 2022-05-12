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

4. Execute the TestBench with the Remote BitBang

After you compiled your system with Verilator,

```
$ cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
$ ln -s ../../../tb/remote_bitbang/librbs.so .
$ ./Vtestharness +openocd
```
