#! usr/bin/bash
# This script is to be run in the x-heep base directory (aka BASE)
# It takes only one argument: the application name.
# It takes for granted that myApplication.c is located in
# BASE/sw/applications/myApplication/
# Run this script from the BASE with
# $ sudo bash pynq-run <name-of-your-app>
# The pynq-z2 board must be
#   - Powered
#   - Loaded with the appropriate bitstream (use vivado)
#   - Connected to the EPFL programmer
#   - With sw1=0(outside) and sw0=1(inside)
make app-clean
# Compile and build the binaries to be loaded in the flash.
# To use different LINKER options, just modify this script.
# (remember to also change the make flash-prog command below)
make app PROJECT=$1 LINKER=flash_load TARGET=pynq-z2 MAINFILE=$1&& 
# To use the iceprog
( cd sw/vendor/yosyshq_icestorm/iceprog && make clean && make all ) &&
# Make and load the flash programmable binary into the iceprog
echo Starting make for flash-prog 
sudo make flash-prog MAINFILE=$1 &&
# Open picocom to communicate with the board through UART
echo Starting PICOCOM
sudo picocom -b 115200 -r -l --imap lfcrlf /dev/ttyUSB2
