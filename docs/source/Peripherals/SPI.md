# SPI Host

## Basics

### SDK

There are a few major structures to operate with the SDK.

The most important one is `spi_t`. This is the structure holding all the 
information needed to identify an external SPI device. It defines which of the
_SPI Host IP_ hardware to use and holds all the data relative to the external SPI slave.

```c
typedef struct {
    spi_idx_e   idx;
    bool        init;
    spi_slave_t slave;
} spi_t;
```

- `idx` can take the values `SPI_IDX_FLASH`, `SPI_IDX_HOST` or `SPI_IDX_HOST_2`,
each refering to a particular _SPI Host IP_.

- `init` is a boolean variable used internally to verify the proper initialization
of the `spi_t` variable.

- `slave` is the structure holding all the information relative to the external
SPI slave.

#### The Slave

The `spi_slave_t` structure is defined as follows:

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

All these fields are dependent on the external SPI device you are with which you
want to communicate.

- `csid` is the _Chip Select_ id (i.e. on what line your slave is connected).
This id can only take the value __0__ or __1__.

- `data_mode` refers to the phase and polarity the slave is using. It can take
one of the following values:

    - `SPI_DATA_MODE_0`: Clock polarity 0, phase 0. This translates to an __active-high__
    SCK, sampling data on the __leading__ edge, and asserting data on the __trailing__ edge.
    - `SPI_DATA_MODE_1`: Clock polarity 0, phase 1. This translates to an __active-high__
    SCK, sampling data on the __trailing__ edge, and asserting data on the __leading__ edge.
    - `SPI_DATA_MODE_2`: Clock polarity 1, phase 0. This translates to an __active-low__
    SCK, sampling data on the __leading__ edge, and asserting data on the __trailing__ edge.
    - `SPI_DATA_MODE_3`: Clock polarity 1, phase 1. This translates to an __active-low__
    SCK, sampling data on the __trailing__ edge, and asserting data on the __leading__ edge.

- `full_cycle` determines if data should be sampled _half SCK_ cycle after shifting out data
or if it should be sampled a _full SCK_ cycle after shifting out data. 

- `csn_idle`, `csn_trail`, and `csn_lead` are the number of _half SCK_ cycles to keep
the _CS_ line high between command segments, after the segments, and before the segments,
respectively. The number of cycles will be the specified number + 1. For example, if
`csn_lead` is set to 5, the _CS_ will go active 6 _half SCK_ cycles before commencing
to execute the command segment.

- `freq` is the maximum frequency of the external SPI device.


In order to start using the SDK, it is indispensable to start by creating an `spi_slave_t`
and an `spi_t`. However, the `spi_t` __can't__ be created directly; it must be created
through the `spi_init` function of the SDK. Failing to do so could end up with unexpected
behaviour.

The function is defined as follows:
```c
spi_t spi_init(spi_idx_e idx, spi_slave_t slave);
```

This function will check the provided parameters and validate that the slave is correctly
configured. If there is any error it will return an invalid `spi_t` structure. This can
be verified by reading the `init` field of the structure.

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
    // Has been properly initialized
}
else {
    // Something was wrong
}
```

````{tip}
A slave with predefined standard values can be created with the macro:
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

Once the `spi_t` has been successfully created (via initialization), we can start
Communicating with its slave device. Communication is done via transactions, which
consist in a list of command segments to be executed by the _SPI Host IP_.

Each command segment consists of a simple instruction of TX, RX or Bidirectional
data transfers and its associated speed (Standard, Dual, or Quad).

The command segment structure `spi_segment_t` is as follows:

```c
typedef struct {
    uint32_t   len  : 24;
    spi_mode_e mode : 4;
} spi_segment_t;
```

- `len` is the length in **bytes** of the data this segment manages. For example,
to send 4 32-bit words, the length would be 16.

- `mode` determines the direction of data transfer and the speed. It can take the
values:

    - `SPI_MODE_DUMMY`  
    - `SPI_MODE_RX_STD` 
    - `SPI_MODE_TX_STD` 
    - `SPI_MODE_BIDIR`  
    - `SPI_MODE_RX_DUAL`
    - `SPI_MODE_TX_DUAL`
    - `SPI_MODE_RX_QUAD`
    - `SPI_MODE_TX_QUAD`

_DUMMY_ mode refers to a period where the _SPI Host IP_ will send _SCK_ pulses but will
neither read nor send data. The amount of _SCK_ cylces will be determined by the
`len` field. For example, to send 10 _SCK_ pulses, `len` must be set to 10.

```{note}
There is no _Dual_ or _Quad_ speed for the _Bidirectional_ mode. This is because 
the _SPI Host IP_ doesn't support it.
```

```{tip}
Note: For each segment _mode_ there is also a macro defined, which allows for simple
command segment creation.

The macros are: `SPI_SEG_DUMMY`, `SPI_SEG_TX`, `SPI_SEG_RX`, `SPI_SEG_BIDIR`,
`SPI_SEG_TX_DUAL`, `SPI_SEG_RX_DUAL`, `SPI_SEG_TX_QUAD`, and `SPI_SEG_RX_QUAD`

Each of these macros take one argument which is set to the `len` field.
```

As mentioned before, a transaction comprises various command segments. Therefore,
in order to execute transactions we have to make an array of segments.

For example:
```c
spi_segment_t segments[2] = { SPI_SEG_TX(4), SPI_SEG_RX(256) };
```

Since these segments will transmit/receive data, we also need a place to store that
data. Hence, we need some TX and RX buffers that we will give to the SDK. These buffers
must imperatively be large enough to contain the data being transmitted/received.
Otherwise we may overwrite memory locations that we should't be modifying.

Therefore, since in our example we used 4 bytes of TX and 256 bytes of RX, we could
declare the following buffers:

```c
uint32_t src_buffer      = 0;
uint32_t dest_buffer[64] = {0};
```

```{caution}
We could make segments with lengths that are not multiple of 4 (which is perfectly 
fine), but the buffers _absolutely_ need to be of a size multiple of 4 bytes. This
is due to the fact that the SDK handles the write/read to the _SPI Host IP_ TX and
RX _FIFOs_ one word at a time (and not one byte).

For example, while it is totally fine to have an RX segment of length 3, the
_dest\_buffer_ should be of at least 4 bytes in order to avoid unexpected behaviour.

The decision of implementing the SDK in this way, was because many more computations
would have been required to track the alignment of each particular segment with
respect to the buffer. And since transactions read/write are handled during interrupts
it seemed a fair trade-off to lose some "useless bytes" in order to gain computational
speed.
```


Now, once we have our buffers, and our segments ready we can issue the transaction
by calling the function:

```c
spi_codes_e spi_execute(spi_t* spi, const spi_segment_t* segments, uint32_t segments_len, const uint32_t* src_buffer, uint32_t* dest_buffer);
```

Hence, our complete example for executing a transaction would be:

```c
// Defining the transaction command segments
spi_segment_t segments[2] = { SPI_SEG_TX(4), SPI_SEG_RX(256) };
// Initializating the data buffers
uint32_t src_buffer      = 0;
uint32_t dest_buffer[64] = {0};
// Executing the transaction
spi_execute(&spi, segments, 2, &src_buffer, dest_buffer);
```