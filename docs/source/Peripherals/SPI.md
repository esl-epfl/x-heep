# SPI Host

```{contents} Table of Contents
:depth: 4
```

## Getting Started

### SDK

#### Initialization

The most important is `spi_t`, which holds all the necessary information to identify 
an external SPI device. It specifies which _SPI Host IP_ hardware to use and contains 
all the data related to the external SPI slave.

```c
typedef struct {
    spi_idx_e   idx;
    bool        init;
    spi_slave_t slave;
} spi_t;
```

- `idx` can be `SPI_IDX_FLASH`, `SPI_IDX_HOST` or `SPI_IDX_HOST_2`,
each referring to a specific _SPI Host IP_.

- `init` is a boolean used to verify if the `spi_t` variable is properly initialized.

- `slave` is the structure containing all information about the external SPI slave.


#### The Slave

The `spi_slave_t` structure is defined as:

```c
typedef struct {
    uint8_t        csid       : 2;
    spi_datamode_e data_mode  : 2;
    bool           full_cycle : 1;
    uint8_t        csn_idle   : 4;
    uint8_t        csn_trail  : 4;
    uint8_t        csn_lead   : 4;
    uint32_t       freq       : 32;
} spi_slave_t;
```

These fields depend on the external SPI slave you intend to communicate with.

- `csid` is the _Chip Select_ ID (i.e. the line to which your slave is connected).
It can only be __0__ or __1__.

- `data_mode` specifies the phase and polarity the slave uses. Possible values are:

    - `SPI_DATA_MODE_0`: Clock polarity 0, phase 0. (__active-high__
    SCK, sampling data on the __leading__ edge, and asserting data on the __trailing__ edge)
    - `SPI_DATA_MODE_1`: Clock polarity 0, phase 1. (__active-high__
    SCK, sampling data on the __trailing__ edge, and asserting data on the __leading__ edge)
    - `SPI_DATA_MODE_2`: Clock polarity 1, phase 0. (__active-low__
    SCK, sampling data on the __leading__ edge, and asserting data on the __trailing__ edge)
    - `SPI_DATA_MODE_3`: Clock polarity 1, phase 1. (__active-low__
    SCK, sampling data on the __trailing__ edge, and asserting data on the __leading__ edge)

- `full_cycle` determines if data should be sampled _half SCK_ cycle or a _full SCK_ cycle 
after shifting out data. 

- `csn_idle`, `csn_trail`, and `csn_lead` indicate the number of _half SCK_ cycles to keep
the _CS_ line active between command segments, after segments, and before segments,
respectively. The actual number of cycles will be the specified number + 1. For instance, if
`csn_lead` is 5, the _CS_ line will go active 6 _half SCK_ cycles before executing the 
command segment.

- `freq` is the maximum frequency of the external SPI device.


To start using the SDK, you must create an `spi_slave_t` and an `spi_t`. However, 
the `spi_t` __can't__ be created directly; it must be created through the `spi_init` 
function of the SDK to avoid unexpected behaviour.

The function is defined as:
```c
spi_t spi_init(spi_idx_e idx, spi_slave_t slave);
```

This function checks the provided parameters and validates the slave configuration. 
If there is any error it will return an invalid `spi_t` structure. This can be verified 
by reading the `init` field of the structure.

Example:
```c
spi_slave_t slave = {
    .csid = 0,
    .csn_idle = 15,
    .csn_lead = 15,
    .csn_trail = 15,
    .data_mode = SPI_DATA_MODE_0,
    .full_cycle = false,
    .freq = FLASH_MAX_FREQ
};

spi_t spi = spi_init(SPI_IDX_FLASH, slave);

if (spi.init) {
    // Properly initialized
}
else {
    // Initialization error
}
```

````{tip}
A slave with predefined standard values can be created using the macro:
```c
#define SPI_SLAVE(csid, freq) (spi_slave_t) { \
    .csid       = csid, \
    .data_mode  = SPI_DATA_MODE_0, \
    .full_cycle = false, \
    .csn_lead   = 10, \
    .csn_trail  = 10, \
    .csn_idle   = 10, \
    .freq       = freq \
}
```
````

#### Transactions

Once the `spi_t` has been successfully initialized, communication with its slave 
device can be performed via transactions, which are lists of command segments executed 
by the _SPI Host IP_.

##### Command Segments

Each command segment consists of a simple instruction dermining the direction of 
data transfer (TX, RX or Bidirectional), its speed (Standard, Dual, or Quad), and
the length of the data.

The command segment structure `spi_segment_t` is as:

```c
typedef struct {
    uint32_t   len  : 24;
    spi_mode_e mode : 4;
} spi_segment_t;
```

- `len` specifies the length in **bytes** of the data managed by this segment. 
For instance, to send 4 32-bit words, the length would be 16.

- `mode` indicates the data transfer direction and speed. Possible values are:

    - `SPI_MODE_DUMMY`  
    - `SPI_MODE_RX_STD` 
    - `SPI_MODE_TX_STD` 
    - `SPI_MODE_BIDIR`  
    - `SPI_MODE_RX_DUAL`
    - `SPI_MODE_TX_DUAL`
    - `SPI_MODE_RX_QUAD`
    - `SPI_MODE_TX_QUAD`

_DUMMY_ mode refers to a period where the _SPI Host IP_ sends _SCK_ pulses without 
reading or sending any data. The number of _SCK_ pulses is determined by the `len` 
field. For example, to send 10 _SCK_ pulses, `len` must be set to 10 (seems evident,
but better safe than sorry).

```{note}
There are no _Dual_ or _Quad_ speeds for _Bidirectional_ mode because the _SPI Host IP_ 
doesn't support it.
```

```{tip}
There are macros for each segment _mode_ simplifying command segment creation.

The macros are: `SPI_SEG_DUMMY`, `SPI_SEG_TX`, `SPI_SEG_RX`, `SPI_SEG_BIDIR`,
`SPI_SEG_TX_DUAL`, `SPI_SEG_RX_DUAL`, `SPI_SEG_TX_QUAD`, and `SPI_SEG_RX_QUAD`

Each macro takes one argument which sets the `len` field.
```

As mentioned above, a transaction comprises various command segments, so an array 
of segments is required.

For example:
```c
spi_segment_t segments[2] = { SPI_SEG_TX(4), SPI_SEG_RX(256) };
```

##### TX/RX Buffers

Since these segments transmit/receive data, appropriate TX and RX buffers are needed. 
These buffers must be large enough to contain the data being transmitted/received to 
avoid memory overwrites.

For our example with 4 bytes of TX and 256 bytes of RX, the buffers would be:

```c
uint32_t src_buffer      = 0;
uint32_t dest_buffer[64] = {0};
```

```{caution}
:name: caution-seglen-bufflen
While segments with lengths not multiple of 4 are allowed, buffers **must** be multiples 
of 4 bytes due to the SDK's handling of write/read operations to the _SPI Host IP_ TX and 
RX _FIFOs_ one word at a time (and not one byte at a time).

It is the _segment length_ that will ultimately determine the amount of bytes effectively
transmitted/received, not the buffer size.

For example, an RX segment of length 3 is fine, but the _dest\_buffer_ should be at least 
4 bytes to avoid unexpected behavior.

This design choice was made to avoid having various alignment computations, thus favoring 
computational speed at the cost of rendering some minimal bytes useless.
```

##### Execution

With the buffers and segments ready, the transaction can be issued using:

```c
spi_codes_e spi_execute(spi_t* spi, const spi_segment_t* segments, uint32_t segments_len, const uint32_t* src_buffer, uint32_t* dest_buffer);
```

This function returns `SPI_CODE_OK` if the transaction is successfully issued. Otherwise, 
it returns an error code.

And, fter a transaction has completed, the result can be checked using:

```c
spi_state_e spi_get_state(spi_t* spi);
```

Which returns `SPI_STATE_DONE` if the transaction completed successfully
(i.e. all segments were completed). Otherwise, it returns `SPI_STATE_ERROR`.

##### Example

A complete example of executing a transaction:

```c
// Assuming spi variable is properly initialized

// Define transaction command segments
spi_segment_t segments[2] = { SPI_SEG_TX(4), SPI_SEG_RX(256) };
// Initialize data buffers
uint32_t src_buffer      = 0;
uint32_t dest_buffer[64] = {0};
// Execute the transaction
if (spi_execute(&spi, segments, 2, &src_buffer, dest_buffer) == SPI_CODE_OK)
{
    if (spi_get_state(&spi) == SPI_STATE_DONE)
    {
        // Transaction successful
    }
    else {
        // Transaction error
    }
}
else {
    // Configuration error
}
```

##### Simplified Transaction Functions

This method is the most configurable. However, there are simpler methods for common 
transactions:

```c
spi_codes_e spi_transmit(spi_t* spi, const uint32_t* src_buffer, uint32_t len);
spi_codes_e spi_receive(spi_t* spi, uint32_t* dest_buffer, uint32_t len);
spi_codes_e spi_transceive(spi_t* spi, const uint32_t* src_buffer, uint32_t* dest_buffer, uint32_t len);
```

```{caution}
The `len` parameter specifies the number of **bytes**, not words.
```

These functions execute basic standard transactions: `spi_transmit` performs a _TX Standard_ 
transaction, `spi_receive` performs an _RX Standard_ transaction, and `spi_transceive` 
performs a _Bidirectional Standard_ transaction. They work like `spi_execute` but do not 
require command segments to be provided.


#### Non-Blocking Transactions

To allow the main program to continue processing while a transaction executes, each 
transaction function has also a _non-blocking_ variant:

```c
spi_codes_e spi_transmit_nb(spi_t* spi, const uint32_t* src_buffer, uint32_t len, spi_callbacks_t callbacks);
spi_codes_e spi_receive_nb(spi_t* spi, uint32_t* dest_buffer, uint32_t len, spi_callbacks_t callbacks);
spi_codes_e spi_transceive_nb(spi_t* spi, const uint32_t* src_buffer, uint32_t* dest_buffer, uint32_t len, spi_callbacks_t callbacks);
spi_codes_e spi_execute_nb(spi_t* spi, const spi_segment_t* segments, uint32_t segments_len, const uint32_t* src_buffer, uint32_t* dest_buffer, spi_callbacks_t callbacks);
```

These functions differ in two aspects from their _blocking_ counterpart. First, these 
functions return immediately after launching the transaction, allowing other data 
processing while the transaction completes. Secondly, they take an additional parameter 
of type `spi_callbacks_t`, defined as:

```c
typedef struct {
    spi_cb_t done_cb;   // Called once transaction is done
    spi_cb_t txwm_cb;   // Called each time TX watermark event is triggered
    spi_cb_t rxwm_cb;   // Called each time RX watermark event is triggered
    spi_cb_t error_cb;  // Called when there was an error during transaction
} spi_callbacks_t;
```

##### Callbacks

Each field of `spi_callbacks_t` is of type `spi_cb_t`, defined as:

```c
typedef void (*spi_cb_t)(const uint32_t*, uint32_t, uint32_t*, uint32_t);
```

This _callback_ type is invoked with four arguments whenever the corresponding event 
occurs:

1. The _TX buffer_ associated to the transaction.
1. The current number of items in the _TX buffer_.
1. The _RX buffer_ associated to the transaction.
1. The current number of items in the _RX buffer_.

Callbacks can be `NULL` if not needed.

In this context, the function `spi_get_state` previously explained, can be used to 
check if the transaction is still ongoing (`SPI_STATE_BUSY`) or has finished. Once
finished the state will also become `SPI_STATE_DONE` or `SPI_STATE_ERROR` depending 
on the result of the transaction.

##### Example

```c
void done_cb(const uint32_t* txbuff, uint32_t txlen, uint32_t* rxbuff, uint32_t rxlen)
{
    // Do stuff with the data once the transaction is done
}

int main(int argc, char *argv[])
{
    // ...

    // We assume an spi variable has been created by properly initializing it.

    uint32_t src_buffer = 0x1234    // For example a command to send to slave
    uint32_t dest_buffer[12] = {0}  // Slave's response

    // Notice that the segments' length are not multiples of 4.
    spi_segment_t segments[2] = { SPI_SEG_TX(2), SPI_SEG_RX(45) };

    // Define the callbacks. Notice we can assign NULL when we do not want any
    // callback to be called
    spi_callbacks_t callbacks = {
        .done_cb  = &done_cb,
        .error_cb = NULL,
        .rxwm_cb  = NULL,
        .txwm_cb  = NULL
    };

    if (!spi_execute_nb(&spi, segments, 2, &src_buffer, dest_buffer, callbacks))
    {
        while(spi_get_state(spi) == SPI_STATE_BUSY)
        {
            // Do stuff while SPI is busy
        }
    }
    else {
        // There was an error in the transaction configuration
    }

    // ...
}
```

```{note}
In this example, the _TX segment length_ is 2 while _src\_buffer_ is 4 bytes long, 
and the _RX segment length_ is 45 while _dest\_buffer_ is 48 bytes long.
For explanation, please refer to [this directive](#caution-seglen-bufflen).
```

## Operation

### HAL

#### Overview

The X-HEEP platform supports three instances of the _SPI Host IP_, defined as:

- `spi_host1`
- `spi_host2`
- `spi_flash`

These macros expand to variables of type `spi_host_t*`. They must be passed to any 
HAL function requiring to reference the specific peripheral.



#### Return Flags

Most functions in the HAL return an enum value of type `spi_return_flags_e`, indicating 
the outcome of the operation. Below is the complete list of return flags:

```c
typedef enum {
    SPI_FLAG_OK                 = 0x0000, /*!< Everithing went well */
    SPI_FLAG_NULL_PTR           = 0x0001, /*!< The SPI variabled passed was a null pointer */
    SPI_FLAG_WATERMARK_EXCEEDS  = 0x0002, /*!< The Watermark exceeded SPI_HOST_PARAM_TX_DEPTH 
    or SPI_HOST_PARAM_RX_DEPTH and was therefore not set */
    SPI_FLAG_CSID_INVALID       = 0x0004, /*!< The CSID was out of the bounds specified in 
    SPI_HOST_PARAM_NUM_C_S */
    SPI_FLAG_COMMAND_FULL       = 0x0008, /*!< The CMD FIFO is currently full so couldn't write command */
    SPI_FLAG_SPEED_INVALID      = 0x0010, /*!< The specified speed is not valid so couldn't write command */
    SPI_FLAG_TX_QUEUE_FULL      = 0x0020, /*!< The TX Queue is full, thus could not write to TX register */
    SPI_FLAG_RX_QUEUE_EMPTY     = 0x0040, /*!< The RX Queue is empty, thus could not read from RX register */
    SPI_FLAG_NOT_READY          = 0x0080, /*!< The SPI is not ready */
    SPI_FLAG_EVENT_INVALID      = 0x0100, /*!< The event to enable is not a valid event */
    SPI_FLAG_ERROR_INVALID      = 0x0200  /*!< The error irq to enable is not a valid error irq */
} spi_return_flags_e;
```


**Note**: All functions in the HAL returning a `spi_return_flags_e` will **always** 
return `SPI_FLAG_NULL_PTR` when the argument `spi_host_t* spi` is a `NULL` pointer. To 
avoid repetition, the `SPI_FLAG_NULL_PTR` will be omitted from discussions of function 
return values.


#### Target Device Configuration

Using the HAL begins by configuring the slave device(s) you intend to communicate 
with. For this purpose, a configuration options structure, `spi_configopts_t`, is 
provided, along with functions to write the slave configuration options to the 
_SPI Host IP_.

##### Configuration Options Structure

```c
typedef struct spi_configopts_s {
    uint16_t clkdiv     : 16; // The clock divider to use with a paricular slave
    uint8_t  csnidle    : 4;  // Indicates the minimum number of sck half-cycles to hold cs_n high between commands
    uint8_t  csntrail   : 4;  // Indicates the number of half sck cycles, CSNTRAIL+1, to leave between last edge of sck and the rising edge of cs_n
    uint8_t  csnlead    : 4;  // Indicates the number of half sck cycles, CSNLEAD+1, to leave between the falling edge of cs_n and the first edge of sck
    bool     __rsvd0    : 1;  // Will be ignored by hardware
    bool     fullcyc    : 1;  // If 1 data is sampled a full cycle after shifting data out, instead of half cycle
    bool     cpha       : 1;  // If 0 data lines change on trailing edge and sample done on leading edge, if 1 it is the opposite
    bool     cpol       : 1;  // If 0 sck is low when idle, and emits high pulses. If 1 sck is high when idle, and emits of low pulses
} spi_configopts_t;

```

##### Configuration Functions

```c
uint32_t spi_create_configopts(const spi_configopts_t configopts);

spi_return_flags_e spi_set_configopts(spi_host_t* spi, uint32_t csid, const uint32_t conf_reg);
```

##### Configuration Steps

To create and write the configuration options to the hardware:

1. **Define Configuration**: Create a `spi_configopts_t` structure with the desired 
settings.
1. **Create Configuration Word**: Call `spi_create_configopts` with the structure 
to generate a 32-bit configuration word.
1. **Apply Configuration**: Call `spi_set_configopts` with the SPI instance, _Chip Select_ 
ID (`csid`), and the configuration word.
1. **Verify Configuration**: Ensure the function returns `SPI_FLAG_OK` to confirm 
successful configuration.

The configuration options persist until overwritten or the _SPI Host IP_ device is reset.


#### Enabling the SPI Host

After configuring the slave, enable the _SPI Host IP_ and its output buffers:

```c
spi_return_flags_e spi_set_enable(spi_host_t* spi, bool enable);
spi_return_flags_e spi_output_enable(spi_host_t* spi, bool enable);
```

Both functions must be called to start communication:

- `spi_set_enable`: Enables the _SPI Host IP_.
- `spi_output_enable`: Enables the output buffers for _SCK_, _CSB_, and _SD_ lines.



#### Communication Commands

Communication with SPI slaves involves creating and inputting commands to the 
_SPI Host IP_. Each command instructs the _SPI Host IP_ to execute a transfer 
(transmit/receive) for a certain amount of bytes, in a specific direction and speed.


##### Command Structure

```c
typedef struct spi_command_s {
    uint32_t    len         : 24; // Length-1 in bytes for the command to transmit/receive
    bool        csaat       : 1;  // Keep CS line active after command has finished (allows to instruct series of commands)
    spi_speed_e speed       : 2;  // Speed of communication
    spi_dir_e   direction   : 2;  // Direction of communication
} spi_command_t;
```

- `len`: Length of bytes - 1 to transmit/receive.
In the case of `SPI_DIR_DUMMY` direction, it represents the number of _SCK_ impulses 
to send without transmitting or receiving any data.
- `csaat`: Keeps CS line active after the command (see [Advanced Commands](#hal-advanced-commands)).
- `speed`: Communication speed (`SPI_SPEED_STANDARD`, `SPI_SPEED_DUAL`, `SPI_SPEED_QUAD`).
- `direction`:Communication direction (`SPI_DIR_DUMMY`, `SPI_DIR_RX_ONLY`, `SPI_DIR_TX_ONLY`,
`SPI_DIR_BIDIR`).

```{note}
The direction `SPI_DIR_BIDIR` can only use the speed `SPI_SPEED_STANDARD`. Configuring 
`SPI_DIR_BIDIR` with any other speed will return `SPI_FLAG_SPEED_INVALID` when issuing
the command.
```

##### Setting Chip Select ID

Specify the target device by setting the Chip Select line:

```c
spi_return_flags_e spi_set_csid(spi_host_t* spi, uint32_t csid);
```

- `csid`: _Chip Select_ line (0 or 1).

This function returns `SPI_FLAG_CSID_INVALID` if the `csid` is out of range, or 
`SPI_FLAG_OK` if set correctly.


##### Loading TX FIFO

If the command transmits data (`SPI_DIR_TX_ONLY` or `SPI_DIR_BIDIR`), load the 
TX _FIFO_ with data:

```c
spi_return_flags_e spi_write_word(spi_host_t* spi, uint32_t wdata);

spi_return_flags_e spi_write_byte(spi_host_t* spi, uint8_t bdata);
```

You can load only one 32-bit word or one byte at a time. Therefore, iterate over 
all the data you want to transmit until either the _FIFO_ is full or all the data 
is loaded.

Both write functions will return either `SPI_FLAG_TX_QUEUE_FULL` if the TX _FIFO_ is 
full or `SPI_FLAG_OK` if the data has been loaded into the _FIFO_.


##### Verifying Readiness

Ensure the SPI Host IP is ready to receive commands:

```c
spi_tristate_e spi_get_ready(spi_host_t* spi);
```

- Returns `SPI_TRISTATE_TRUE` if ready, `SPI_TRISTATE_FALSE` if not, and 
`SPI_TRISTATE_ERROR` if the spi pointer is `NULL`.

Alternatively, you may wait for readiness:

```c
spi_return_flags_e spi_wait_for_ready(spi_host_t* spi);
```

##### Issuing Commands

```c
uint32_t spi_create_command(const spi_command_t command);

spi_return_flags_e spi_set_command(spi_host_t* spi, uint32_t cmd_reg);
```

Issue commands by following these steps:

1. **Create Command Word**: Call `spi_create_command` with the command structure.
1. **Send Command**: Call `spi_set_command` with the generated command word.
1. **Check for Errors**: `spi_set_command` returns `SPI_FLAG_SPEED_INVALID` for 
invalid speed, `SPI_FLAG_NOT_READY` if not ready, or `SPI_FLAG_OK` if successful.


##### Reading Received Data

For RX or Bidirectional commands, read received data with:

```c
spi_return_flags_e spi_read_word(spi_host_t* spi, uint32_t* dst);
```

This function returns `SPI_FLAG_RX_QUEUE_EMPTY` if no words are available. Otherwise, 
stores the read word in `uint32_t* dst`.

```{note}
There is no function to read single bytes from the RX _FIFO_ because the _SPI Host IP_ 
only offers entire words to be read. The unused bytes (when the RX command length 
parameter does not describe full words) will be zero-padded.
```


(hal-advanced-commands)=
##### Advanced Commands

For complex commands (e.g. TX followed by RX without deactivating the CS line, etc.), 
use the `csaat` field to keep the _CS_ line active:

- Set `csaat` to `true` to keep the _CS_ line active.
- Repeat the command steps until the last _segment_, where `csaat` should be `false`.



#### SPI Host Status

To retrieve the current status of the _SPI Host IP_, a structure and a function 
are provided:

```c
typedef struct spi_status_s {
    uint8_t txqd        : 8;  // TX queue depth (how many unsent words are in the FIFO)
    uint8_t rxqd        : 8;  // RX queue depth (how many unread words are in the FIFO)
    uint8_t cmdqd       : 4;  // CMD queue depth (how many unprocessed commands are in the FIFO)
    bool    rxwm        : 1;  // Indicates wether rxqd is above the RX Watermark
    bool    __rsvd0     : 1;  // Not used
    bool    byteorder   : 1;  // The endianness of the SPI Peripheral
    bool    rxstall     : 1;  // Indicates if the SPI still still has more data to read but the RX FIFO is full
    bool    rxempty     : 1;  // Indicates RX FIFO is empty
    bool    rxfull      : 1;  // Indicates RX FIFO is full
    bool    txwm        : 1;  // Indicates wether txqd is below the TX Watermark
    bool    txstall     : 1;  // Indicates if the SPI still has more data to send but the TX FIFO is empty
    bool    txempty     : 1;  // Indicates TX FIFO is empty
    bool    txfull      : 1;  // Indicates TX FIFO is full
    bool    active      : 1;  // Indicates if the SPI peripheral is currently processing a command
    bool    ready       : 1;  // Indicates if the SPI peripheral is ready to receive more commands
} spi_status_t;

const volatile spi_status_t* spi_get_status(spi_host_t* spi);
```

Calling this function provides read access to all the fields in `spi_status_t`, 
mapped to the `STATUS` register of the _SPI Host_ device.

If `spi_host_t* spi` is `NULL`, this function returns a `NULL` pointer.
Check for `NULL` before accessing fields.




## Using HAL Together With SDK