# Instructions
This assumes that the _arm_ folder has been copied to the PYNQ-Z2 board in the home folder. If this is not desirable, the script content should be updated to reflect the changes.

## Enabling UART1
To enable UART1 enter the script/ folder and run the script ./uart_enable.sh

## Setting Up the Environment
Launching the script ./tmux_setup.sh sets up three panes with the UART coming from x-heep, the openocd server, and a terminal to execute and debug the code. 

### Running OpenOCD by itself
If you want to run the openocd server by hand browse to the openocd_cfg/ folder and execute: 

```
sudo openocd -f ./gpio_bitbang.cfg
```

## Setting Up the Fake Flash
In order to setup the Fake Flash enter the fake_flash/ folder and execute the binary after having compiled it: 

```
sudo make clean
sudo make

sudo ./fake_flash
```

When the buffer is allocated leave it open in the background by pressing _ctrl+z_. 

To close the buffer and dump the content execute in the terminal _fg_ and press enter. 
