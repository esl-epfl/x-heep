## Prerequisites

1. Build the riscv toolchain for armhf on the PYNQ (or at least gdb). 
2. Install openocd for armhf on the PYNQ
3. Make sure PATH contains all the necessary bin folders

## Start the openocd server
```
sudo openocd -f ./gpio_bitbang.cfg
```

1. Start the openocd server and use the following target adapter configuration: 

```
# SPDX-License-Identifier: GPL-2.0-or-later

#
# Config for using RaspberryPi's expansion header
#
# This is best used with a fast enough buffer but also
# is suitable for direct connection if the target voltage
# matches RPi's 3.3V
#
# Do not forget the GND connection, pin 6 of the expansion header.
#

adapter driver sysfsgpio
transport select jtag

set _CHIPNAME riscv
jtag newtap $_CHIPNAME cpu -irlen 5 -expected-id 0x10001c05

set _TARGETNAME $_CHIPNAME.cpu
target create $_TARGETNAME riscv -chain-position $_TARGETNAME -coreid 0x000
echo "Target created"

riscv set_reset_timeout_sec 2000
riscv set_command_timeout_sec 2000
riscv set_prefer_sba off

echo "Setting preferences"


# Each of the JTAG lines need a gpio number set: tck tms tdi tdo
# Header pin numbers: 23 22 19 21
sysfsgpio jtag_nums 964 961 963 962

# Each of the SWD lines need a gpio number set: swclk swdio
# Header pin numbers: 23 22
#sysfsgpio swd_nums 11 25

# If you define trst or srst, use appropriate reset_config
# Header pin numbers: TRST - 26, SRST - 18

sysfsgpio trst_num 960
reset_config trst_only

# sysfsgpio srst_num 24
# reset_config srst_only srst_push_pull

# or if you have both connected,
# reset_config trst_and_srst srst_push_pull

scan_chain

init

echo "Init routine started"

halt
echo "Ready for connections"
```
_NOTE_: the configuration was created by merging the one already used with the EPFL programmer and the raspberry-pi sysfs example. 

This instructs openocd to use the riscv HART and jtag module but instead of transmitting the commands over the usual FTDI usb adapter, it bitbangs the GPIOs by using the SYSFS framework. 
_NOTE: in theory we should use the newer libgpiod interface to GPIOs, but this was quicker to set up and for now works_

In order to understand the pin numbering used in the script we have to know how the JTAG has been ported to the PYNQ without using external adapters. 
The ZYNQ 7000 offers several banks of GPIO lines used for different purposes. The first 54 are MIO (multiplexed I/O) and are configured for different purposes but are PS-side only. After those, we have 64 EMIO (extended MIO) which are lines that can be routed to the PL). The idea is therefore to expose 5 EMIO GPIOS (5 jtag signals) and route them to the PL and to x-heep directly. Those can be controlled as normal GPIO lines and allow interfacing directly from ARM. 

Therefore, the pin numbers used in the configuration file are derived as follows:
1. Find the GPIO starting numbers that linux uses by running ``` sudo cat /sys/kernel/debug/gpio ``` -> in this case it was __906__
2. Add 54 (the first 54 GPIOS are the MIOs)
3. Add the gpio_n assigned to the specific signal (depends on the HDL connections)


## Hardware Modifications
On the hardware side, we must configure the ZYNQ processing system to use the EMIO and export them externally so that they can be attached to x-heep. They are then routed outwards from the wrapper to x-heep JTAG signals. 
A similar process is done for the UART1, which is first enabled in the ZYNQ processing system and routed to EMIO pins in its configuration. Then the external signals are routed to x-heep UART lines. 
__NOTE__: remembed to connect tx to rx and viceversa

## Enable UART1
__NOTE__: I believe it's best if we program the bitstream before doing the following steps. It may work anyway but I have not tested.

In order to ask Linux to use that as a normal serial port, we need to modify the DEVICE TREE to expose this new hardware piece. In theory, the UART1 peripheral is already defined in the tree but is disabled. Instead of rebuilding the device tree, we can add an overlay at runtime to add and change configuration properties of the tree. In particular, we need to extend the device tree with the following: 

```
/dts-v1/;
/plugin/;

/{
	fragment@0{
		target-path = "/aliases";
		__overlay__ {
			serial1 = "/axi/serial@e0001000";	
		};
	};

	fragment@1{
		target = <&uart1>;
		__overlay__ {
			status = "okay";
		};
	};
};

```
This first makes an alias to make sure that the uart1 (/axi/serial@e0001000) is called serial1, and then enables it by setting the status as "okay". Aliasing is important to avoid a problem that could arise if the kernel decides to swap the ttyPS1 and ttyPS0 (uart0 and the one used to connect to ARM through serial), because we'd have no access to the board over serial. 

1. DTSI file must be compiled into binary format as follows:

```
dtc -O dtb -o uart_test.dtbo -b 0 -@ uart_test.dtsi
```
2. We mount the configfs, which is a RAM-based configuration file-system exposed to add overlays
```
sudo mount configfs configfs /configfs
sudo mkdir configfs/device-tree/overlays/uart_test
```
3. Concatenate the dtbo binary file to insert into the kernel device tree
```
sudo su
cat uart_test.dtbo >/configfs/device-tree/overlays/uart_test/dtbo
```
4. Make sure with dmesg that no errors were thrown and that /dev/ttySP1 has appeared -> that is x-heep's serial 

## Adding Fake Flash
Adding the Fake Flash is easy once the axi_spi_slave IP is tested and testbenched to reverse-engineer the protocol spoken. It offers a standard and qspi interface and an AXI4 FULL interface. The SPI interface is directly attached to x-heep's flash spi, while the AXI master interface is attached to the AXI interconnect crossbar connected to the PS's AXI HP port to give access to the entire address space. 
When connecting, we need to remember to export the reset and aclk needed by the axi part of axi_spi_slave.
