// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "handler.h"

/**
 * Weak definition of the external IRQ handler meant to be overwritten
 */
__attribute__((weak)) void external_irq_handler(void) {
  printf("External IRQ triggered!\n");
  while (1) {
  }
}
