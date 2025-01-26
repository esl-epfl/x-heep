# OBI SPI slave


```{contents} Table of Contents
:depth: 2
```

## Preliminary Definitions

### Master and Host

The terms _Master_ and _Host_ are used interchangeably.

### Slave and Target

The terms _Slave_ and _Target_ are used interchangeably.

### SPI Target

The term _SPI Target_ refers to the SPI target of the _OBI SPI slave IP_.

### TX and RX

- TX: Transmission-related data or operations.

- RX: Reception-related data or operations.

This is relative depending on the device used. E.g. when a _SPI Host_ sends data to a _SPI Target_ it is a TX transmission for the _SPI Host_ and a RX transmission for the _SPI Target_. 

### Word

A word, when used as a unit of digital information, always refers to 32 bits
throughout the entire documentation.

### Command

A command is a 8-bit long instruction sent by the _SPI Host_ to the _SPI Target_.

### Wrap length

The wrap length describes the amount of words that are to be written or read. 

### Dummy cycles

The dummy cycles describe the amount of clock cycles the _SPI Target_ waits until it sends its data to the _SPI Host_. 

### IP

IP is short for integrated peripheral. In this context IP means that the peripheral has been integrated into X-HEEP.


## Introduction

The _OBI SPI slave IP_ is connected to X-HEEP's OBI bus and enables the flow of data from the OBI bus to an external _SPI Host_. This documentation uses the code from the _example_spi_obi_slave_ which uses the already into X-HEEPs integrated _SPI Host_ to demonstrate how the communication with the _OBI SPI slave IP_ works. It is important to note that the _example_spi_obi_slave_ code uses the current library from the _SPI Host IP_ which means that it potentially could be outdated and improved.  

## Configurations

### Chip select

The _SPI Host_ has to activate the active low chip select signal for every kind of communication with the _SPI Target_.


### SPI target registers 

The _SPI Target_ of the _OBI SPI slave IP_ contains three different 8-bit registers which store the values for the dummy cycles and wrap length features. To configurate these registers one has to send a read or write command byte to the _SPI Target_ to specify the desired register. After the command byte has been sent the desired 8-bit value can follow. The _OBI SPI slave IP_ processes data with the most significant bit (MSB) first.


- `reg0`: This register stores the amount of dummy cycles. `0x11` is the command to write to it and `0x07` is the command to read from it. The default value is `32`.
- `reg1`: This register stores the lower byte of the wrap length. `0x20` is the command to write to it and `0x21` is the command to read from it. The default value is `0`.
- `reg2`: This register stores the upper byte of the wrap length. `0x30` is the command to write to it and `0x31` is the command to read from it. The default value is `0`.

```{note}
The values of the registers will return to their default values when reset signal from the OBI bus is active. 
```

### Reading and writing 

To read from the memory the _SPI Host_ has to send the read command which is `0x0B`. After the read command has been sent it has to be followed by the 32-bit starting address of the desired data. 

To write from the _SPI Host_ to the memory the _SPI Host_ has to send the write command which is `0x02`. After the write command has been sent it has to be followed by the 32-bit address of the desired data destination. 

## Writing Data

### Process

The process of writing data from a _SPI Host_ to the _SPI Target_ is as follows:

1. Activate the the active low chip select
1. Define the amount of words that are to be sent. This is done by sending the command byte to write into `reg1` followed by the lower byte of the wrap length. This is followed up with the command byte to write into `reg2` and the upper wrap length.   
1. Send the write command to the _SPI Target_ (which is `0x02`). 
1. Send the address where the data is to be stored. This has to be a 32-bit address.
1. Send the data. The data can only be sent in words. 
1. Deactivate the chip select. 

### Example with SPI Host IP

In the code examples the explanation of the initial configuration of the _SPI Host IP_ is omitted. It is, however, required when using the integrated _SPI Host IP_. The code example follows the proccess defined above and adapts it to the _SPI Host IP_. 

This function handles the entire writing process:

```c
static spi_flags_e spi_slave_write(uint32_t addr, uint32_t *data, uint16_t length) {
    /*  The  OBI SPI slave IP has no way of knowing or handling incorrect addresses. 
     *  Using an incorrect address leads to an abort of the simulation and it is   
     *  unkown what happens when this issue occurs after physical implementation. 
     *  This is why it is absolutely necessary to include checks such as the following two lines. 
    */
    if(DIV_ROUND_UP(length, WORD_SIZE_IN_BYTES) > MAX_DATA_SIZE) return SPI_SLAVE_FLAG_SIZE_OF_DATA_EXCEEDED;
    if (addr % 4 != 0 || (addr + length) > LAST_VALID_ADDRESS) return SPI_SLAVE_FLAG_ADDRESS_INVALID;


    spi_slave_write_wrap_length(length);

    spi_write_byte(spi_hst, WRITE_SPI_SLAVE_CMD);
    spi_wait_for_ready(spi_hst); 
    send_command_to_spi_host(1, true, SPI_DIR_TX_ONLY);

    ///write address
    spi_write_word(spi_hst, make_word_compatible_for_spi_host(addr));
    spi_wait_for_ready(spi_hst); 
    send_command_to_spi_host(4, true, SPI_DIR_TX_ONLY);

 
    /*
     * Place data in TX FIFO
     * In simulation it do not wait for the flash to be ready, so we must check
     * if the FIFO is full before writing.
    */
    uint16_t counter = 0;
    uint32_t *data_32bit = (uint32_t *)data;
    for (uint16_t i = 0; i < (length>>2 ); i++) {
        if(SPI_HOST_PARAM_TX_DEPTH == counter){
            send_command_to_spi_host(SPI_HOST_PARAM_TX_DEPTH*WORD_SIZE_IN_BYTES, true, SPI_DIR_TX_ONLY);
            counter = 0;
        }
        spi_wait_for_tx_not_full(spi_hst);
        spi_write_word(spi_hst, make_word_compatible_for_spi_host(data_32bit[i]));
        counter++;
    }
    if (length % 4 != 0) {
        uint32_t last_word = 0;
        memcpy(&last_word, &data[length - length % 4], length % 4);
        spi_wait_for_tx_not_full(spi_hst);
        spi_write_word(spi_hst, make_word_compatible_for_spi_host(last_word));
    }

    uint16_t last_words = length-((length/(SPI_HOST_PARAM_TX_DEPTH*4))*(SPI_HOST_PARAM_TX_DEPTH*4));

    if(last_words == 0){
        last_words = SPI_HOST_PARAM_TX_DEPTH*4;
    }

    send_command_to_spi_host(last_words, false, SPI_DIR_TX_ONLY);
    spi_wait_for_tx_empty(spi_hst);
    return SPI_FLAG_SUCCESS; // Success
}
```

#### Activate the the active low chip select
The _SPI Host IP_ automatically activates the chip select when a command is sent. It has to be specified if the chip select should remain active after sending a command to ensure deactivation. 

The helper function `send_command_to_spi_host` is used to create and send commands to the SPI Host:


```c
static void send_command_to_spi_host(uint32_t len, bool csaat, uint8_t direction){
    if(direction != SPI_DIR_DUMMY){
        len--; //The SPI HOST IP uses len-1 = amount of bytes to read and write. But also len = amount of dummy cycles
    }
    const uint32_t send_cmd_byte = spi_create_command((spi_command_t){
        .len        = len,                     
        .csaat      = csaat,                    // Command not finished e.g. CS remains low after transaction
        .speed      = SPI_SPEED_STANDARD,       // Single speed
        .direction  = direction
    });
    spi_set_command(spi_hst, send_cmd_byte);
    spi_wait_for_ready(spi_hst);
}
```


#### Define the amount of words that are to be sent

The data size (wrap length) must be set by writing to `reg1` (lower byte) and `reg2` (upper byte). The following function handles this:
```c
void spi_slave_write_wrap_length(uint16_t length){

    uint32_t wrap_length_cmds = (WRITE_SPI_SLAVE_REG_1 << 24)               
                                + (LOWER_BYTE_16_BITS(length>>2) << 16)     //Wraplength low 
                                + (WRITE_SPI_SLAVE_REG_2 << 8)              
                                + (UPPER_BYTE_16_BITS(length>>2));          //Wraplength high

    spi_write_word(spi_hst, make_word_compatible_for_spi_host(wrap_length_cmds));
    spi_wait_for_ready(spi_hst);

    send_command_to_spi_host(4, true, SPI_DIR_TX_ONLY);
}
```

The _SPI Host IP_ sends an individual byte in the most significant bit (MSB) first order. However, when the _SPI Host IP_ sends out more than one byte or a word, the byte order gets changed to send out the least significant byte first. To correct this the following helper function has been added:

```c
uint32_t make_word_compatible_for_spi_host(uint32_t word){
    return (LOWER_BYTE_16_BITS(LOWER_BYTES_32_BITS(word)) << 24) 
        | (UPPER_BYTE_16_BITS(LOWER_BYTES_32_BITS(word)) << 16) 
        | (LOWER_BYTE_16_BITS(UPPER_BYTES_32_BITS(word)) << 8) 
        | UPPER_BYTE_16_BITS(UPPER_BYTES_32_BITS(word));
}
```
#### Send the write command to the SPI Target

The write command (`0x02`) is sent to the SPI Target to initiate the data transfer.

```c
spi_flags_e spi_slave_write(uint32_t addr, uint32_t *data, uint16_t length) {

    ...

    spi_write_byte(spi_hst, WRITE_SPI_SLAVE_CMD);
    spi_wait_for_ready(spi_hst); 
    send_command_to_spi_host(1, true, SPI_DIR_TX_ONLY);

    ...
}
```

#### Send the address where the data is to be stored

The code includes checks to make sure that the chosen address is correct and that the amount of data doesn't overflow the memory. This is important since the _OBI SPI slave IP_ has no way of indicating invalid addresses or memory overflows of X-HEEPs RAM. Also it is unknown how X-HEEPs OBI bus handles the access of invalid addresses since the simulation aborts in that case. Thus this has to be prevented in the code of the SPI Host. 

```c
spi_flags_e spi_slave_write(uint32_t addr, uint32_t *data, uint16_t length) {
    if(DIV_ROUND_UP(length, WORD_SIZE_IN_BYTES) > MAX_DATA_SIZE) return SPI_SLAVE_FLAG_SIZE_OF_DATA_EXCEEDED;
    if (addr % 4 != 0 || (addr + length) > LAST_VALID_ADDRESS) return SPI_SLAVE_FLAG_ADDRESS_INVALID;

    ...

    spi_write_word(spi_hst, make_word_compatible_for_spi_host(addr));
    spi_wait_for_ready(spi_hst); 
    send_command_to_spi_host(4, true, SPI_DIR_TX_ONLY);


    ...
}
```

#### Send the data

The data is sent word by word. If the length exceeds the TX FIFO depth of the _SPI Host IP_, the process ensures the FIFO is flushed in chunks.


```c
static spi_flags_e spi_slave_write(uint32_t addr, uint32_t *data, uint16_t length) {
    
    ...

    /*
     * Place data in TX FIFO
     * In simulation it do not wait for the flash to be ready, so we must check
     * if the FIFO is full before writing.
    */
    uint16_t counter = 0;
    uint32_t *data_32bit = (uint32_t *)data;
    for (uint16_t i = 0; i < (length>>2 ); i++) {
        if(SPI_HOST_PARAM_TX_DEPTH == counter){
            send_command_to_spi_host(SPI_HOST_PARAM_TX_DEPTH*WORD_SIZE_IN_BYTES, true, SPI_DIR_TX_ONLY);
            counter = 0;
        }
        spi_wait_for_tx_not_full(spi_hst);
        spi_write_word(spi_hst, make_word_compatible_for_spi_host(data_32bit[i]));
        counter++;
    }
    if (length % 4 != 0) {
        uint32_t last_word = 0;
        memcpy(&last_word, &data[length - length % 4], length % 4);
        spi_wait_for_tx_not_full(spi_hst);
        spi_write_word(spi_hst, make_word_compatible_for_spi_host(last_word));
    }

    uint16_t last_words = length-((length/(SPI_HOST_PARAM_TX_DEPTH*4))*(SPI_HOST_PARAM_TX_DEPTH*4));

    if(last_words == 0){
        last_words = SPI_HOST_PARAM_TX_DEPTH*4;
    }

    send_command_to_spi_host(last_words, false, SPI_DIR_TX_ONLY);
    spi_wait_for_tx_empty(spi_hst);
    return SPI_FLAG_SUCCESS; 
}
```
#### Deactivate the chip select

As stated before every command includes the option of deactivating the chip select after transmission which has been done.   

```c
static spi_flags_e spi_slave_write(uint32_t addr, uint32_t *data, uint16_t length) {

    ...

    send_command_to_spi_host(last_words, false, SPI_DIR_TX_ONLY);
    return SPI_FLAG_SUCCESS; 
}
```


## Reading Data

### Process 

The process of reading data from the _SPI Target_ with a_SPI Host_ is as follows:

1. Activate the the active low chip select
1. (Optional) Set the amount of dummy cycles. This is done by writing to `reg` with the command `0x11` which is followed up by the desired dummy cycle value. This is a 8-bit value. By default the dummy cycles are 32 when not changed previously. 
1. Define the amount of words that are to be read. This is done by sending the command byte to write into `reg1` followed by the lower byte of the wrap length. This is followed up with the command byte to write into `reg2` and the upper wrap length.   
1. Send the read command to the _SPI Target_ (which is `0x0B`). 
1. Send the starting address where the data is stored. This has to be a 32-bit address.
1. Wait the amount of dummy cycles.
1. Read the data. The data can only be read in words. 
1. Deactivate the chip select. 

```{caution}
The amount of dummy cycles can't be too low since the communication between the _OBI SPI slave IP_ and the OBI Bus doesn't fill the TX FIFO buffer instantly. Thus setting the dummy cycles too low would lead the _SPI Host_ to read from the _SPI Target_ when the FIFO buffer of the _SPI Target_ isn't ready. This would result in the _SPI Host_ reading incorrect values instead of the real data and it would desynchronize the _SPI Host_ and _OBI SPI Slave IP_.
```
### Example with SPI Host IP

```c
spi_flags_e spi_slave_read(uint32_t addr, void* data, uint16_t length, uint8_t dummy_cycles){
    if(DIV_ROUND_UP(length, WORD_SIZE_IN_BYTES) > MAX_DATA_SIZE) return SPI_SLAVE_FLAG_SIZE_OF_DATA_EXCEEDED;
    if (addr % 4 != 0 || (addr + length) > LAST_VALID_ADDRESS) return SPI_SLAVE_FLAG_ADDRESS_INVALID;
    

    spi_slave_write_dummy_cycles(dummy_cycles);

    spi_slave_write_wrap_length(length);
 

    spi_write_byte(spi_hst, READ_SPI_SLAVE_CMD);
    spi_wait_for_ready(spi_hst);

    send_command_to_spi_host(1, true, SPI_DIR_TX_ONLY);

    spi_write_word(spi_hst, make_word_compatible_for_spi_host(addr));
    spi_wait_for_ready(spi_hst); 
    send_command_to_spi_host(4, true, SPI_DIR_TX_ONLY);

    spi_host_wait(dummy_cycles);

    send_command_to_spi_host(length, false, SPI_DIR_RX_ONLY);


    /*
     * Set RX watermark to length. The watermark is in words.
     * If the length is not a multiple of 4, the RX watermark is set to length/4+1
     * to take into account the extra bytes.
     * If the length is higher then the RX FIFO depth, the RX watermark is set to
     * RX FIFO depth. In this case the flag is not set to 0, so the loop will
     * continue until all the data is read.
    */
    bool flag = 1;
    uint16_t to_read = 0;
    uint16_t i_start = 0;
    uint16_t length_original = length;
    uint32_t *data_32bit = (uint32_t *)data;
    while (flag) {
        if (length >= SPI_HOST_PARAM_RX_DEPTH) {
            spi_set_rx_watermark(spi_hst, SPI_HOST_PARAM_RX_DEPTH>>2);
            length -= SPI_HOST_PARAM_RX_DEPTH;
            to_read += SPI_HOST_PARAM_RX_DEPTH;
        }
        else {
            spi_set_rx_watermark(spi_hst, (length%4==0 ? length>>2 : (length>>2)+1));
            to_read += length;
            flag = 0;
        }
        // Wait till SPI Host RX FIFO is full (or I read all the data)
        spi_wait_for_rx_watermark(spi_hst);
        // Read data from SPI Host RX FIFO
        for (uint16_t i = i_start; i < to_read>>2; i++) {
            spi_read_word(spi_hst, &data_32bit[i]); // Writes a full word
        }
        // Update the starting index
        i_start += SPI_HOST_PARAM_RX_DEPTH>>2;
    }
    // Take into account the extra bytes (if any)
    if (length_original % 4 != 0) {
        uint32_t last_word = 0;
        spi_read_word(spi_hst, &last_word);
        memcpy(&data_32bit[length_original>>2], &last_word, length%4);
    }

    return SPI_FLAG_SUCCESS; // Success
}
```
#### Activate the the active low chip select
Similar to the write operation, the SPI Host automatically activates the chip select when commands are sent.



#### (Optional) Set the amount of dummy cycles
Dummy cycles are set by writing to `reg0`. By default, 32 cycles are used if not explicitly set.


```c
void spi_slave_write_dummy_cycles(uint8_t cycles){

    // Load command to TX FIFO
    spi_write_byte(spi_hst, WRITE_SPI_SLAVE_REG_0);
    spi_wait_for_ready(spi_hst);

    //The spi_write_byte function fills always the same byte in the TX FIFO of the SPI HOST IP
    //Hence it is not possible to write the reg 1 command and cycles at the same time without writing a word.
    send_command_to_spi_host(1, true, SPI_DIR_TX_ONLY);

    // Load command to TX FIFO
    spi_write_byte(spi_hst, cycles);
    spi_wait_for_ready(spi_hst);

    send_command_to_spi_host(1, true, SPI_DIR_TX_ONLY);
}
```

#### Define the amount of words that are to be read
This is handled the same way as in the write operation.

#### Send the read command to the SPI Target

The read command (`0x0B`) is sent to the SPI Target:

```c
spi_flags_e spi_slave_read(uint32_t addr, void* data, uint16_t length, uint8_t dummy_cycles){
 
    ...
 
    spi_write_byte(spi_hst, READ_SPI_SLAVE_CMD);
    spi_wait_for_ready(spi_hst);

    send_command_to_spi_host(1, true, SPI_DIR_TX_ONLY);
}
```

#### Send the starting address where the data is stored
Same as in the writing example.

#### Wait the amount of dummy cycles

The SPI Host has to send the amount of dummy cycles as clock cycles to the _OBI SPI slave IP_. This functionality is already integrated in the _SPI Host IP_.

```c
void spi_host_wait(uint8_t cycles){
    if(cycles == 0){
        return;
    }
    send_command_to_spi_host(cycles, true, SPI_DIR_DUMMY);
}
```

#### Read the data

```c
spi_flags_e spi_slave_read(uint32_t addr, void* data, uint16_t length, uint8_t dummy_cycles){

    ...

    /*
     * Set RX watermark to length. The watermark is in words.
     * If the length is not a multiple of 4, the RX watermark is set to length/4+1
     * to take into account the extra bytes.
     * If the length is higher then the RX FIFO depth, the RX watermark is set to
     * RX FIFO depth. In this case the flag is not set to 0, so the loop will
     * continue until all the data is read.
    */
    bool flag = 1;
    uint16_t to_read = 0;
    uint16_t i_start = 0;
    uint16_t length_original = length;
    uint32_t *data_32bit = (uint32_t *)data;
    while (flag) {
        if (length >= SPI_HOST_PARAM_RX_DEPTH) {
            spi_set_rx_watermark(spi_hst, SPI_HOST_PARAM_RX_DEPTH>>2);
            length -= SPI_HOST_PARAM_RX_DEPTH;
            to_read += SPI_HOST_PARAM_RX_DEPTH;
        }
        else {
            spi_set_rx_watermark(spi_hst, (length%4==0 ? length>>2 : (length>>2)+1));
            to_read += length;
            flag = 0;
        }
        // Wait till SPI Host RX FIFO is full (or I read all the data)
        spi_wait_for_rx_watermark(spi_hst);
        // Read data from SPI Host RX FIFO
        for (uint16_t i = i_start; i < to_read>>2; i++) {
            spi_read_word(spi_hst, &data_32bit[i]); // Writes a full word
        }
        // Update the starting index
        i_start += SPI_HOST_PARAM_RX_DEPTH>>2;
    }
    // Take into account the extra bytes (if any)
    if (length_original % 4 != 0) {
        uint32_t last_word = 0;
        spi_read_word(spi_hst, &last_word);
        memcpy(&data_32bit[length_original>>2], &last_word, length%4);
    }

    return SPI_FLAG_SUCCESS; // Success
}
```

#### Deactivate the chip select
Same as in the writing example.


## Remarks for writing a SPI Host code

### Synchronization Challenges

The CPU has no way of knowing if the _OBI SPI Slave IP_ is actively writing. Consequently, when the CPU attempts to read data from RAM that the _OBI SPI Slave IP_ has not yet finished writing to, the data the CPU reads can be partially incorrect. In the _example_spi_obi_slave_ code, a workaround was introduced: the CPU waits until the TX FIFO buffer of the _SPI Host IP_ has been emptied:

```c
static spi_flags_e spi_slave_write(uint32_t addr, uint32_t *data, uint16_t length) {
    
    ...

    spi_wait_for_tx_empty(spi_hst);
    return SPI_FLAG_SUCCESS; 
}
```

```{caution}
This workaround alone is insufficient to guarantee that the synchronization issue will not occur when the CPU reads from memory. It has only been added to the _example_spi_obi_slave_ code for functional testing purposes of the IP.
```