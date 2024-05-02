# SPI Host

## Usage

### SDK

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