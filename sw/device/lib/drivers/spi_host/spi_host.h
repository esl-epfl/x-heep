// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Basic device functions for opentitan SPI host

#ifndef _DRIVERS_SPI_HOST_H_
#define _DRIVERS_SPI_HOST_H_

#include <stdint.h>

#include "mmio.h"
#include "spi_host_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialization parameters for SPI.
 *
 */
typedef struct spi {
    /**
    * The base address for the SPI hardware registers.
    */
    mmio_region_t base_addr;
} spi_host_t;

/**
* SPI channel status structure
*/
typedef struct spi_ch_status {
    bool empty : 1;
    bool full  : 1;
    bool wm    : 1;
    bool stall : 1;
} spi_ch_status_t;

/**
* SPI speed type
*/
typedef enum {
    kSpiSpeedStandard   = 0,
    kSpiSpeedDual       = 1,
    kSpiSpeedQuad       = 2
} spi_speed_e;

/**
* SPI directionality
*/
typedef enum {
    kSpiDirDummy        = 0,
    kSpiDirRxOnly       = 1,
    kSpiDirTxOnly       = 2,
    kSpiDirBidir        = 3
} spi_dir_e;

/**
* SPI chip (slave) configuration structure
*/
typedef struct spi_configopts {
    uint16_t clkdiv     : 16;
    uint8_t  csnidle    : 4;
    uint8_t  csntrail   : 4;
    uint8_t  csnlead    : 4;
    bool     __rsvd0    : 1;
    bool     fullcyc    : 1;
    bool     cpha       : 1;
    bool     cpol       : 1;
} spi_configopts_t;

/**
* SPI command structure
*/
typedef struct spi_command {
    uint32_t    len         : 24;
    bool        csaat       : 1;
    spi_speed_e speed       : 2;
    spi_dir_e   direction   : 2;
} spi_command_t;

// SPI registers access functions

/**
 * Read the TX FIFO depth register.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return TX FIFO depth.
 */
volatile uint8_t spi_get_tx_queue_depth(const spi_host_t *spi);

/**
 * Read the TX channel status register.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return TX channel status structure.
 */
volatile spi_ch_status_t spi_get_tx_channel_status(const spi_host_t *spi);

/**
 * Read the RX FIFO depth register.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return RX FIFO depth.
 */
volatile uint8_t spi_get_rx_queue_depth(const spi_host_t *spi);

/**
 * Read the RX channel status register.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return RX channel status structure.
 */
volatile spi_ch_status_t spi_get_rx_channel_status(const spi_host_t *spi);

/**
 * Read the Chip Select (CS) ID register.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return Chip Select (CS) ID.
 */
volatile uint32_t spi_get_csid(const spi_host_t *spi);

/**
 * Reset the SPI from software.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
void spi_sw_reset(const spi_host_t *spi);

/**
 * Enable the SPI host.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param enable SPI enable register value.
 */
void spi_set_enable(const spi_host_t *spi, bool enable);

/**
 * Set the transmit queue watermark level (to enable interrupt triggering).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param watermark Queue/fifo trigger level (minimum level).
 */
void spi_set_tx_watermark(const spi_host_t *spi, uint8_t watermark);

/**
 * Set the receive queue watermark level (to enable interrupt triggering).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param watermark Queue/fifo trigger level (maximum level).
 */
void spi_set_rx_watermark(const spi_host_t *spi, uint8_t watermark);

/**
 * Set the requirement of a target device (i.e., a slave).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param csid Chip Select (CS) ID.
 * @param conf_reg Slave transmission configuration.
 */
void spi_set_configopts(const spi_host_t *spi, uint32_t csid, uint32_t conf_reg);

/**
 * Select which device to target with the next command.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param csid Chip Select (SC) ID.
 */
void spi_set_csid(const spi_host_t *spi, uint32_t csid);

/**
 * Set the next command (one for all attached SPI devices).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param cmd_reg Command register value (Length, speed, ...).
 */
void spi_set_command(const spi_host_t *spi, uint32_t cmd_reg);

/**
 * Write one word to the TX FIFO.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param wdata Data to write.
 */
void spi_write_word(const spi_host_t *spi, uint32_t wdata);

/**
 * Read one word to the RX FIFO.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param rdata Read data.
 */
void spi_read_word(const spi_host_t *spi, uint32_t* dst);

/**
 * Enable SPI event interrupt
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param enable SPI event interrupt bit value.
 */
void spi_enable_evt_intr(const spi_host_t *spi, bool enable);

/**
 * Enable SPI error interrupt
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param enable SPI error interrupt bit value.
 */
void spi_enable_error_intr(const spi_host_t *spi, bool enable);

/**
 * Enable SPI watermark event interrupt
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param enable SPI RX watermark interrupt bit value.
 */
void spi_enable_rxwm_intr(const spi_host_t *spi, bool enable);

/**
 * Enable SPI TX empty event interrupt
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param enable SPI TX empty interrupt bit value.
 */
void spi_enable_txempty_intr(const spi_host_t *spi, bool enable);

/**
 * Enable SPI output
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param enable SPI TX empty interrupt bit value.
 */
void spi_output_enable(const spi_host_t *spi, bool enable);


// Inline functions

/**
 * Read SPI status register
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) volatile uint32_t spi_get_status(const spi_host_t *spi) {
    return mmio_region_read32(spi->base_addr, SPI_HOST_STATUS_REG_OFFSET);
}

/**
 * Read SPI active bit from status register
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) volatile bool spi_get_active(const spi_host_t *spi) {
    volatile uint32_t status_reg = spi_get_status(spi);
    return bitfield_bit32_read(status_reg, SPI_HOST_STATUS_ACTIVE_BIT);
}

/**
 * Read SPI ready bit from status register
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) volatile bool spi_get_ready(const spi_host_t *spi) {
    volatile uint32_t status_reg = spi_get_status(spi);
    return bitfield_bit32_read(status_reg, SPI_HOST_STATUS_READY_BIT);
}

/**
 * Wait SPI is ready to receive commands.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_ready(const spi_host_t *spi) {
    while (!spi_get_ready(spi));
}

/**
 * Wait TX FIFO reach watermark.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_watermark(const spi_host_t *spi) {
    while (!mmio_region_get_bit32(spi->base_addr, SPI_HOST_STATUS_REG_OFFSET, SPI_HOST_STATUS_TXWM_BIT));
}

/**
 * Wait TX FIFO empty.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_empty(const spi_host_t *spi) {
    while (!mmio_region_get_bit32(spi->base_addr, SPI_HOST_STATUS_REG_OFFSET, SPI_HOST_STATUS_TXEMPTY_BIT));
}

/**
 * Wait TX FIFO not empty.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_not_empty(const spi_host_t *spi) {
    while (mmio_region_get_bit32(spi->base_addr, SPI_HOST_STATUS_REG_OFFSET, SPI_HOST_STATUS_TXEMPTY_BIT));
}

/**
 * Wait TX FIFO not full.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_not_full(const spi_host_t *spi) {
    while (mmio_region_get_bit32(spi->base_addr, SPI_HOST_STATUS_REG_OFFSET, SPI_HOST_STATUS_TXFULL_BIT));
}

/**
 * Wait RX FIFO reach watermark.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_rx_watermark(const spi_host_t *spi) {
    while (!mmio_region_get_bit32(spi->base_addr, SPI_HOST_STATUS_REG_OFFSET, SPI_HOST_STATUS_RXWM_BIT));
}

/**
 * Create SPI target device configuration word.
 *
 * @param configopts Target device configuation structure.
 */
static inline __attribute__((always_inline)) __attribute__((const)) uint32_t spi_create_configopts(const spi_configopts_t configopts) {
    uint32_t conf_reg = 0;
    conf_reg = bitfield_field32_write(conf_reg, SPI_HOST_CONFIGOPTS_0_CLKDIV_0_FIELD, configopts.clkdiv);
    conf_reg = bitfield_field32_write(conf_reg, SPI_HOST_CONFIGOPTS_0_CSNIDLE_0_FIELD, configopts.csnidle);
    conf_reg = bitfield_field32_write(conf_reg, SPI_HOST_CONFIGOPTS_0_CSNTRAIL_0_FIELD, configopts.csntrail);
    conf_reg = bitfield_field32_write(conf_reg, SPI_HOST_CONFIGOPTS_0_CSNLEAD_0_FIELD, configopts.csnlead);
    conf_reg = bitfield_bit32_write(conf_reg, SPI_HOST_CONFIGOPTS_0_FULLCYC_0_BIT, configopts.fullcyc);
    conf_reg = bitfield_bit32_write(conf_reg, SPI_HOST_CONFIGOPTS_0_CPHA_0_BIT, configopts.cpha);
    conf_reg = bitfield_bit32_write(conf_reg, SPI_HOST_CONFIGOPTS_0_CPOL_0_BIT, configopts.cpol);
    return conf_reg;
}

/**
 * Create SPI command word.
 *
 * @param command Command configuration structure.
 */
static inline __attribute__((always_inline)) __attribute__((const)) uint32_t spi_create_command(const spi_command_t command) {
    uint32_t cmd_reg = 0;
    cmd_reg = bitfield_field32_write(cmd_reg, SPI_HOST_COMMAND_LEN_FIELD, command.len);
    cmd_reg = bitfield_bit32_write(cmd_reg, SPI_HOST_COMMAND_CSAAT_BIT, command.csaat);
    cmd_reg = bitfield_field32_write(cmd_reg, SPI_HOST_COMMAND_SPEED_FIELD, command.speed);
    cmd_reg = bitfield_field32_write(cmd_reg, SPI_HOST_COMMAND_DIRECTION_FIELD, command.direction);
    return cmd_reg;
}

/**
 * @brief Attends the plic interrupt.
 */
__attribute__((weak, optimize("O0"))) void handler_irq_spi(uint32_t id);


#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_SPI_HOST_H_
