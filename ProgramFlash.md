Install the required linux tools:

```
$ sudo apt install pkg-config libftdi1-2 libusb-1.0-4
```

Compile the iceprog program by doing

```
cd iceprog
make
```

Plug a micro-USB into the EPFL programmer, and connect the EPFL programmer to the Pynq-z2 FPGA PMODs.

We use the the `FT4232H` which has Vendor ID `0x0403` and Product ID `0x6011`.

On your Linux shell, execute `lsusb`, you should see something like:

```
Bus 001 Device 006: ID 0403:6011 Future Technology Devices International, Ltd FT4232H Quad HS USB-UART/FIFO IC
```

and by executing `dmesg --time-format iso | grep FTDI` you should see something like:

```
2022-07-14T11:32:47,136453+02:00 usb 1-1.6: FTDI USB Serial Device converter now attached to ttyUSB4
2022-07-14T11:32:47,136543+02:00 ftdi_sio 1-1.6:1.3: FTDI USB Serial Device converter detected
2022-07-14T11:32:47,136878+02:00 usb 1-1.6: FTDI USB Serial Device converter now attached to ttyUSB5
```

Now do this, use B for the SPI, -t to read the FLASH ID

```
./iceprog -d i:0x0403:0x6011 -I B -t
```

The output should be:

```
init..
cdone: high
reset..
cdone: high
flash ID: 0xEF 0x70 0x18 0x00
cdone: high
Bye.
```

If you get:

```
init..
Can't find iCE FTDI USB device (device string i:0x0403:0x6011).
ABORT.
```

create a file called `61-ftdi4.rules` (or whatever) in the folder `/etc/udev/rules.d/` (you need sudo permits).

Write the following text in the created file to describe the attributes of the FT4232H chip:

```
# ftdi4
ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6011", MODE="666", GROUP="plugdev"
```

You may also need to run `sudo usermod -a -G plugdev yourusername` and restart the utility with `sudo udevadm control --reload`.

You may want to unplug and plug back the USB cable and repeat the iceprog command.


Build the FPGA bistream as

```
fusesoc --cores-root . run --no-export --target=pynq-z2 --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildvivado.log
```

then go to

```
build/openhwgroup.org_systems_core-v-mini-mcu_0/pynq-z2-vivado
```

Open Vivado, open the Hardware Manager --> Open Target --> Autoconnect --> Program Device

and choose the file `openhwgroup.org_systems_core-v-mini-mcu_0.bit`


Program the FLASH as:

Reset the bitstream of the FPGA, then

```
./iceprog -d i:0x0403:0x6011 -I B ../../core-v-mini-mcu/sw/applications/hello_world/hello_world.hex
```

you can read the content of the FLASH as:

```
./iceprog -d i:0x0403:0x6011 -I B -r flash_content.txt
```

remember to have the jumper in, and then out after programming the FLASH to disconnect the FTDI chip.


then re-load the bitstream of the FPGA.

