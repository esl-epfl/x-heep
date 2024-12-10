# OBI SPI slave


```{contents} Table of Contents
:depth: 2
```

## Preliminary Definitions

### Master and Host

The terms _Master_ and _Host_ are used interchangeably.

### Slave and Target

The terms _Slave_ and _Target_ are used interchangeably.

### TX and RX

- TX: Transmission-related data or operations.

- RX: Reception-related data or operations.

This is relative depending on the device used. E.g. when a _SPI Host_ sends data to a _SPI Target_ it is a TX transmission for the _SPI Host_ and a RX transmission for the _SPI Target_. 

### Word

A word, when used as a unit of digital information, always refers to 32 bits
throughout the entire documentation.

### Command

A command is a 8-bit long instruction sent by the _SPI Host_ to the _SPI Target_ of the _OBI SPI slave IP_.

### Wrap length

The wrap length describes the amount of words that are to be written or read. 

### Dummy cycles

The dummy cycles describe the amount of clock cycles the _SPI Target_ waits until it sends its data to the _SPI Host_. 


## Introduction

The _OBI SPI slave IP_ is connected to X-HEEP's OBI bus and enables the flow of data from the OBI bus to an external _SPI Host_. This documentation uses the code from the _example_spi_obi_slave_ which uses the already into X-HEEPs integrated _SPI Host_ to demonstrate how the communication with the _OBI SPI slave IP_ works. It is important to note that the _example_spi_obi_slave_ code uses the current _SPI Host_ library which means that it potentially could be outdated and improved.  

## Configurations

### Chip select

The _SPI Host_ has to activate the active low chip select signal for every kind of communication with the _SPI Target_.


### SPI target registers 

The _SPI Target_ of the _OBI SPI slave IP_ contains four different 8-bit registers which store the values for the dummy cycles and wrap length features. To configurate these registers one has to send a read or write command byte to the _SPI Target_ to specify the desired register. After the command byte has been sent the desired 8-bit value can follow. 

- `reg0`: This register has no functionality. `0x01`is the command to write to it and `0x05`is the command to read from it. It has the default value `0`.
- `reg1`: This register stores the amount of dummy cycles. `0x11` is the command to write to it and `0x07` is the command to read from it. The default value is `32`.
- `reg2`: This register stores the lower byte of the wrap length. `0x20` is the command to write to it and `0x21` is the command to read from it. The default value is `0`.
- `reg3`: This register stores the upper byte of the wrap length. `0x30` is the command to write to it and `0x31` is the command to read from it. The default value is `0`.

```{note}
The values of the registers will return to their default values when reset signal from the OBI bus is active. 
```

### Reading and writing 

To read from the _SPI Target_ the _SPI Host_ has to send the read command which is `0x0B`. After the read command has been sent it has to be followed by the 32-bit starting address of the desired data. 

To write from the _SPI Host_ to the _SPI Target_ the _SPI Host_ has to send the write command which is `0x02`. After the write command has been sent it has to be followed by the 32-bit address of the desired data destination. 

## Writing Data

### Process

The process of writing data from a _SPI Host_ to the _SPI Target_ is as follows:

1. Activate the the active low chip select
1. Define the amount of words that are to be sent. This is done by sending the command byte to write into `reg2` followed by the lower byte of the wrap length. This is followed up with the command byte to write into `reg3` and the upper wrap length.   
1. Send the write command to the _SPI Target_ (which is `0x02`). 
1. Send the address where the data is to be stored. This has to be a 32-bit address.
1. Send the data. The data can only be sent in words. 
1. Deactivate the chip select. 

### Example with SPI Host IP

code has to be cleaned up before this can be written

## Reading Data

### Process 

The process of reading data from the _SPI Target_ with a_SPI Host_ is as follows:

1. Activate the the active low chip select
1. (Optional) Set the amount of dummy cycles. By default 32 when not changed previously.
1. Define the amount of words that are to be read. This is done by sending the command byte to write into `reg2` followed by the lower byte of the wrap length. This is followed up with the command byte to write into `reg3` and the upper wrap length.   
1. Send the read command to the _SPI Target_ (which is `0x0B`). 
1. Send the starting address where the data is stored. This has to be a 32-bit address.
1. Read the data. The data can only be read in words. 
1. Deactivate the chip select. 

```{caution}
The amount of dummy cycles can't be too low since the communication between the _OBI SPI slave IP_ and the OBI Bus doesn't fill the TX FIFO buffer instantly. Thus setting the dummy cycles too low would lead the _SPI Host_ to read from the _SPI Target_ when the FIFO buffer of the _SPI Target_ isn't ready. This would result in the _SPI Host_ reading zeroes instead of the real data and it would desynchronize the _SPI Host_ and _SPI Target_.
```
### Example with SPI Host IP

code has to be cleaned up before this can be written