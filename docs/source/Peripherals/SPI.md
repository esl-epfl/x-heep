# SPI Host Driver Documentation

## File Overview
- **File**: `spi_host.h`
- **Date**: 06/03/24
- **Brief**: This file provides the API for the Serial Peripheral Interface (SPI) driver. It allows setting up and using the SPI peripheral for communication with various devices.

## Definitions

- `SPI_MAX_IDX`: The maximum index for SPI peripherals.
- `SPI_IDX_INVALID(idx)`: Macro to check if an SPI index is invalid.
- `SPI_CSID_INVALID(csid)`: Macro to check if a Chip Select ID (CSID) is invalid.

## Enums

### `spi_idx_e`
Defines SPI peripheral indexes.
- `SPI_IDX_FLASH`: SPI index for Flash.
- `SPI_IDX_MMIO`: SPI index for MMIO.
- `SPI_IDX_HOST`: SPI index for the Host.
- `SPI_IDX_HOST_2`: SPI index for the secondary Host.

### `spi_speed_e`
Defines SPI speed types.
- `SPI_SPEED_STANDARD`: Standard speed.
- `SPI_SPEED_DUAL`: Dual speed.
- `SPI_SPEED_QUAD`: Quad speed.

### `spi_dir_e`
Defines SPI directionality.
- `SPI_DIR_DUMMY`: Dummy direction.
- `SPI_DIR_RX_ONLY`: Receive only.
- `SPI_DIR_TX_ONLY`: Transmit only.
- `SPI_DIR_BIDIR`: Bidirectional.

### `spi_event_e`
Defines SPI events.
- `SPI_EVENT_NONE`: No event.
- `SPI_EVENT_RXFULL`: RX Full.
- `SPI_EVENT_TXEMPTY`: TX Empty.
- `SPI_EVENT_RXWM`: RX Watermark.
- `SPI_EVENT_TXWM`: TX Watermark.
- `SPI_EVENT_READY`: Ready.
- `SPI_EVENT_IDLE`: Idle.
- `SPI_EVENT_ALL`: All events.

### `spi_error_e`
Defines SPI errors.
- Various errors such as `SPI_ERROR_NONE`, `SPI_ERROR_CMDBUSY`, etc.

### `spi_return_flags_e`
Defines SPI function return flags.
- Flags such as `SPI_FLAG_OK`, `SPI_FLAG_NULL_PTR`, etc.

## Structures

### `spi_host_t`
Initialization parameters for SPI.

### `spi_ch_status_t`
SPI channel (TX/RX) status structure.

### `spi_configopts_t`
SPI chip (slave) configuration structure.

### `spi_command_t`
SPI command structure.

## Functions

- **Initialization**
  - `spi_idx_e spi_init_flash()`: Initialize SPI for flash device.
  - `spi_idx_e spi_init_host()`: Initialize SPI for host device.
  - `spi_idx_e spi_init_host2()`: Initialize SPI for secondary host device.

- **Event and Error Management**
  - `spi_return_flags_e spi_get_events_enabled(const spi_idx_e peri_id, spi_event_e* events)`: Get enabled events.
  - `spi_return_flags_e spi_set_events_enabled(const spi_idx_e peri_id, spi_event_e* events, bool enable)`: Set enabled events.
  - `spi_return_flags_e spi_get_errors_enabled(const spi_idx_e peri_id, spi_error_e* errors)`: Get enabled error interrupts.
  - `spi_return_flags_e spi_set_errors_enabled(const spi_idx_e peri_id, spi_error_e* errors, bool enable)`: Set enabled error interrupts.

- **Data Transmission**
  - `spi_return_flags_e spi_write_byte(const spi_idx_e peri_id, uint8_t bdata)`: Write a byte of data.
  - Various other functions for managing SPI communication, such as `spi_enable_error_intr_test`, `spi_enable_evt_intr_test`, etc.

- **Queue and FIFO Management**
  - Functions to manage TX/RX FIFOs and queue depths, such as `spi_get_tx_queue_depth`, `spi_get_rx_queue_depth`, etc.

- **Configuration and Status**
  - Functions to configure SPI settings, read status registers, and manage chip selects, such as `spi_sw_reset`, `spi_set_enable`, `spi_set_csid`, etc.

- **Inline Functions**
  - Inline functions for quick status checks and configurations, such as `spi_get_evt_intr_state`, `spi_wait_for_ready`, etc.

## Weak Functions
- Functions like `spi_intr_handler_event_flash`, `spi_intr_handler_error_flash`, etc., are provided with a weak linkage. Users can override these with their implementations.