// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "spi_host_old.h"

#include "mmio.h"
#include "bitfield.h"

// SPI get functions

volatile uint8_t spi_get_tx_queue_depth_old(const spi_host_t_old *spi) {
    volatile uint32_t status_reg = spi_get_status_old(spi);
    return bitfield_field32_read(status_reg, SPI_HOST_STATUS_TXQD_FIELD);
}

volatile spi_ch_status_t_old spi_get_tx_channel_status_old(const spi_host_t_old *spi) {
    volatile uint32_t status_reg = spi_get_status_old(spi);
    spi_ch_status_t_old ch_status = {
        .empty = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXEMPTY_BIT),
        .full = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXFULL_BIT),
        .stall = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXSTALL_BIT),
        .wm = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXWM_BIT)
    };
    return ch_status;
}

volatile uint8_t spi_get_rx_queue_depth_old(const spi_host_t_old *spi) {
    volatile uint32_t status_reg = spi_get_status_old(spi);
    return bitfield_field32_read(status_reg, SPI_HOST_STATUS_RXQD_FIELD);
}

volatile spi_ch_status_t_old spi_get_rx_channel_status_old(const spi_host_t_old *spi) {
    volatile uint32_t status_reg = spi_get_status_old(spi);
    spi_ch_status_t_old ch_status = {
        .empty = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXEMPTY_BIT),
        .full = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXFULL_BIT),
        .stall = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXSTALL_BIT),
        .wm = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXWM_BIT)
    };
    return ch_status;
}

volatile uint32_t spi_get_csid_old(const spi_host_t_old *spi) {
    return mmio_region_read32(spi->base_addr, SPI_HOST_CSID_REG_OFFSET);
}

// SPI set functions

void spi_sw_reset_old(const spi_host_t_old *spi) {
    volatile uint32_t ctrl_reg = mmio_region_read32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET);
    ctrl_reg = bitfield_bit32_write(ctrl_reg, SPI_HOST_CONTROL_SW_RST_BIT, 1);
    mmio_region_write32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET, ctrl_reg);
}

void spi_set_enable_old(const spi_host_t_old *spi, bool enable) {
    volatile uint32_t ctrl_reg = mmio_region_read32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET);
    ctrl_reg = bitfield_bit32_write(ctrl_reg, SPI_HOST_CONTROL_SPIEN_BIT, enable);
    mmio_region_write32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET, ctrl_reg);
}

void spi_set_tx_watermark_old(const spi_host_t_old *spi, uint8_t watermark) {
    volatile uint32_t ctrl_reg = mmio_region_read32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET);
    ctrl_reg = bitfield_field32_write(ctrl_reg, SPI_HOST_CONTROL_TX_WATERMARK_FIELD, watermark);
    mmio_region_write32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET, ctrl_reg);
}

void spi_set_rx_watermark_old(const spi_host_t_old *spi, uint8_t watermark) {
    volatile uint32_t ctrl_reg = mmio_region_read32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET);
    ctrl_reg = bitfield_field32_write(ctrl_reg, SPI_HOST_CONTROL_RX_WATERMARK_FIELD, watermark);
    mmio_region_write32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET, ctrl_reg);
}

void spi_set_configopts_old(const spi_host_t_old *spi, uint32_t csid, const uint32_t conf_reg) {
    mmio_region_write32(spi->base_addr, sizeof(uint32_t) * csid + SPI_HOST_CONFIGOPTS_0_REG_OFFSET, conf_reg);
}

void spi_set_csid_old(const spi_host_t_old* spi, uint32_t csid) {
    mmio_region_write32(spi->base_addr, SPI_HOST_CSID_REG_OFFSET, csid);
}

void spi_set_command_old(const spi_host_t_old *spi, const uint32_t cmd_reg) {
    mmio_region_write32(spi->base_addr, SPI_HOST_COMMAND_REG_OFFSET, cmd_reg);
}

void spi_write_word_old(const spi_host_t_old *spi, uint32_t wdata) {
    mmio_region_write32(spi->base_addr, SPI_HOST_TXDATA_REG_OFFSET, wdata);
}

void spi_read_word_old(const spi_host_t_old *spi, uint32_t* dst) {
    *dst = mmio_region_read32(spi->base_addr, SPI_HOST_RXDATA_REG_OFFSET);
}

void spi_enable_evt_intr_old(const spi_host_t_old *spi, bool enable) {
    volatile uint32_t intr_enable_reg = mmio_region_read32(spi->base_addr, SPI_HOST_INTR_ENABLE_REG_OFFSET);
    intr_enable_reg = bitfield_bit32_write(intr_enable_reg, SPI_HOST_INTR_ENABLE_SPI_EVENT_BIT, enable);
    mmio_region_write32(spi->base_addr, SPI_HOST_INTR_ENABLE_REG_OFFSET, intr_enable_reg);
}

void spi_enable_error_intr_old(const spi_host_t_old *spi, bool enable) {
    volatile uint32_t intr_enable_reg = mmio_region_read32(spi->base_addr, SPI_HOST_INTR_ENABLE_REG_OFFSET);
    intr_enable_reg = bitfield_bit32_write(intr_enable_reg, SPI_HOST_INTR_STATE_ERROR_BIT, enable);
    mmio_region_write32(spi->base_addr, SPI_HOST_INTR_ENABLE_REG_OFFSET, intr_enable_reg);
}

void spi_enable_rxwm_intr_old(const spi_host_t_old *spi, bool enable) {
    volatile uint32_t intr_enable_reg = mmio_region_read32(spi->base_addr, SPI_HOST_EVENT_ENABLE_REG_OFFSET);
    intr_enable_reg = bitfield_bit32_write(intr_enable_reg, SPI_HOST_EVENT_ENABLE_RXWM_BIT, enable);
    mmio_region_write32(spi->base_addr, SPI_HOST_EVENT_ENABLE_REG_OFFSET, intr_enable_reg);
}

void spi_enable_txempty_intr_old(const spi_host_t_old *spi, bool enable) {
    volatile uint32_t intr_enable_reg = mmio_region_read32(spi->base_addr, SPI_HOST_EVENT_ENABLE_REG_OFFSET);
    intr_enable_reg = bitfield_bit32_write(intr_enable_reg, SPI_HOST_EVENT_ENABLE_TXEMPTY_BIT, enable);
    mmio_region_write32(spi->base_addr, SPI_HOST_EVENT_ENABLE_REG_OFFSET, intr_enable_reg);
}

void spi_output_enable_old(const spi_host_t_old *spi, bool enable){
    volatile uint32_t output_enable_reg = mmio_region_read32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET);
    output_enable_reg = bitfield_bit32_write(output_enable_reg, SPI_HOST_CONTROL_OUTPUT_EN_BIT, enable);
    mmio_region_write32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET, output_enable_reg);
}

__attribute__((weak, optimize("O0"))) void handler_irq_spi_old(uint32_t id)
{
 // Replace this function with a non-weak implementation
}