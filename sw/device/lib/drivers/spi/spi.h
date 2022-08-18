// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Basic device functions for opentitan SPI host

#include <stdint.h>

#include "mmio.h"

#include "spi_regs.h"

/**
 * Initialization parameters for SPI.
 *
 */
typedef struct spi {
    /**
    * The base address for the SPI hardware registers.
    */
    mmio_region_t base_addr;
} spi_t;

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
    uint16_t    len         : 9;
    bool        csaat       : 1;
    spi_speed_e speed       : 2;
    spi_dir_e   direction   : 2;
} spi_command_t;

// SPI registers access functions

/**
 * Read the TX FIFO depth register.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @return TX FIFO depth.
 */
volatile uint8_t spi_get_tx_queue_depth(spi_t *spi);

/**
 * Read the TX channel status register.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @return TX channel status structure.
 */
volatile spi_ch_status_t spi_get_tx_channel_status(spi_t *spi);

/**
 * Read the RX FIFO depth register.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @return RX FIFO depth.
 */
volatile uint8_t spi_get_rx_queue_depth(spi_t *spi);

/**
 * Read the RX channel status register.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @return RX channel status structure.
 */
volatile spi_ch_status_t spi_get_rx_channel_status(spi_t *spi);

/**
 * Read the Chip Select (CS) ID register.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @return Chip Select (CS) ID.
 */
volatile uint32_t spi_get_csid(spi_t *spi);

/**
 * Reset the SPI from software.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 */
void spi_sw_reset(spi_t *spi);

/**
 * Enable the SPI host.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 */
void spi_set_enable(spi_t *spi, bool enable);

/**
 * Set the transmit queue watermark level (to enable interrupt triggering).
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @param watermark Queue/fifo trigger level (minimum level).
 */
void spi_set_tx_watermark(spi_t *spi, uint8_t watermark);

/**
 * Set the receive queue watermark level (to enable interrupt triggering).
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @param watermark Queue/fifo trigger level (maximum level).
 */
void spi_set_rx_watermark(spi_t *spi, uint8_t watermark);

/**
 * Set the requirement of a target device (i.e., a slave).
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @param csid Chip Select (CS) ID.
 * @param conf_reg Slave transmission configuration.
 */
void spi_set_configopts(spi_t *spi, uint32_t csid, uint32_t conf_reg);

/**
 * Select which device to target with the next command.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @param csid Chip Select (SC) ID.
 */
void spi_set_csid(spi_t *spi, uint32_t csid);

/**
 * Set the next command (one for all attached SPI devices).
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @param cmd_reg Command register value (Length, speed, ...).
 */
void spi_set_command(spi_t *spi, uint32_t cmd_reg);

/**
 * Write one word to the TX FIFO.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @param wdata Data to write.
 */
void spi_write_word(spi_t *spi, uint32_t wdata);

/**
 * Reads a chunk of data from RX FIFO (which must contains at least 128B!).
 *
 * @param spi Pointer to spi_t represting the target SPI.
 * @param dst Read data destination pointer.
 */
void spi_read_chunk_32B(spi_t *spi, uint32_t* dst);

// Inline functions

/**
 * Read SPI status register
 *
 * @param spi Pointer to spi_t represting the target SPI.
 */
static inline __attribute__((always_inline)) volatile uint32_t spi_get_status(spi_t *spi) {
    return mmio_region_read32(spi->base_addr, SPI_HOST_STATUS_REG_OFFSET);
}

/**
 * Read SPI active bit from status register
 *
 * @param spi Pointer to spi_t represting the target SPI.
 */
static inline __attribute__((always_inline)) volatile bool spi_get_active(spi_t *spi) {
    volatile uint32_t status_reg = spi_get_status(spi);
    return bitfield_bit32_read(status_reg, SPI_HOST_STATUS_ACTIVE_BIT);
}

/**
 * Read SPI ready bit from status register
 *
 * @param spi Pointer to spi_t represting the target SPI.
 */
static inline __attribute__((always_inline)) volatile bool spi_get_ready(spi_t *spi) {
    volatile uint32_t status_reg = spi_get_status(spi);
    return bitfield_bit32_read(status_reg, SPI_HOST_STATUS_READY_BIT);
}

// /**
//  * Get SPI handler
//  *
//  * @param spi Pointer to spi_t represting the target SPI.
//  */
// static inline __attribute__((always_inline)) __attribute__((const)) volatile spi_t spi_get_handle(const uintptr_t base_addr) {
//     spi_t spi = {
//         .base_addr = mmio_region_from_addr(base_addr),
//     };
//     return spi;
//     // return (spi_t){
//     //     .mmio = mmio_region_from_addr(spi_base),
//     // };
// }

/**
 * Wait SPI is ready to receive commands.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_ready(spi_t *spi) {
    while (!spi_get_ready(spi));
}

/**
 * Wait TX FIFO reach watermark.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_watermark(spi_t *spi) {
    while (!mmio_region_get_bit32(spi->base_addr, SPI_HOST_STATUS_REG_OFFSET, SPI_HOST_STATUS_TXWM_BIT));
}

/**
 * Wait RX FIFO reach watermark.
 *
 * @param spi Pointer to spi_t represting the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_rx_watermark(spi_t *spi) {
    while (!mmio_region_get_bit32(spi->base_addr, SPI_HOST_STATUS_REG_OFFSET, SPI_HOST_STATUS_RXWM_BIT));
}

/**
 * Create SPI target device configuration word.
 *
 * @param configopts Target device configuation structure.
 */
static inline __attribute__((always_inline)) __attribute__((const)) uint32_t spi_create_configopts(const spi_configopts_t configopts) {
    uint32_t co_reg = 0;
    co_reg = bitfield_field32_write(co_reg, SPI_HOST_CONFIGOPTS_0_CLKDIV_0_FIELD, configopts.clkdiv);
    co_reg = bitfield_field32_write(co_reg, SPI_HOST_CONFIGOPTS_0_CSNIDLE_0_FIELD, configopts.csnidle);
    co_reg = bitfield_field32_write(co_reg, SPI_HOST_CONFIGOPTS_0_CSNTRAIL_0_FIELD, configopts.csntrail);
    co_reg = bitfield_field32_write(co_reg, SPI_HOST_CONFIGOPTS_0_CSNLEAD_0_FIELD, configopts.csnlead);
    co_reg = bitfield_bit32_write(co_reg, SPI_HOST_CONFIGOPTS_0_FULLCYC_0_BIT, configopts.fullcyc);
    co_reg = bitfield_bit32_write(co_reg, SPI_HOST_CONFIGOPTS_0_CPHA_0_BIT, configopts.cpha);
    co_reg = bitfield_bit32_write(co_reg, SPI_HOST_CONFIGOPTS_0_CPOL_0_BIT, configopts.cpol);
    return co_reg;
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
