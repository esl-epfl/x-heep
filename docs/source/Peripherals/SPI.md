# SPI Host

```{contents} Table of Contents
:depth: 4
```

## Basics

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

Each field is of type `spi_cb_t`, defined as:

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

Example implementation:

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