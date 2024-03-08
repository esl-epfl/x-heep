/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************
**
** project  : X-HEEP
** filename : spi_host.h
** version  : 1
** date     : 06/03/24
**
***************************************************************************
**
** Copyright (c) EPFL contributors.
** All rights reserved.
**
***************************************************************************
*/

/***************************************************************************/
/***************************************************************************/
/**
* @file   spi_host.h
* @date   06/03/24
* @brief  The Serial Peripheral Interface (SPI) driver to set up and use the
* SPI peripheral
*/

#ifndef _DRIVERS_SPI_HOST_H_
#define _DRIVERS_SPI_HOST_H_

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include <stdint.h>

#include "mmio.h"

#include "spi_host_regs.h"       // Generated
#include "spi_host_structs.h"    // Generated

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

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
* SPI Watermark Set Return Flags
*/
typedef enum {
    SPI_WATERMARK_OK        = 0x00,    /*!< The Watermark was correctly set */
    SPI_WATERMARK_EXCEEDS   = 0x01     /*!< The Watermark exceeded SPI_HOST_PARAM_TX_DEPTH 
    or SPI_HOST_PARAM_RX_DEPTH and was therefore not set */
} spi_watermark_flags_e;

/**
* SPI CSID Set Return Flags
*/
typedef enum {
    SPI_CSID_OK         = 0x00,    /*!< The CSID was correctly set */
    SPI_CSID_INVALID    = 0x01     /*!< The CSID was out of the bounds specified in 
    SPI_HOST_PARAM_NUM_C_S */
} spi_csid_flags_e;

/**
* SPI Configopts Set Return Flags
*/
typedef enum {
    SPI_CONFIGOPTS_OK               = 0x00, /*!< The configopts was correctly set for the provided CSID */
    SPI_CONFIGOPTS_CSID_INVALID     = 0x01  /*!< The CSID was out of the bounds 
    specified in SPI_HOST_PARAM_NUM_C_S. @Note that this is equal to SPI_CSID_INVALID */
} spi_configopts_flags_e;

/**
* SPI Command Set Return Flags
*/
typedef enum {
    SPI_COMMAND_OK              = 0x00, /*!< The command was correctly written in the register */
    SPI_COMMAND_QUEUE_FULL      = 0x01, /*!< The CMD FIFO is currently full so couldn't write command */
    SPI_COMMAND_SPEED_INVALID   = 0x02  /*!< The specified speed is not valid (i.e. = 3) so couldn't write command */
} spi_command_flags_e;

/**
* SPI Read/Write Return Flags
*/
typedef enum {
    SPI_READ_WRITE_OK           = 0x00, /*!< Word correctly read / written */
    SPI_READ_WRITE_QUEUE_FULL   = 0x01, /*!< The TX Queue is full, thus could not write to TX register */
    SPI_READ_WRITE_QUEUE_EMPTY  = 0x02  /*!< The RX Queue is empty, thus could not read from RX register */
} spi_read_write_flags_e;

/**
 * Initialization parameters for SPI.
 */
typedef struct spi {
    /**
    * The base address for the SPI hardware registers.
    */
    mmio_region_t base_addr;
} spi_host_t;

/**
* SPI channel (TX/RX) status structure
*/
typedef struct spi_ch_status {
    bool empty : 1; // channel FIFO is empty
    bool full  : 1; // channel FIFO is full
    bool wm    : 1; // amount of words in channel FIFO exceeds watermark (if 
                    // RX) or is currently less than watermark (if TX)
    bool stall : 1; // RX FIFO is full and SPI is waiting for software to remove
                    // data or TX FIFO is empty and SPI is waiting for data
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
    uint32_t    len         : 24;
    bool        csaat       : 1;
    spi_speed_e speed       : 2;
    spi_dir_e   direction   : 2;
} spi_command_t;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

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
 * @return Flag indicating problems. Returns SPI_WATERMARK_OK == 0 if everithing
 * went well.
 */
spi_watermark_flags_e spi_set_tx_watermark(const spi_host_t *spi, uint8_t watermark);

/**
 * Set the receive queue watermark level (to enable interrupt triggering).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param watermark Queue/fifo trigger level (maximum level).
 * @return Flag indicating problems. Returns SPI_WATERMARK_OK == 0 if everithing
 * went well.
 */
spi_watermark_flags_e spi_set_rx_watermark(const spi_host_t *spi, uint8_t watermark);

/**
 * Set the requirement of a target device (i.e., a slave).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param csid Chip Select (CS) ID.
 * @param conf_reg Slave transmission configuration.
 * @return Flag indicating problems. Returns SPI_CONFIGOPTS_OK == 0 if everithing
 * went well.
 */
spi_configopts_flags_e spi_set_configopts(const spi_host_t *spi, uint32_t csid, uint32_t conf_reg);

/**
 * Select which device to target with the next command.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param csid Chip Select (SC) ID.
 * @return Flag indicating problems. Returns SPI_CSID_OK == 0 if everithing went
 * well.
 */
spi_csid_flags_e spi_set_csid(const spi_host_t *spi, uint32_t csid);

/**
 * Set the next command (one for all attached SPI devices).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param cmd_reg Command register value (Length, speed, ...).
 * @return Flag indicating problems. Returns SPI_COMMAND_OK == 0 if everithing
 * went well.
 */
spi_command_flags_e spi_set_command(const spi_host_t *spi, uint32_t cmd_reg);

/**
 * Write one word to the TX FIFO.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param wdata Data to write.
 * @return Flag indicating problems. Returns SPI_READ_WRITE_OK == 0 if everithing
 * went well.
 */
spi_read_write_flags_e spi_write_word(const spi_host_t *spi, uint32_t wdata);

/**
 * Read one word to the RX FIFO.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param rdata Read data.
 * @return Flag indicating problems. Returns SPI_READ_WRITE_OK == 0 if everithing
 * went well.
 */
spi_read_write_flags_e spi_read_word(const spi_host_t *spi, uint32_t* dst);

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

/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/

/**
 * Read SPI status register
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) volatile uint32_t spi_get_status(const spi_host_t *spi) {
    return spi_host_peri->STATUS;
}

/**
 * Read SPI active bit from status register
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) volatile bool spi_get_active(const spi_host_t *spi) {
    volatile uint32_t status_reg = spi_get_status(spi);
    return bitfield_read(status_reg, BIT_MASK_1, SPI_HOST_STATUS_ACTIVE_BIT);
}

/**
 * Read SPI ready bit from status register
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) volatile bool spi_get_ready(const spi_host_t *spi) {
    volatile uint32_t status_reg = spi_get_status(spi);
    return bitfield_read(status_reg, BIT_MASK_1, SPI_HOST_STATUS_READY_BIT);
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
    while (!bitfield_read(spi_host_peri->STATUS, BIT_MASK_1, SPI_HOST_STATUS_TXWM_BIT));
}

/**
 * Wait TX FIFO empty.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_empty(const spi_host_t *spi) {
    while (!bitfield_read(spi_host_peri->STATUS, BIT_MASK_1, SPI_HOST_STATUS_TXEMPTY_BIT));
}

/**
 * Wait TX FIFO not empty.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_not_empty(const spi_host_t *spi) {
    while (!bitfield_read(spi_host_peri->STATUS, BIT_MASK_1, SPI_HOST_STATUS_TXEMPTY_BIT));
}

/**
 * Wait TX FIFO not full.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_not_full(const spi_host_t *spi) {
    while (!bitfield_read(spi_host_peri->STATUS, BIT_MASK_1, SPI_HOST_STATUS_TXFULL_BIT));
}

/**
 * Wait RX FIFO reach watermark.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_rx_watermark(const spi_host_t *spi) {
    while (!bitfield_read(spi_host_peri->STATUS, BIT_MASK_1, SPI_HOST_STATUS_RXWM_BIT));
}

/**
 * Create SPI target device configuration word.
 *
 * @param configopts Target device configuation structure.
 */
static inline __attribute__((always_inline)) __attribute__((const)) uint32_t spi_create_configopts(const spi_configopts_t configopts) {
    uint32_t conf_reg = 0;
    conf_reg = bitfield_write(conf_reg, SPI_HOST_CONFIGOPTS_0_CLKDIV_0_MASK, SPI_HOST_CONFIGOPTS_0_CLKDIV_0_OFFSET, configopts.clkdiv);
    conf_reg = bitfield_write(conf_reg, SPI_HOST_CONFIGOPTS_0_CSNIDLE_0_MASK, SPI_HOST_CONFIGOPTS_0_CSNIDLE_0_OFFSET, configopts.csnidle);
    conf_reg = bitfield_write(conf_reg, SPI_HOST_CONFIGOPTS_0_CSNTRAIL_0_MASK, SPI_HOST_CONFIGOPTS_0_CSNTRAIL_0_OFFSET, configopts.csntrail);
    conf_reg = bitfield_write(conf_reg, SPI_HOST_CONFIGOPTS_0_CSNLEAD_0_MASK, SPI_HOST_CONFIGOPTS_0_CSNLEAD_0_OFFSET, configopts.csnlead);
    conf_reg = bitfield_write(conf_reg, BIT_MASK_1, SPI_HOST_CONFIGOPTS_0_FULLCYC_0_BIT, configopts.fullcyc);
    conf_reg = bitfield_write(conf_reg, BIT_MASK_1, SPI_HOST_CONFIGOPTS_0_CPHA_0_BIT, configopts.cpha);
    conf_reg = bitfield_write(conf_reg, BIT_MASK_1, SPI_HOST_CONFIGOPTS_0_CPOL_0_BIT, configopts.cpol);
    return conf_reg;
}

/**
 * Create SPI command word.
 *
 * @param command Command configuration structure.
 */
static inline __attribute__((always_inline)) __attribute__((const)) uint32_t spi_create_command(const spi_command_t command) {
    uint32_t cmd_reg = 0;
    cmd_reg = bitfield_write(cmd_reg, SPI_HOST_COMMAND_LEN_MASK, SPI_HOST_COMMAND_LEN_OFFSET, command.len);
    cmd_reg = bitfield_write(cmd_reg, BIT_MASK_1, SPI_HOST_COMMAND_CSAAT_BIT, command.csaat);
    cmd_reg = bitfield_write(cmd_reg, SPI_HOST_COMMAND_SPEED_MASK, SPI_HOST_COMMAND_SPEED_OFFSET, command.speed);
    cmd_reg = bitfield_write(cmd_reg, SPI_HOST_COMMAND_DIRECTION_MASK, SPI_HOST_COMMAND_DIRECTION_OFFSET, command.direction);
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

/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
