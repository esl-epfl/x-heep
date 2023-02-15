### Hardware Configurations

On the hardware side, we must configure the ZYNQ processing system to enable UART1 and route tx and rx to the EMIO pins so that they can be attached to x-heep. They are then routed outwards from the wrapper to x-heep UART signals.

__NOTE__: remembed to connect tx to rx and viceversa.

### Software Configurations

## Enable UART1

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
dtc -O dtb -o uart_enable.dtbo -b 0 -@ uart_enable.dtsi
```
2. We mount the configfs, which is a RAM-based configuration file-system exposed to add overlays.

```
sudo mount configfs configfs /configfs
sudo mkdir configfs/device-tree/overlays/uart_enable
```

3. Concatenate the dtbo binary file to insert into the kernel device tree.

```
sudo su
cat uart_enable.dtbo >/configfs/device-tree/overlays/uart_enable/dtbo
exit
```

4. Make sure with dmesg that no errors were thrown and that /dev/ttySP1 has appeared -> that is x-heep's serial.
