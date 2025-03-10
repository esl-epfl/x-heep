// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Modified version for core-v-mini-mcu
// original at: https://github.com/lowRISC/opentitan/blob/master/sw/

#ifndef _DRIVERS_UART_H_
#define _DRIVERS_UART_H_

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NCO_WIDTH 16

/**
 * Initialization parameters for UART.
 *
 */
typedef struct uart {
  /**
   * The base address for the UART hardware registers.
   */
  mmio_region_t base_addr;
  /**
   * The desired baudrate of the UART.
   */
  uint32_t baudrate;
  /**
  * The NCO to control the baudrate
  */
  uint64_t nco;
  /**
   * The peripheral clock frequency (used to compute the UART baudrate divisor).
   */
  uint32_t clk_freq_hz;
} uart_t;

/**
 * Initialize the UART with the request parameters.
 *
 * @param uart Pointer to uart_t with the requested parameters.
 * @return kErrorOk if successful, else an error code.
 */
system_error_t uart_init(const uart_t *uart);

/**
 * Write a single byte to the UART.
 *
 * @param uart Pointer to uart_t represting the target UART.
 * @param byte Byte to send.
 */
void uart_putchar(const uart_t *uart, uint8_t byte);

/**
 * Write a buffer to the UART.
 *
 * Writes the complete buffer to the UART and wait for transmision to complete.
 *
 * @param uart Pointer to uart_t represting the target UART.
 * @param data Pointer to buffer to write.
 * @param len Length of the buffer to write.
 * @return Number of bytes written.
 */
size_t uart_write(const uart_t *uart, const uint8_t *data, size_t len);

/**
 * Sink a buffer to the UART.
 *
 * This is a wrapper for uart_write which conforms to the type signature
 * required by the print library.
 *
 * @param uart Pointer to uart_t represting the target UART.
 * @param data Pointer to buffer to write.
 * @param len Length of the buffer to write.
 * @return Number of bytes written.
 */

size_t uart_getchar(const uart_t *uart, uint8_t *data);

size_t uart_read(const uart_t *uart, const uint8_t *data, size_t len);

size_t uart_sink(void *uart, const char *data, size_t len);


/**
 * @brief Attends the plic interrupt.
 */
__attribute__((weak, optimize("O0"))) void handler_irq_uart(uint32_t id);

#ifdef __cplusplus
}
#endif

#endif  // OPENTITAN_SW_DEVICE_SILICON_CREATOR_LIB_DRIVERS_UART_H_
