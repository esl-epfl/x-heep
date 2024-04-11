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

#define SPI_MAX_IDX 3
#define SPI_IDX_INVALID(idx)    idx > SPI_MAX_IDX
#define SPI_CSID_INVALID(csid)  csid >= SPI_HOST_PARAM_NUM_C_S

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/**
* SPI Peripheral IDX
*/
typedef enum {
    SPI_IDX_FLASH       = 0,
    SPI_IDX_MMIO        = 1,
    SPI_IDX_HOST        = 2,
    SPI_IDX_HOST_2      = 3
} spi_idx_e;

/**
* SPI speed type
*/
typedef enum {
    SPI_SPEED_STANDARD   = 0,
    SPI_SPEED_DUAL       = 1,
    SPI_SPEED_QUAD       = 2
} spi_speed_e;

/**
* SPI directionality
*/
typedef enum {
    SPI_DIR_DUMMY       = 0,
    SPI_DIR_RX_ONLY     = 1,
    SPI_DIR_TX_ONLY     = 2,
    SPI_DIR_BIDIR       = 3
} spi_dir_e;

/**
* SPI events
*/
typedef enum {
    SPI_EVENT_NONE      = 0,
    SPI_EVENT_RXFULL    = (1 << SPI_HOST_EVENT_ENABLE_RXFULL_BIT),
    SPI_EVENT_TXEMPTY   = (1 << SPI_HOST_EVENT_ENABLE_TXEMPTY_BIT),
    SPI_EVENT_RXWM      = (1 << SPI_HOST_EVENT_ENABLE_RXWM_BIT),
    SPI_EVENT_TXWM      = (1 << SPI_HOST_EVENT_ENABLE_TXWM_BIT),
    SPI_EVENT_READY     = (1 << SPI_HOST_EVENT_ENABLE_READY_BIT),
    SPI_EVENT_IDLE      = (1 << SPI_HOST_EVENT_ENABLE_IDLE_BIT),
    SPI_EVENT_ALL       = (1 << SPI_HOST_EVENT_ENABLE_IDLE_BIT+1) - 1
} spi_event_e;

/**
* SPI errors
*/
typedef enum {
    SPI_ERROR_NONE          = 0,
    SPI_ERROR_CMDBUSY       = (1 << SPI_HOST_ERROR_ENABLE_CMDBUSY_BIT),
    SPI_ERROR_OVERFLOW      = (1 << SPI_HOST_ERROR_ENABLE_OVERFLOW_BIT),
    SPI_ERROR_UNDERFLOW     = (1 << SPI_HOST_ERROR_ENABLE_UNDERFLOW_BIT),
    SPI_ERROR_CMDINVAL      = (1 << SPI_HOST_ERROR_ENABLE_CMDINVAL_BIT),
    SPI_ERROR_CSIDINVAL     = (1 << SPI_HOST_ERROR_ENABLE_CSIDINVAL_BIT),
    SPI_ERROR_ACCESSINVAL   = (1 << SPI_HOST_ERROR_STATUS_ACCESSINVAL_BIT),
    SPI_ERROR_IRQALL        = (1 << SPI_HOST_ERROR_ENABLE_CSIDINVAL_BIT+1) - 1,
    SPI_ERROR_ALL           = (1 << SPI_HOST_ERROR_STATUS_ACCESSINVAL_BIT+1) - 1
} spi_error_e;

/**
* SPI functions return flags, informs user what problem there was or if all OK
*/
typedef enum {
    SPI_FLAG_OK                 = 0x0000,
    SPI_FLAG_NULL_PTR           = 0x0001,
    SPI_FLAG_WATERMARK_EXCEEDS  = 0x0002, /*!< The Watermark exceeded SPI_HOST_PARAM_TX_DEPTH 
    or SPI_HOST_PARAM_RX_DEPTH and was therefore not set */
    SPI_FLAG_CSID_INVALID       = 0x0004, /*!< The CSID was out of the bounds specified in 
    SPI_HOST_PARAM_NUM_C_S */
    SPI_FLAG_COMMAND_FULL       = 0x0008, /*!< The CMD FIFO is currently full so couldn't write command */
    SPI_FLAG_SPEED_INVALID      = 0x0010, /*!< The specified speed is not valid (i.e. = 3) so couldn't write command */
    SPI_FLAG_TX_QUEUE_FULL      = 0x0020, /*!< The TX Queue is full, thus could not write to TX register */
    SPI_FLAG_RX_QUEUE_EMPTY     = 0x0040, /*!< The RX Queue is empty, thus could not read from RX register */
    SPI_FLAG_NOT_READY          = 0x0080, /*!< The SPI is not ready */
    SPI_FLAG_EVENT_INVALID      = 0x0100, /*!< The event to enable is not a valid event */
    SPI_FLAG_ERROR_INVALID      = 0x0200  /*!< The error irq to enable is not a valid error irq */
} spi_return_flags_e;

/**
 * Initialization parameters for SPI.
 */
typedef struct spi {
    /**
    * The base address for the SPI hardware registers.
    */
    spi_host* peri;
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

/**
* SPI status structure
* @TODO: Check if the fields should be volatile if this approach is implemented
*/
// typedef struct spi_status {
//     uint8_t txqd        : 8;
//     uint8_t rxqd        : 8;
//     uint8_t cmdqd       : 4;
//     bool    rxwm        : 1;
//     bool    __rsvd0     : 1;
//     bool    byteorder   : 1;
//     bool    rxstall     : 1;
//     bool    rxempty     : 1;
//     bool    rxfull      : 1;
//     bool    txwm        : 1;
//     bool    txstall     : 1;
//     bool    txempty     : 1;
//     bool    txfull      : 1;
//     bool    active      : 1;
//     bool    ready       : 1;
// } spi_status_t;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

extern volatile spi_host* const spi_peris[4];

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/


/**
 * Initialize SPI for flash device.
 *
 * @return The SPI peripheral identifier index for flash.
 */
spi_host_t spi_init_flash();

/**
 * Initialize SPI for host device.
 *
 * @return The SPI peripheral identifier index for host.
 */
spi_host_t spi_init_host();

/**
 * Initialize SPI for secondary host device.
 *
 * @return The SPI peripheral identifier index for host 2.
 */
spi_host_t spi_init_host2();

/**
 * Get enabled events for a specified SPI peripheral.
 *
 * @param peri_id The SPI peripheral identifier index.
 * @param events Pointer to store enabled event interrupts.
 * @return Flag indicating problems. Returns SPI_FLAG_OK if everything went well.
 */
spi_return_flags_e spi_get_events_enabled(const spi_idx_e peri_id, spi_event_e* events);

/**
 * Set enabled events for a specified SPI peripheral.
 *
 * @param peri_id The SPI peripheral identifier index.
 * @param events Pointer to events to enable/disable and to store the currently enabled
 *               event interrupts.
 * @param enable Flag to enable (true) or disable (false) the specified event interrupts.
 * @return Flag indicating problems. Returns SPI_FLAG_OK if everything went well.
 */
spi_return_flags_e spi_set_events_enabled(const spi_idx_e peri_id, spi_event_e* events, bool enable);

/**
 * Get enabled error interrupts for a specified SPI peripheral.
 *
 * @param peri_id The SPI peripheral identifier index.
 * @param errors Pointer to store enabled error interrupts.
 * @return Flag indicating problems. Returns SPI_FLAG_OK if everything went well.
 */
spi_return_flags_e spi_get_errors_enabled(const spi_idx_e peri_id, spi_error_e* errors);

/**
 * Set enabled error interrupts for a specified SPI peripheral.
 *
 * @param peri_id The SPI peripheral identifier index.
 * @param errors Pointer to error interrupts to enable/disable and to store the currently
 *               enabled error interrupts.
 * @param enable Flag to enable (true) or disable (false) the specified error interrupts.
 * @return Flag indicating problems. Returns SPI_FLAG_OK if everything went well.
 */
spi_return_flags_e spi_set_errors_enabled(const spi_idx_e peri_id, spi_error_e* errors, bool enable);

/**
 * Write a byte of data to the transmit queue of a specified SPI peripheral.
 *
 * @param peri_id The SPI peripheral identifier index.
 * @param bdata Byte of data to write.
 * @return Flag indicating problems. Returns SPI_FLAG_OK if everything went well.
 */
spi_return_flags_e spi_write_byte(const spi_idx_e peri_id, uint8_t bdata);

/**
 * Enable or disable error interrupt test mode for a specified SPI peripheral.
 *
 * @param peri_id The SPI peripheral identifier index.
 * @param enable Flag to enable (true) or disable (false) error interrupt test mode.
 * @return Flag indicating problems. Returns SPI_FLAG_OK if everything went well.
 */
spi_return_flags_e spi_enable_error_intr_test(const spi_idx_e peri_id, bool enable);

/**
 * Enable or disable event interrupt test mode for a specified SPI peripheral.
 *
 * @param peri_id The SPI peripheral identifier index.
 * @param enable Flag to enable (true) or disable (false) event interrupt test mode.
 * @return Flag indicating problems. Returns SPI_FLAG_OK if everything went well.
 */
spi_return_flags_e spi_enable_evt_intr_test(const spi_idx_e peri_id, bool enable);

/**
 * Trigger a fatal fault test alert for a specified SPI peripheral.
 *
 * @param peri_id The SPI peripheral identifier index.
 * @return Flag indicating problems. Returns SPI_FLAG_OK if everything went well.
 */
spi_return_flags_e spi_alert_test_fatal_fault_trigger(const spi_idx_e peri_id);



// SPI registers access functions

/**
 * Read the TX FIFO depth register.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return TX FIFO depth.
 */
volatile uint8_t spi_get_tx_queue_depth(const spi_idx_e peri_id);

/**
 * Read the TX channel status register.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return TX channel status structure.
 */
volatile spi_ch_status_t spi_get_tx_channel_status(const spi_idx_e peri_id);

/**
 * Read the RX FIFO depth register.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return RX FIFO depth.
 */
volatile uint8_t spi_get_rx_queue_depth(const spi_idx_e peri_id);

/**
 * Read the RX channel status register.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return RX channel status structure.
 */
volatile spi_ch_status_t spi_get_rx_channel_status(const spi_idx_e peri_id);

/**
 * Read the Chip Select (CS) ID register.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return Chip Select (CS) ID.
 */
volatile uint32_t spi_get_csid(const spi_idx_e peri_id);

/**
 * Reset the SPI from software.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
spi_return_flags_e spi_sw_reset(const spi_idx_e peri_id);

/**
 * Enable the SPI host.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param enable SPI enable register value.
 */
spi_return_flags_e spi_set_enable(const spi_idx_e peri_id, bool enable);

/**
 * Set the transmit queue watermark level (to enable interrupt triggering).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param watermark Queue/fifo trigger level (minimum level).
 * @return Flag indicating problems. Returns SPI_WATERMARK_OK == 0 if everithing
 * went well.
 */
spi_return_flags_e spi_set_tx_watermark(const spi_idx_e peri_id, uint8_t watermark);

/**
 * Set the receive queue watermark level (to enable interrupt triggering).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param watermark Queue/fifo trigger level (maximum level).
 * @return Flag indicating problems. Returns SPI_WATERMARK_OK == 0 if everithing
 * went well.
 */
spi_return_flags_e spi_set_rx_watermark(const spi_idx_e peri_id, uint8_t watermark);

/**
 * Set the requirement of a target device (i.e., a slave).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param csid Chip Select (CS) ID.
 * @param conf_reg Slave transmission configuration.
 * @return Flag indicating problems. Returns SPI_CONFIGOPTS_OK == 0 if everithing
 * went well.
 */
spi_return_flags_e spi_set_configopts(const spi_idx_e peri_id, uint32_t csid, uint32_t conf_reg);

/**
 * Select which device to target with the next command.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param csid Chip Select (SC) ID.
 * @return Flag indicating problems. Returns SPI_CSID_OK == 0 if everithing went
 * well.
 */
spi_return_flags_e spi_set_csid(const spi_idx_e peri_id, uint32_t csid);

/**
 * Set the next command (one for all attached SPI devices).
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param cmd_reg Command register value (Length, speed, ...).
 * @return Flag indicating problems. Returns SPI_COMMAND_OK == 0 if everithing
 * went well.
 */
spi_return_flags_e spi_set_command(const spi_idx_e peri_id, uint32_t cmd_reg);

/**
 * Write one word to the TX FIFO.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param wdata Data to write.
 * @return Flag indicating problems. Returns SPI_READ_WRITE_OK == 0 if everithing
 * went well.
 */
spi_return_flags_e spi_write_word(const spi_idx_e peri_id, uint32_t wdata);

/**
 * Read one word to the RX FIFO.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param rdata Read data.
 * @return Flag indicating problems. Returns SPI_READ_WRITE_OK == 0 if everithing
 * went well.
 */
spi_return_flags_e spi_read_word(const spi_idx_e peri_id, uint32_t* dst);

/**
 * Enable SPI event interrupt
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param enable SPI event interrupt bit value.
 */
spi_return_flags_e spi_enable_evt_intr(const spi_idx_e peri_id, bool enable);

/**
 * Enable SPI error interrupt
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param enable SPI error interrupt bit value.
 */
spi_return_flags_e spi_enable_error_intr(const spi_idx_e peri_id, bool enable);

/**
 * Enable SPI output
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param enable SPI TX empty interrupt bit value.
 */
spi_return_flags_e spi_output_enable(const spi_idx_e peri_id, bool enable);

/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/


// TODO: This is lacking sanity checks but by coherence shouldn't have sanity check
//       hence check the proper way to implement this

static inline __attribute__((always_inline)) const bool spi_get_evt_intr_state(const spi_idx_e peri_id) {
    return bitfield_read(spi_peris[peri_id]->INTR_STATE, BIT_MASK_1, SPI_HOST_INTR_STATE_SPI_EVENT_BIT);
}

static inline __attribute__((always_inline)) const bool spi_get_error_intr_state(const spi_idx_e peri_id) {
    return bitfield_read(spi_peris[peri_id]->INTR_STATE, BIT_MASK_1, SPI_HOST_INTR_STATE_ERROR_BIT);
}

static inline __attribute__((always_inline)) const bool spi_get_evt_intr_enable(const spi_idx_e peri_id) {
    return bitfield_read(spi_peris[peri_id]->INTR_ENABLE, BIT_MASK_1, SPI_HOST_INTR_ENABLE_SPI_EVENT_BIT);
}

static inline __attribute__((always_inline)) const bool spi_get_error_intr_enable(const spi_idx_e peri_id) {
    return bitfield_read(spi_peris[peri_id]->INTR_ENABLE, BIT_MASK_1, SPI_HOST_INTR_ENABLE_ERROR_BIT);
}
// ============================================================================


// TODO: This function shouldn't exist... check how to adapt things for w25q.c to work
static inline __attribute__((always_inline)) const uintptr_t spi_get_base_addr(const spi_idx_e peri_id) {
    return (uintptr_t) spi_peris[peri_id];
}

/**
 * Read SPI status register
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) volatile uint32_t spi_get_status(const spi_idx_e peri_id) {
    // TODO: Check if this is a good idea
    // return (spi_status_t*) &spi_peris[peri_id]->STATUS;
    return spi_peris[peri_id]->STATUS;
}

/**
 * Read SPI active bit from status register
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) volatile bool spi_get_active(const spi_idx_e peri_id) {
    // TODO: Find better approach to inform user
    if (SPI_IDX_INVALID(peri_id)) return false;
    return bitfield_read(spi_get_status(peri_id), BIT_MASK_1, SPI_HOST_STATUS_ACTIVE_BIT);
}

/**
 * Read SPI ready bit from status register
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) volatile bool spi_get_ready(const spi_idx_e peri_id) {
    // TODO: Find better approach to inform user
    if (SPI_IDX_INVALID(peri_id)) return false;
    return bitfield_read(spi_get_status(peri_id), BIT_MASK_1, SPI_HOST_STATUS_READY_BIT);
}

/**
 * Wait SPI is ready to receive commands.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_ready(const spi_idx_e peri_id) {
    while (!spi_get_ready(peri_id));
}

/**
 * Wait SPI is no longer processing commands.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_idle(const spi_idx_e peri_id) {
    while (spi_get_active(peri_id));
}

/**
 * Wait TX FIFO reach watermark.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_watermark(const spi_idx_e peri_id) {
    // TODO: Find better approach to inform user
    if (SPI_IDX_INVALID(peri_id)) return;
    while (!bitfield_read(spi_get_status(peri_id), BIT_MASK_1, SPI_HOST_STATUS_TXWM_BIT));
}

/**
 * Wait TX FIFO empty.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_empty(const spi_idx_e peri_id) {
    // TODO: Find better approach to inform user
    if (SPI_IDX_INVALID(peri_id)) return;
    while (!bitfield_read(spi_get_status(peri_id), BIT_MASK_1, SPI_HOST_STATUS_TXEMPTY_BIT));
}

/**
 * Wait TX FIFO not empty.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_not_empty(const spi_idx_e peri_id) {
    // TODO: Find better approach to inform user
    if (SPI_IDX_INVALID(peri_id)) return;
    while (!bitfield_read(spi_get_status(peri_id), BIT_MASK_1, SPI_HOST_STATUS_TXEMPTY_BIT));
}

/**
 * Wait TX FIFO not full.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_tx_not_full(const spi_idx_e peri_id) {
    // TODO: Find better approach to inform user
    if (SPI_IDX_INVALID(peri_id)) return;
    while (!bitfield_read(spi_get_status(peri_id), BIT_MASK_1, SPI_HOST_STATUS_TXFULL_BIT));
}

/**
 * Wait RX FIFO empty.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_rx_empty(const spi_idx_e peri_id) {
    // TODO: Find better approach to inform user
    if (SPI_IDX_INVALID(peri_id)) return;
    while (!bitfield_read(spi_get_status(peri_id), BIT_MASK_1, SPI_HOST_STATUS_RXEMPTY_BIT));
}

/**
 * Wait RX FIFO not empty.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_rx_not_empty(const spi_idx_e peri_id) {
    // TODO: Find better approach to inform user
    if (SPI_IDX_INVALID(peri_id)) return;
    while (!bitfield_read(spi_get_status(peri_id), BIT_MASK_1, SPI_HOST_STATUS_RXEMPTY_BIT));
}

/**
 * Wait RX FIFO not full.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_rx_not_full(const spi_idx_e peri_id) {
    // TODO: Find better approach to inform user
    if (SPI_IDX_INVALID(peri_id)) return;
    while (!bitfield_read(spi_get_status(peri_id), BIT_MASK_1, SPI_HOST_STATUS_RXFULL_BIT));
}

/**
 * Wait RX FIFO reach watermark.
 *
 * @param spi Pointer to spi_host_t representing the target SPI.
 */
static inline __attribute__((always_inline)) void spi_wait_for_rx_watermark(const spi_idx_e peri_id) {
    // TODO: Find better approach to inform user
    if (SPI_IDX_INVALID(peri_id)) return;
    while (!bitfield_read(spi_get_status(peri_id), BIT_MASK_1, SPI_HOST_STATUS_RXWM_BIT));
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
void handler_irq_spi(uint32_t id);

/**
 * @brief Attends the plic interrupt.
 */
void fic_irq_spi(void);

/**
 * @brief Attends the plic interrupt.
 */
void fic_irq_spi_flash(void);


/**
 * @brief weak implementation of the function that gets called when an event interrupt is
 *        triggered on the SPI Flash.
 *        Replace with your own implementation.
 */
__attribute__((weak, optimize("O0"))) void spi_intr_handler_event_flash(spi_event_e events);

/**
 * @brief weak implementation of the function that gets called when an error interrupt is
 *        triggered on the SPI Flash.
 *        Replace with your own implementation.
 */
__attribute__((weak, optimize("O0"))) void spi_intr_handler_error_flash(spi_error_e errors);

/**
 * @brief weak implementation of the function that gets called when an event interrupt is
 *        triggered on the SPI Host.
 *        Replace with your own implementation.
 */
__attribute__((weak, optimize("O0"))) void spi_intr_handler_event_host(spi_event_e events);

/**
 * @brief weak implementation of the function that gets called when an error interrupt is
 *        triggered on the SPI Host.
 *        Replace with your own implementation.
 */
__attribute__((weak, optimize("O0"))) void spi_intr_handler_error_host(spi_error_e errors);

/**
 * @brief weak implementation of the function that gets called when an event interrupt is
 *        triggered on the SPI Host 2.
 *        Replace with your own implementation.
 */
__attribute__((weak, optimize("O0"))) void spi_intr_handler_event_host2(spi_event_e events);

/**
 * @brief weak implementation of the function that gets called when an error interrupt is
 *        triggered on the SPI Host 2.
 *        Replace with your own implementation.
 */
__attribute__((weak, optimize("O0"))) void spi_intr_handler_error_host2(spi_error_e errors);


#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_SPI_HOST_H_

/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
