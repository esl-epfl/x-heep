// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include "spi_host.h"

#include "mmio.h"
#include "bitfield.h"

// SPI get functions

volatile uint8_t spi_get_tx_queue_depth(const spi_host_t *spi) {
    volatile uint32_t status_reg = spi_get_status(spi);
    return bitfield_field32_read(status_reg, SPI_HOST_STATUS_TXQD_FIELD);
}

volatile spi_ch_status_t spi_get_tx_channel_status(const spi_host_t *spi) {
    volatile uint32_t status_reg = spi_get_status(spi);
    spi_ch_status_t ch_status = {
        .empty = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXEMPTY_BIT),
        .full = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXFULL_BIT),
        .stall = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXSTALL_BIT),
        .wm = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXWM_BIT)
    };
    return ch_status;
    // return (spi_ch_status_t){
    //     .empty = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXEMPTY_BIT),
    //     .full = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXFULL_BIT),
    //     .stall = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXSTALL_BIT),
    //     .wm = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_TXWM_BIT)
    // };
}

volatile uint8_t spi_get_rx_queue_depth(const spi_host_t *spi) {
    volatile uint32_t status_reg = spi_get_status(spi);
    return bitfield_field32_read(status_reg, SPI_HOST_STATUS_RXQD_FIELD);
}

volatile spi_ch_status_t spi_get_rx_channel_status(const spi_host_t *spi) {
    volatile uint32_t status_reg = spi_get_status(spi);
    spi_ch_status_t ch_status = {
        .empty = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXEMPTY_BIT),
        .full = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXFULL_BIT),
        .stall = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXSTALL_BIT),
        .wm = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXWM_BIT)
    };
    return ch_status;
    // return (spi_ch_status_t){
    //     .empty = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXEMPTY_BIT),
    //     .full = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXFULL_BIT),
    //     .stall = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXSTALL_BIT),
    //     .wm = bitfield_bit32_read(status_reg, SPI_HOST_STATUS_RXWM_BIT)
    // };
}

volatile uint32_t spi_get_csid(const spi_host_t *spi) {
    return mmio_region_read32(spi->base_addr, SPI_HOST_CSID_REG_OFFSET);
}

// SPI set functions

void spi_sw_reset(const spi_host_t *spi) {
    volatile uint32_t ctrl_reg = mmio_region_read32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET);
    ctrl_reg = bitfield_bit32_write(ctrl_reg, SPI_HOST_CONTROL_SW_RST_BIT, 1);
    mmio_region_write32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET, ctrl_reg);
}

void spi_set_enable(const spi_host_t *spi, bool enable) {
    volatile uint32_t ctrl_reg = mmio_region_read32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET);
    ctrl_reg = bitfield_bit32_write(ctrl_reg, SPI_HOST_CONTROL_SPIEN_BIT, enable);
    mmio_region_write32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET, ctrl_reg);
}

void spi_set_tx_watermark(const spi_host_t *spi, uint8_t watermark) {
    volatile uint32_t ctrl_reg = mmio_region_read32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET);
    ctrl_reg = bitfield_field32_write(ctrl_reg, SPI_HOST_CONTROL_TX_WATERMARK_FIELD, watermark);
    mmio_region_write32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET, ctrl_reg);
}

void spi_set_rx_watermark(const spi_host_t *spi, uint8_t watermark) {
    volatile uint32_t ctrl_reg = mmio_region_read32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET);
    ctrl_reg = bitfield_field32_write(ctrl_reg, SPI_HOST_CONTROL_RX_WATERMARK_FIELD, watermark);
    mmio_region_write32(spi->base_addr, SPI_HOST_CONTROL_REG_OFFSET, ctrl_reg);
}

void spi_set_configopts(const spi_host_t *spi, uint32_t csid, const uint32_t conf_reg) {
    mmio_region_write32(spi->base_addr, sizeof(uint32_t) * csid + SPI_HOST_CONFIGOPTS_0_REG_OFFSET, conf_reg);
}

void spi_set_csid(const spi_host_t* spi, uint32_t csid) {
    mmio_region_write32(spi->base_addr, SPI_HOST_CSID_REG_OFFSET, csid);
}

void spi_set_command(const spi_host_t *spi, const uint32_t cmd_reg) {
    mmio_region_write32(spi->base_addr, SPI_HOST_COMMAND_REG_OFFSET, cmd_reg);
}

void spi_write_word(const spi_host_t *spi, uint32_t wdata) {
    volatile uint32_t* fifo = spi->base_addr.base + SPI_HOST_DATA_REG_OFFSET;
    asm volatile ("sw %[w], 0(%[fifo]);" :: [w]"r"(wdata), [fifo]"r"(fifo));
}

void spi_read_word(const spi_host_t *spi, uint32_t* dst) {
    volatile uint32_t* fifo = spi->base_addr.base + SPI_HOST_DATA_REG_OFFSET;
    asm volatile (
        "lw    a7, 0 (%[fifo]);"
        "sw    a7, 0 (%[dst]);"
        :: [fifo]"r"(fifo), [dst]"r"(dst)
        : "a7"
    );
}

void spi_read_chunk_32B(const spi_host_t *spi, uint32_t* dst) {
    volatile uint32_t* fifo = spi->base_addr.base + SPI_HOST_DATA_REG_OFFSET;
    asm volatile (
        "lw    a7, 0 (%[fifo]);"
        "lw    t0, 0 (%[fifo]);"
        "lw    t1, 0 (%[fifo]);"
        "lw    t2, 0 (%[fifo]);"
        "lw    t3, 0 (%[fifo]);"
        "lw    t4, 0 (%[fifo]);"
        "lw    t5, 0 (%[fifo]);"
        "lw    t6, 0 (%[fifo]);"
        "sw    a7, 0 (%[dst]);"
        "sw    t0, 4 (%[dst]);"
        "sw    t1, 8 (%[dst]);"
        "sw    t2, 12(%[dst]);"
        "sw    t3, 16(%[dst]);"
        "sw    t4, 20(%[dst]);"
        "sw    t5, 24(%[dst]);"
        "sw    t6, 28(%[dst]);"
        :: [fifo]"r"(fifo), [dst]"r"(dst)
        : "a7", "t0", "t1", "t2", "t3", "t4", "t5", "t6"
    );
}
