// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Basic device functions for opentitan SPI host

#ifndef _DRIVERS_SPI_MEMIO_H_
#define _DRIVERS_SPI_MEMIO_H_

#include <stdint.h>

#include "mmio.h"
#include "spimem_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialization parameters for SPI MEMIO.
 *
 */
typedef struct spi_memio {
    /**
    * The base address for the SPI MEMIO hardware registers.
    */
    mmio_region_t base_addr;
} spi_memio_t;

#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_SPI_MEMIO_H_
