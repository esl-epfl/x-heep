// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Modified version for core-v-mini-mcu
// original at: https://github.com/lowRISC/opentitan/blob/master/sw/
#ifdef __cplusplus
extern "C" {
#endif
#include "uart.h"

#include <stddef.h>
#include <stdint.h>
#include "assert.h"
#include "bitfield.h"
#include "mmio.h"
#include "error.h"

#include "uart_regs.h"  // Generated.

#define NCO_WIDTH 16

//#ifdef __cplusplus
static_assert((1UL << NCO_WIDTH) - 1 == UART_CTRL_NCO_MASK, "Bad value for NCO_WIDTH");
//#else
static_assert((1UL << NCO_WIDTH) - 1 == UART_CTRL_NCO_MASK, "Bad value for NCO_WIDTH");
//#endif

static void uart_reset(const uart_t *uart) {
  mmio_region_write32(uart->base_addr, UART_CTRL_REG_OFFSET, 0u);

  // Write to the relevant bits clears the FIFOs.
  uint32_t reg = 0;
  reg = bitfield_bit32_write(reg, UART_FIFO_CTRL_RXRST_BIT, true);
  reg = bitfield_bit32_write(reg, UART_FIFO_CTRL_TXRST_BIT, true);
  mmio_region_write32(uart->base_addr, UART_FIFO_CTRL_REG_OFFSET, reg);

  mmio_region_write32(uart->base_addr, UART_OVRD_REG_OFFSET, 0u);
  mmio_region_write32(uart->base_addr, UART_TIMEOUT_CTRL_REG_OFFSET, 0u);
  mmio_region_write32(uart->base_addr, UART_INTR_ENABLE_REG_OFFSET, 0u);
  mmio_region_write32(uart->base_addr, UART_INTR_STATE_REG_OFFSET, UINT32_MAX);
}

system_error_t uart_init(const uart_t *uart) {
  if (uart == NULL) {
    return kErrorUartInvalidArgument;
  }

  if (uart->baudrate == 0 || uart->clk_freq_hz == 0) {
    return kErrorUartInvalidArgument;
  }

  // Calculation formula: NCO = 16 * 2^nco_width * baud / fclk.
  // NCO creates 16x of baudrate. So, in addition to the nco_width,
  // 2^4 should be multiplied.
  uint64_t nco =
      ((uint64_t)uart->baudrate << (NCO_WIDTH + 4)) / uart->clk_freq_hz;
  uint32_t nco_masked = nco & UART_CTRL_NCO_MASK;

  // Requested baudrate is too high for the given clock frequency.
  if (nco != nco_masked) {
    return kErrorUartBadBaudRate;
  }

  // Must be called before the first write to any of the UART registers.
  uart_reset(uart);

  // Set baudrate, TX, no parity bits.
  uint32_t reg = 0;
  reg = bitfield_field32_write(reg, UART_CTRL_NCO_FIELD, nco_masked);
  reg = bitfield_bit32_write(reg, UART_CTRL_TX_BIT, true);
  reg = bitfield_bit32_write(reg, UART_CTRL_PARITY_EN_BIT, false);
  reg = bitfield_bit32_write(reg, UART_CTRL_RX_BIT, true);
  mmio_region_write32(uart->base_addr, UART_CTRL_REG_OFFSET, reg);

  // Disable interrupts.
  mmio_region_write32(uart->base_addr, UART_INTR_ENABLE_REG_OFFSET, 0u);
  return kErrorOk;
}

static bool uart_tx_full(const uart_t *uart) {
  uint32_t reg = mmio_region_read32(uart->base_addr, UART_STATUS_REG_OFFSET);
  return bitfield_bit32_read(reg, UART_STATUS_TXFULL_BIT);
}

static bool uart_tx_idle(const uart_t *uart) {
  uint32_t reg = mmio_region_read32(uart->base_addr, UART_STATUS_REG_OFFSET);
  return bitfield_bit32_read(reg, UART_STATUS_TXIDLE_BIT);
}

static bool uart_rx_empty(const uart_t *uart) {
  uint32_t reg = mmio_region_read32(uart->base_addr, UART_STATUS_REG_OFFSET);
  return bitfield_bit32_read(reg, UART_STATUS_RXEMPTY_BIT);
}

void uart_putchar(const uart_t *uart, uint8_t byte) {
  // If the transmit FIFO is full, wait.
  while (uart_tx_full(uart)) {
  }
  uint32_t reg = bitfield_field32_write(0, UART_WDATA_WDATA_FIELD, byte);
  mmio_region_write32(uart->base_addr, UART_WDATA_REG_OFFSET, reg);

  // If the transmitter is active, wait.
  while (!uart_tx_idle(uart)) {
  }
}

static uint8_t uart_rx_fifo_read(const uart_t *uart) {
  uint32_t reg = mmio_region_read32(uart->base_addr, UART_RDATA_REG_OFFSET);

  return bitfield_field32_read(reg, UART_RDATA_RDATA_FIELD);
}

/**
* Read 1 byte from the RX FIFO.
*/
size_t uart_getchar(const uart_t *uart, uint8_t *data) {
  while (uart_rx_empty(uart));
  *data = uart_rx_fifo_read(uart);
  return 1;
}

/**
 * Write `len` bytes to the UART TX FIFO.
 */
size_t uart_write(const uart_t *uart, const uint8_t *data, size_t len) {
  size_t total = len;
  while (len) {
    uart_putchar(uart, *data);
    data++;
    len--;
  }
  return total;
}

/**
 * Read `len` bytes from the UART RX FIFO.
 */
size_t uart_read(const uart_t *uart, const uint8_t *data, size_t len) {
  size_t total = len;
  while (len) {
    uart_getchar(uart, data);
    data++;
    len--;
  }
  return total;
}

size_t uart_sink(void *uart, const char *data, size_t len) {
  return uart_write((const uart_t *)uart, (const uint8_t *)data, len);
}

__attribute__((weak, optimize("O0"))) void handler_irq_uart(uint32_t id)
{
 // Replace this function with a non-weak implementation
}

#ifdef __cplusplus
}
#endif