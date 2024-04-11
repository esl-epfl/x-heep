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

#define SPI_EVENTS_MASK     BIT_MASK_6
#define SPI_EVENTS_INDEX    0
#define SPI_ERRORS_IRQ_MASK BIT_MASK_5
#define SPI_ERRORS_MASK     BIT_MASK_6
#define SPI_ERRORS_INDEX    0

/****************************************************************************/
/**                                                                        **/
/*                        TYPEDEFS AND STRUCTURES                           */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

spi_return_flags_e spi_get_events(const spi_idx_e peri_id, spi_event_e* events);
spi_return_flags_e spi_get_errors(const spi_idx_e peri_id, spi_error_e* errors);

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


spi_return_flags_e spi_get_events_enabled(const spi_idx_e peri_id, spi_event_e* events) 
{
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    *events = bitfield_read(spi_peris[peri_id]->EVENT_ENABLE, SPI_EVENTS_MASK, SPI_EVENTS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_events_enabled(const spi_idx_e peri_id, spi_event_e* events, bool enable) 
{
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    if (*events > SPI_EVENT_ALL) return SPI_FLAG_EVENT_INVALID;
    if (enable) spi_peris[peri_id]->EVENT_ENABLE |= *events;
    else        spi_peris[peri_id]->EVENT_ENABLE &= ~*events;

    *events = bitfield_read(spi_peris[peri_id]->EVENT_ENABLE, SPI_EVENTS_MASK, SPI_EVENTS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_get_errors_enabled(const spi_idx_e peri_id, spi_error_e* errors) 
{
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;

    *errors = bitfield_read(spi_peris[peri_id]->ERROR_ENABLE, SPI_ERRORS_IRQ_MASK, SPI_ERRORS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_errors_enabled(const spi_idx_e peri_id, spi_error_e* errors, bool enable)
{
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    if (*errors > SPI_ERROR_IRQALL) return SPI_FLAG_ERROR_INVALID;
    if (enable) spi_peris[peri_id]->ERROR_ENABLE |= *errors;
    else        spi_peris[peri_id]->ERROR_ENABLE &= ~*errors;

    *errors = bitfield_read(spi_peris[peri_id]->ERROR_ENABLE, SPI_ERRORS_IRQ_MASK, SPI_ERRORS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_write_byte(const spi_idx_e peri_id, uint8_t bdata) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    if (spi_get_tx_queue_depth(peri_id) >= SPI_HOST_PARAM_TX_DEPTH) return SPI_FLAG_TX_QUEUE_FULL;
    spi_peris[peri_id]->TXDATA = bdata;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_error_intr_test(const spi_idx_e peri_id, bool enable) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi_peris[peri_id]->INTR_TEST;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_TEST_ERROR_BIT, enable);
    spi_peris[peri_id]->INTR_TEST = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_evt_intr_test(const spi_idx_e peri_id, bool enable) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi_peris[peri_id]->INTR_TEST;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_TEST_SPI_EVENT_BIT, enable);
    spi_peris[peri_id]->INTR_TEST = intr_enable_reg;
    return SPI_FLAG_OK;
}

// TODO: Improve the implementation for this function since this is for fatal_fault alert only
spi_return_flags_e spi_enable_alert_test(const spi_idx_e peri_id, bool enable) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi_peris[peri_id]->ALERT_TEST;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_ALERT_TEST_FATAL_FAULT_BIT, enable);
    spi_peris[peri_id]->ALERT_TEST = intr_enable_reg;
    return SPI_FLAG_OK;
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
    volatile uint32_t status = spi_get_status(peri_id);
    while (status & (1 << SPI_HOST_STATUS_ACTIVE_BIT
                     | SPI_HOST_STATUS_TXQD_MASK << SPI_HOST_STATUS_TXQD_OFFSET
                     | SPI_HOST_STATUS_RXQD_MASK << SPI_HOST_STATUS_RXQD_OFFSET)
          );
    ctrl_reg = spi_peris[peri_id]->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, BIT_MASK_1, SPI_HOST_CONTROL_SW_RST_BIT, 0);
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
    spi_return_flags_e flags = SPI_FLAG_OK;
    if (SPI_IDX_INVALID(peri_id)) flags += SPI_FLAG_NULL_PTR;
    if (watermark > SPI_HOST_PARAM_TX_DEPTH) flags += SPI_FLAG_WATERMARK_EXCEEDS;
    if (flags) return flags;

    volatile uint32_t ctrl_reg = spi_peris[peri_id]->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, SPI_HOST_CONTROL_TX_WATERMARK_MASK, SPI_HOST_CONTROL_TX_WATERMARK_OFFSET, watermark);
    spi_peris[peri_id]->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_rx_watermark(const spi_idx_e peri_id, uint8_t watermark) {
    spi_return_flags_e flags = SPI_FLAG_OK;
    if (SPI_IDX_INVALID(peri_id)) flags += SPI_FLAG_NULL_PTR;
    if (watermark > SPI_HOST_PARAM_RX_DEPTH) flags += SPI_FLAG_WATERMARK_EXCEEDS;
    if (flags) return flags;

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
    spi_return_flags_e flags = SPI_FLAG_OK;
    if (SPI_IDX_INVALID(peri_id)) flags += SPI_FLAG_NULL_PTR;
    if (SPI_CSID_INVALID(csid)) flags += SPI_FLAG_CSID_INVALID;
    if (flags) return flags;

    spi_peris[peri_id]->CSID = csid;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_command(const spi_idx_e peri_id, const uint32_t cmd_reg) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;

    spi_return_flags_e flags = SPI_FLAG_OK;
    spi_speed_e speed = bitfield_read(cmd_reg, SPI_HOST_COMMAND_SPEED_MASK, SPI_HOST_COMMAND_SPEED_OFFSET);
    spi_dir_e direction = bitfield_read(cmd_reg, SPI_HOST_COMMAND_DIRECTION_MASK, SPI_HOST_COMMAND_DIRECTION_OFFSET);

    if (bitfield_read(spi_get_status(peri_id), SPI_HOST_STATUS_CMDQD_MASK, SPI_HOST_STATUS_CMDQD_OFFSET) >= SPI_HOST_PARAM_CMD_DEPTH)
        flags += SPI_FLAG_COMMAND_FULL;
    if (speed > SPI_SPEED_QUAD || (direction == SPI_DIR_BIDIR && speed != SPI_SPEED_STANDARD))
        flags += SPI_FLAG_SPEED_INVALID;
    if (!spi_get_ready(peri_id)) flags += SPI_FLAG_NOT_READY;
    if (flags) return flags;

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

spi_return_flags_e spi_output_enable(const spi_idx_e peri_id, bool enable){
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t output_enable_reg = spi_peris[peri_id]->CONTROL;
    output_enable_reg = bitfield_write(output_enable_reg, BIT_MASK_1, SPI_HOST_CONTROL_OUTPUT_EN_BIT, enable);
    spi_peris[peri_id]->CONTROL = output_enable_reg;
    return SPI_FLAG_OK;
}

void handler_irq_spi(uint32_t id)
{
    spi_error_e errors;
    spi_get_errors(spi_peris[SPI_IDX_HOST_2], &errors);
    if (errors) {
        spi_intr_handler_error_host2(errors);
    }
    else {
        spi_event_e events;
        spi_get_events(spi_peris[SPI_IDX_HOST_2], &events);
        spi_intr_handler_event_host2(events);
    }
}

void fic_irq_spi(void)
{
    spi_error_e errors;
    spi_get_errors(spi_peris[SPI_IDX_HOST], &errors);
    if (errors) {
        spi_intr_handler_error_host(errors);
    }
    else {
        spi_event_e events;
        spi_get_events(spi_peris[SPI_IDX_HOST], &events);
        spi_intr_handler_event_host(events);
    }
}

void fic_irq_spi_flash(void)
{
    spi_error_e errors;
    spi_get_errors(spi_peris[SPI_IDX_FLASH], &errors);
    if (errors) {
        spi_intr_handler_error_flash(errors);
    }
    else {
        spi_event_e events;
        spi_get_events(spi_peris[SPI_IDX_FLASH], &events);
        spi_intr_handler_event_flash(events);
    }
}

__attribute__((weak, optimize("O0"))) void spi_intr_handler_event_flash(spi_event_e events) {

}

__attribute__((weak, optimize("O0"))) void spi_intr_handler_error_flash(spi_error_e errors) {

}

__attribute__((weak, optimize("O0"))) void spi_intr_handler_event_host(spi_event_e events) {

}

__attribute__((weak, optimize("O0"))) void spi_intr_handler_error_host(spi_error_e errors) {

}

__attribute__((weak, optimize("O0"))) void spi_intr_handler_event_host2(spi_event_e events) {

}

__attribute__((weak, optimize("O0"))) void spi_intr_handler_error_host2(spi_error_e errors) {


}

/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

spi_return_flags_e spi_get_events(const spi_idx_e peri_id, spi_event_e* events) {
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    volatile uint32_t status = spi_get_status(peri_id);
    *events |= bitfield_read(status, BIT_MASK_1, SPI_HOST_STATUS_RXFULL_BIT)  << SPI_HOST_EVENT_ENABLE_RXFULL_BIT
             | bitfield_read(status, BIT_MASK_1, SPI_HOST_STATUS_TXEMPTY_BIT) << SPI_HOST_EVENT_ENABLE_TXEMPTY_BIT
             | bitfield_read(status, BIT_MASK_1, SPI_HOST_STATUS_RXWM_BIT)    << SPI_HOST_EVENT_ENABLE_RXWM_BIT
             | bitfield_read(status, BIT_MASK_1, SPI_HOST_STATUS_TXWM_BIT)    << SPI_HOST_EVENT_ENABLE_TXWM_BIT
             | bitfield_read(status, BIT_MASK_1, SPI_HOST_STATUS_READY_BIT)   << SPI_HOST_EVENT_ENABLE_READY_BIT
             | ~bitfield_read(status, BIT_MASK_1, SPI_HOST_STATUS_ACTIVE_BIT) << SPI_HOST_EVENT_ENABLE_IDLE_BIT;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_get_errors(const spi_idx_e peri_id, spi_error_e* errors)
{
    if (SPI_IDX_INVALID(peri_id)) return SPI_FLAG_NULL_PTR;
    *errors = bitfield_read(spi_peris[peri_id]->ERROR_ENABLE, SPI_ERRORS_MASK, SPI_ERRORS_INDEX);
    return SPI_FLAG_OK;
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
