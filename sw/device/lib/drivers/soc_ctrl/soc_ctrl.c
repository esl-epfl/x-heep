// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "soc_ctrl.h"

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#include "soc_ctrl.h"
#include "soc_ctrl_regs.h"  // Generated.

void soc_ctrl_set_valid(const soc_ctrl_t *soc_ctrl, uint8_t valid) {
  mmio_region_write8(soc_ctrl->base_addr, (ptrdiff_t)(SOC_CTRL_EXIT_VALID_REG_OFFSET), valid);
}

void soc_ctrl_set_exit_value(const soc_ctrl_t *soc_ctrl, uint32_t exit_value) {
  mmio_region_write32(soc_ctrl->base_addr, (ptrdiff_t)(SOC_CTRL_EXIT_VALUE_REG_OFFSET), exit_value);
}

uint32_t soc_ctrl_get_frequency(const soc_ctrl_t *soc_ctrl) {
  return mmio_region_read32(soc_ctrl->base_addr, (ptrdiff_t)(SOC_CTRL_SYSTEM_FREQUENCY_HZ_REG_OFFSET));
}

void soc_ctrl_set_frequency(const soc_ctrl_t *soc_ctrl, uint32_t frequency) {
  mmio_region_write32(soc_ctrl->base_addr, (ptrdiff_t)(SOC_CTRL_SYSTEM_FREQUENCY_HZ_REG_OFFSET), frequency);
}

void soc_ctrl_select_spi_memio(const soc_ctrl_t *soc_ctrl) {
  mmio_region_write32(soc_ctrl->base_addr, (ptrdiff_t)(SOC_CTRL_ENABLE_SPI_SEL_REG_OFFSET), 0x1);
  mmio_region_write32(soc_ctrl->base_addr, (ptrdiff_t)(SOC_CTRL_USE_SPIMEMIO_REG_OFFSET), SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO);
}

void soc_ctrl_select_spi_host(const soc_ctrl_t *soc_ctrl) {
  mmio_region_write32(soc_ctrl->base_addr, (ptrdiff_t)(SOC_CTRL_ENABLE_SPI_SEL_REG_OFFSET), 0x1);
  mmio_region_write32(soc_ctrl->base_addr, (ptrdiff_t)(SOC_CTRL_USE_SPIMEMIO_REG_OFFSET), SOC_CTRL_SPI_FLASH_MODE_SPIHOST);
}

uint32_t get_spi_flash_mode(const soc_ctrl_t *soc_ctrl) {
  return mmio_region_read32(soc_ctrl->base_addr, (ptrdiff_t)(SOC_CTRL_USE_SPIMEMIO_REG_OFFSET));
}