# SPI Host

## Basics

### SDK

There are a few major structures to operate with the SDK.

The most important one is `spi_t`. This is the structure holding all the 
information needed to identify an external SPI device. It defines which of the
_SPI IP_ hardware to use and holds all the data relative to the external SPI slave.

```
typedef struct {
    spi_idx_e   idx;
    bool        init;
    spi_slave_t slave;
} spi_t;
```

Here, `idx` can take the values `SPI_IDX_FLASH`, `SPI_IDX_HOST` or `SPI_IDX_HOST_2`,
each refering to a particular _SPI IP_.

`init` is a boolean variable used internally to verify the proper initialization
of the `spi_t` variable.

And `slave` is the structure holding all the information relative to the external
SPI slave.

#### The Slave

The `spi_slave_t` structure is defined as follows:

```
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

- `csn_idle`, `csn_trail`, and `csn_lead` are the number of _half SCK_ cycles to keep
the _CS_ line high between command segments, after the segments, and before the segments,
respectively. The number of cycles will be the specified number + 1. For example, if
`csn_lead` is set to 5, the _CS_ will go active 6 _half SCK_ cycles before commencing
to execute the command segment.

- `freq` is the maximum frequency of the external SPI device.






### HAL

## Usage

### SDK

In order to start using the SDK, the following steps must be completed before being
able to start any communication:

- Create an `spi_slave_t` with the parameters relevant to the device with which
you want to communicate.
- Call `spi_init` and store the returned `spi_t` structure in a variable.

Example:
```
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
```



In order to use the sdk, the first thing that should be done is to initialize the desired spi peripheral (flash, host, host2)
with the function spi_init. This function will return an spi structure that will have to be used whenever calling any other
function of the sdk.

There are multiple ways of executing spi transactions. For simple transmit, receive or transceive there are functions that
just have to be called passing a buffer of the apropriate size along with its size. These functions will issue the transaction,
wait until it is finished and return. The buffer will have been filled or emptied according to the size provided.

There is also a more flexible manner of issuing transactions for more complex transactions (different speeds, multiple speeds,
etc.). This can be achieved with the execute function

### HAL

## Testing

### Available Applications

## 😎 X-pert Zone