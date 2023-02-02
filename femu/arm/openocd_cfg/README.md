### Hardware Configurations

On the hardware side, we must configure the ZYNQ processing system to use the EMIO and export them externally so that they can be attached to x-heep. They are then routed outwards from the wrapper to x-heep JTAG signals.

### Software Configurations

## Target adapter configuration __gpio_bitbang.cfg__:

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

bindto 0.0.0.0

set _CHIPNAME riscv
jtag newtap $_CHIPNAME cpu -irlen 5 -expected-id 0x10001c05

set _TARGETNAME $_CHIPNAME.cpu
target create $_TARGETNAME riscv -chain-position $_TARGETNAME -coreid 0x000
echo "Target created"

riscv set_reset_timeout_sec 2000
riscv set_command_timeout_sec 2000
# riscv set_prefer_sba off

echo "Setting preferences"

# Each of the JTAG lines need a gpio number set: tck tms tdi tdo
# Header pin numbers: 23 22 19 21
sysfsgpio jtag_nums 964 961 963 962

# Each of the SWD lines need a gpio number set: swclk swdio
# Header pin numbers: 23 22
# sysfsgpio swd_nums 11 25

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

This instructs openocd to use the riscv HART and jtag module but instead of transmitting the commands over the usual FTDI usb adapter, it bitbangs the GPIOs by using the SYSFS framework.

In order to understand the pin numbering used in the script we have to know how the JTAG has been ported to the PYNQ without using external adapters. The ZYNQ 7000 offers several banks of GPIO lines used for different purposes. The first 54 are MIO (multiplexed I/O) and are configured for different purposes but are PS-side only. After those, we have 64 EMIO (extended MIO) which are lines that can be routed to the PL). The idea is therefore to expose 5 EMIO GPIOS (5 jtag signals) and route them to the PL and to x-heep directly. Those can be controlled as normal GPIO lines and allow interfacing directly from ARM.

Therefore, the pin numbers used in the configuration file are derived as follows:

1. Find the GPIO starting numbers that linux uses by running ``` sudo cat /sys/kernel/debug/gpio ``` -> in this case it was __906__

2. Add 54 (the first 54 GPIOS are the MIOs).

3. Add the gpio_n assigned to the specific signal (depends on the HDL connections).
