/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************
**
** project  : X-HEEP
** filename : spi_host.c
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
* @file   spi_host.c
* @date   06/03/24
* @brief  The Serial Peripheral Interface (SPI) driver to set up and use the
* SPI peripheral
*/

/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "spi_host.h"

#include "mmio.h"
#include "bitfield.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                        TYPEDEFS AND STRUCTURES                           */
/**                                                                        **/
/****************************************************************************/

#define MAX_SPEED 2

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED VARIABLES                             */
/**                                                                        **/
/****************************************************************************/

volatile spi_host* const spi_peris[4] = {
    ((volatile spi_host *) SPI_FLASH_START_ADDRESS),
    ((volatile spi_host *) SPI_MEMIO_START_ADDRESS),
    ((volatile spi_host *) SPI_HOST_START_ADDRESS),
    ((volatile spi_host *) SPI2_START_ADDRESS)
};

/****************************************************************************/
/**                                                                        **/
/*                            GLOBAL VARIABLES                              */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/


volatile spi_event_e spi_get_events_enabled(const spi_idx_e peri_id) {
    if (SPI_IDX_INVALID(peri_id)) return 0;
    return bitfield_read(spi_peris[peri_id]->EVENT_ENABLE, BIT_MASK_6, 0);
}

volatile spi_event_e spi_set_events_enabled(const spi_idx_e peri_id, spi_event_e events, bool enable) {
    if (SPI_IDX_INVALID(peri_id)) return 0;
    if (events >= __SPI_EVENT_INVALID) return 0;
    if (enable) spi_peris[peri_id]->EVENT_ENABLE |= events;
    else spi_peris[peri_id]->EVENT_ENABLE &= ~events;
    return bitfield_read(spi_peris[peri_id]->EVENT_ENABLE, BIT_MASK_6, 0);
}

volatile spi_event_e spi_get_events(const spi_idx_e peri_id) {

}

volatile spi_error_e spi_get_errors_enabled(const spi_idx_e peri_id) {
    if (SPI_IDX_INVALID(peri_id)) return 0;
    return bitfield_read(spi_peris[peri_id]->ERROR_ENABLE, BIT_MASK_5, 0);
}

volatile spi_error_e spi_set_errors_enabled(const spi_idx_e peri_id, spi_error_e errors, bool enable) {
    if (SPI_IDX_INVALID(peri_id)) return 0;
    if (errors >= __SPI_ERROR_INVALID) return 0;
    if (enable) spi_peris[peri_id]->ERROR_ENABLE |= errors;
    else spi_peris[peri_id]->ERROR_ENABLE &= ~errors;
    return bitfield_read(spi_peris[peri_id]->ERROR_ENABLE, BIT_MASK_5, 0);
}

volatile spi_error_e spi_get_errors(const spi_idx_e peri_id) {
    if (SPI_IDX_INVALID(peri_id)) return 0;
    return bitfield_read(spi_peris[peri_id]->ERROR_STATUS, BIT_MASK_6, 0);
}

// TODO: This is dangerous at the moment, needs more safety checks
spi_return_flags_e spi_transaction(const spi_idx_e peri_id, const uint32_t* segments, const uint8_t len) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    for (int i = 0; i < len; i++) {
        spi_wait_for_ready(peri_id);
        spi_return_flags_e flag = spi_set_command(peri_id, segments[i]);
        if (flag) return flag;
    }
    return SPI_FLAG_OK;
}

uint8_t spi_read(const spi_idx_e peri_id, uint32_t* dst, const uint8_t len) {
    uint8_t rx_depth = spi_get_rx_queue_depth(peri_id);
    uint8_t true_len = len > rx_depth ? rx_depth : len;
    for (int i = 0; i < true_len; i++)
    {
        if (spi_read_word(peri_id, &dst[i])) return i;
    }
    return true_len;
}

uint8_t spi_write(const spi_idx_e peri_id, uint32_t* src, const uint8_t len) {
    // TODO: Think if it is not better to abort whole writing if not enough space
    uint8_t tx_depth = spi_get_rx_queue_depth(peri_id);
    uint8_t true_len = len < tx_depth ? tx_depth : len;
    for (int i = 0; i < true_len; i++)
    {
        if (spi_write_word(peri_id, src[i])) return i;
    }
    return true_len;
}

// SPI get functions

// return max(uint8_t) on NULL pointer
volatile uint8_t spi_get_tx_queue_depth(const spi_idx_e peri_id) {
    if (SPI_IDX_INVALID(peri_id)) return -1;
    volatile uint32_t status_reg = spi_get_status(peri_id);
    return bitfield_read(status_reg, SPI_HOST_STATUS_TXQD_MASK, SPI_HOST_STATUS_TXQD_OFFSET);
}

volatile spi_ch_status_t spi_get_tx_channel_status(const spi_idx_e peri_id) {
    // TODO: Find a good idea to return error flag if spi == NULL
    volatile uint32_t status_reg = spi_get_status(peri_id);
    spi_ch_status_t ch_status = {
        .empty = bitfield_read(status_reg, BIT_MASK_1, SPI_HOST_STATUS_TXEMPTY_BIT),
        .full = bitfield_read(status_reg, BIT_MASK_1, SPI_HOST_STATUS_TXFULL_BIT),
        .stall = bitfield_read(status_reg, BIT_MASK_1, SPI_HOST_STATUS_TXSTALL_BIT),
        .wm = bitfield_read(status_reg, BIT_MASK_1, SPI_HOST_STATUS_TXWM_BIT)
    };
    return ch_status;
}

// return max(uint8_t) on NULL pointer
volatile uint8_t spi_get_rx_queue_depth(const spi_idx_e peri_id) {
    if (SPI_IDX_INVALID(peri_id)) return -1;
    volatile uint32_t status_reg = spi_get_status(peri_id);
    return bitfield_read(status_reg, SPI_HOST_STATUS_RXQD_MASK, SPI_HOST_STATUS_RXQD_OFFSET);
}

volatile spi_ch_status_t spi_get_rx_channel_status(const spi_idx_e peri_id) {
    // TODO: Find a good idea to return error flag if spi == NULL
    volatile uint32_t status_reg = spi_get_status(peri_id);
    spi_ch_status_t ch_status = {
        .empty = bitfield_read(status_reg, BIT_MASK_1, SPI_HOST_STATUS_RXEMPTY_BIT),
        .full = bitfield_read(status_reg, BIT_MASK_1, SPI_HOST_STATUS_RXFULL_BIT),
        .stall = bitfield_read(status_reg, BIT_MASK_1, SPI_HOST_STATUS_RXSTALL_BIT),
        .wm = bitfield_read(status_reg, BIT_MASK_1, SPI_HOST_STATUS_RXWM_BIT)
    };
    return ch_status;
}

// return max(uint32_t) on NULL pointer
volatile uint32_t spi_get_csid(const spi_idx_e peri_id) {
    if (SPI_IDX_INVALID(peri_id)) return -1;
    return spi_peris[peri_id]->CSID;
}

// SPI set functions

spi_return_flags_e spi_sw_reset(const spi_idx_e peri_id) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t ctrl_reg = spi_peris[peri_id]->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, BIT_MASK_1, SPI_HOST_CONTROL_SW_RST_BIT, 1);
    spi_peris[peri_id]->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_enable(const spi_idx_e peri_id, bool enable) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t ctrl_reg = spi_peris[peri_id]->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, BIT_MASK_1, SPI_HOST_CONTROL_SPIEN_BIT, enable);
    spi_peris[peri_id]->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_tx_watermark(const spi_idx_e peri_id, uint8_t watermark) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    if (watermark > SPI_HOST_PARAM_TX_DEPTH) return SPI_FLAG_WATERMARK_EXCEEDS;
    volatile uint32_t ctrl_reg = spi_peris[peri_id]->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, SPI_HOST_CONTROL_TX_WATERMARK_MASK, SPI_HOST_CONTROL_TX_WATERMARK_OFFSET, watermark);
    spi_peris[peri_id]->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_rx_watermark(const spi_idx_e peri_id, uint8_t watermark) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    if (watermark > SPI_HOST_PARAM_RX_DEPTH) return SPI_FLAG_WATERMARK_EXCEEDS;
    volatile uint32_t ctrl_reg = spi_peris[peri_id]->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, SPI_HOST_CONTROL_RX_WATERMARK_MASK, SPI_HOST_CONTROL_RX_WATERMARK_OFFSET, watermark);
    spi_peris[peri_id]->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_configopts(const spi_idx_e peri_id, uint32_t csid, const uint32_t conf_reg) {
    // TODO: check if this could be generalized to more than 2 CSIDs... because right 
    // now not very consistent with spi_set_csid which uses SPI_HOST_PARAM_NUM_C_S
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    switch (csid)
    {
    case 0:
        spi_peris[peri_id]->CONFIGOPTS0 = conf_reg;
        break;
    case 1:
        spi_peris[peri_id]->CONFIGOPTS1 = conf_reg;
        break;
    default:
        return SPI_FLAG_CSID_INVALID;
        break;
    }
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_csid(const spi_idx_e peri_id, uint32_t csid) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    if (csid >= SPI_HOST_PARAM_NUM_C_S) return SPI_FLAG_CSID_INVALID;
    spi_peris[peri_id]->CSID = csid;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_command(const spi_idx_e peri_id, const uint32_t cmd_reg) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    if (bitfield_read(spi_get_status(peri_id), SPI_HOST_STATUS_CMDQD_MASK, SPI_HOST_STATUS_CMDQD_OFFSET) >= SPI_HOST_PARAM_CMD_DEPTH)
        return SPI_FLAG_COMMAND_FULL;
    if (bitfield_read(cmd_reg, SPI_HOST_COMMAND_SPEED_MASK, SPI_HOST_COMMAND_SPEED_OFFSET) > MAX_SPEED)
        return SPI_FLAG_SPEED_INVALID;
    spi_peris[peri_id]->COMMAND = cmd_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_write_word(const spi_idx_e peri_id, uint32_t wdata) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    if (spi_get_tx_queue_depth(peri_id) >= SPI_HOST_PARAM_TX_DEPTH) return SPI_FLAG_TX_QUEUE_FULL;
    spi_peris[peri_id]->TXDATA = wdata;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_read_word(const spi_idx_e peri_id, uint32_t* dst) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    if (spi_get_rx_queue_depth(peri_id) == 0) return SPI_FLAG_RX_QUEUE_EMPTY;
    *dst = spi_peris[peri_id]->RXDATA;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_evt_intr(const spi_idx_e peri_id, bool enable) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi_peris[peri_id]->INTR_ENABLE;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_ENABLE_SPI_EVENT_BIT, enable);
    spi_peris[peri_id]->INTR_ENABLE = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_error_intr(const spi_idx_e peri_id, bool enable) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi_peris[peri_id]->INTR_ENABLE;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_STATE_ERROR_BIT, enable);
    spi_peris[peri_id]->INTR_ENABLE = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_rxwm_intr(const spi_idx_e peri_id, bool enable) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi_peris[peri_id]->EVENT_ENABLE;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_EVENT_ENABLE_RXWM_BIT, enable);
    spi_peris[peri_id]->EVENT_ENABLE = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_txempty_intr(const spi_idx_e peri_id, bool enable) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi_peris[peri_id]->EVENT_ENABLE;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_EVENT_ENABLE_TXEMPTY_BIT, enable);
    spi_peris[peri_id]->EVENT_ENABLE = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_output_enable(const spi_idx_e peri_id, bool enable){
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t output_enable_reg = spi_peris[peri_id]->CONTROL;
    output_enable_reg = bitfield_write(output_enable_reg, BIT_MASK_1, SPI_HOST_CONTROL_OUTPUT_EN_BIT, enable);
    spi_peris[peri_id]->CONTROL = output_enable_reg;
    return SPI_FLAG_OK;
}

__attribute__((weak, optimize("O0"))) void handler_irq_spi(uint32_t id)
{
 // Replace this function with a non-weak implementation
}

/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
