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

#include "fast_intr_ctrl.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

#define SPI_EVENTS_INDEX    0
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

/**
 * @brief Function that translates the status register to spi_event_e type. This
 * has been implemented because the SPI peripheral has no other way to know which
 * event was triggered other than reading the status.
 * This function has been made local since only the event interrupt handler should
 * need it.
 * 
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @param events Pointer to store the events that were triggered.
 * @return spi_return_flags_e indicating problems, SPI_FLAG_OK if all went well.
 */
spi_return_flags_e spi_get_events(spi_host_t* spi, spi_event_e* events);

/**
 * @brief Function to acknoledge event interrupts once received to prevent them
 * from triggering again.
 * 
 * @param spi Pointer to spi_host_t representing the target SPI.
 * @return spi_return_flags_e indicating problems, SPI_FLAG_OK if all went well.
 */
spi_return_flags_e spi_acknowledge_event(spi_host_t* spi);

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED VARIABLES                             */
/**                                                                        **/
/****************************************************************************/

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

spi_return_flags_e spi_get_events_enabled(spi_host_t* spi, spi_event_e* events) 
{
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    *events = bitfield_read(SPI_HOST_HW(spi)->EVENT_ENABLE, SPI_EVENT_ALL, SPI_EVENTS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_events_enabled(spi_host_t* spi, spi_event_e events, bool enable) 
{
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    if (events > SPI_EVENT_ALL) return SPI_FLAG_EVENT_INVALID;

    if (enable) SPI_HOST_HW(spi)->EVENT_ENABLE |= events;
    else        SPI_HOST_HW(spi)->EVENT_ENABLE &= ~events;

    return SPI_FLAG_OK;
}

spi_return_flags_e spi_get_errors_enabled(spi_host_t* spi, spi_error_e* errors) 
{
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    *errors = bitfield_read(SPI_HOST_HW(spi)->ERROR_ENABLE, SPI_ERROR_IRQALL, SPI_ERRORS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_errors_enabled(spi_host_t* spi, spi_error_e errors, bool enable)
{
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    if (errors > SPI_ERROR_IRQALL) return SPI_FLAG_ERROR_INVALID;

    if (enable) SPI_HOST_HW(spi)->ERROR_ENABLE |= errors;
    else        SPI_HOST_HW(spi)->ERROR_ENABLE &= ~errors;

    return SPI_FLAG_OK;
}

spi_return_flags_e spi_get_errors(spi_host_t* spi, spi_error_e* errors)
{
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    *errors = bitfield_read(SPI_HOST_HW(spi)->ERROR_STATUS, SPI_ERROR_ALL, SPI_ERRORS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_acknowledge_errors(spi_host_t* spi) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    SPI_HOST_HW(spi)->ERROR_STATUS = bitfield_write(SPI_HOST_HW(spi)->ERROR_STATUS, SPI_ERROR_ALL, SPI_ERRORS_INDEX, SPI_ERROR_ALL);
    SPI_HOST_HW(spi)->INTR_STATE   = bitfield_write(SPI_HOST_HW(spi)->INTR_STATE, BIT_MASK_1, SPI_HOST_INTR_STATE_ERROR_BIT, 1);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_write_byte(spi_host_t* spi, uint8_t bdata) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    if (spi_get_tx_queue_depth(spi) >= SPI_HOST_PARAM_TX_DEPTH) return SPI_FLAG_TX_QUEUE_FULL;
    SPI_HOST_HW(spi)->TXDATA = bdata;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_error_intr_test(spi_host_t* spi, bool enable) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = SPI_HOST_HW(spi)->INTR_TEST;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_TEST_ERROR_BIT, enable);
    SPI_HOST_HW(spi)->INTR_TEST = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_evt_intr_test(spi_host_t* spi, bool enable) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = SPI_HOST_HW(spi)->INTR_TEST;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_TEST_SPI_EVENT_BIT, enable);
    SPI_HOST_HW(spi)->INTR_TEST = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_alert_test_fatal_fault_trigger(spi_host_t* spi) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = SPI_HOST_HW(spi)->ALERT_TEST;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_ALERT_TEST_FATAL_FAULT_BIT, true);
    SPI_HOST_HW(spi)->ALERT_TEST = intr_enable_reg;
    return SPI_FLAG_OK;
}

// TODO: This is redundant... just use get_status...
volatile uint8_t spi_get_tx_queue_depth(spi_host_t* spi) {
    if (spi == NULL) return UINT8_MAX;
    return spi_get_status(spi)->txqd;
}

// TODO: This is redundant... just use get_status...
spi_return_flags_e spi_get_tx_channel_status(spi_host_t* spi, volatile spi_ch_status_t* ch_status) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile spi_status_t* status = spi_get_status(spi);
    ch_status->empty = status->txempty;
    ch_status->full  = status->txfull;
    ch_status->stall = status->txstall;
    ch_status->wm    = status->txwm;
    return SPI_FLAG_OK;
}

// TODO: This is redundant... just use get_status...
volatile uint8_t spi_get_rx_queue_depth(spi_host_t* spi) {
    if (spi == NULL) return UINT8_MAX;
    return spi_get_status(spi)->rxqd;
}

// TODO: This is redundant... just use get_status...
spi_return_flags_e spi_get_rx_channel_status(spi_host_t* spi, volatile spi_ch_status_t* ch_status) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile spi_status_t* status = spi_get_status(spi);
    ch_status->empty = status->rxempty;
    ch_status->full  = status->rxfull;
    ch_status->stall = status->rxstall;
    ch_status->wm    = status->rxwm;
    return SPI_FLAG_OK;
}

volatile uint32_t spi_get_csid(spi_host_t* spi) {
    if (spi == NULL) return UINT32_MAX;
    return SPI_HOST_HW(spi)->CSID;
}

spi_return_flags_e spi_sw_reset(spi_host_t* spi) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;

    // Assert spi reset bit
    volatile uint32_t ctrl_reg = SPI_HOST_HW(spi)->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, BIT_MASK_1, SPI_HOST_CONTROL_SW_RST_BIT, true);
    SPI_HOST_HW(spi)->CONTROL = ctrl_reg;

    volatile spi_status_t* status = spi_get_status(spi);

    // Wait for spi active and txqd & rxqd both go to 0
    while (status->active || status->txqd || status->rxqd);

    // Deassert spi reset bit
    ctrl_reg = bitfield_write(ctrl_reg, BIT_MASK_1, SPI_HOST_CONTROL_SW_RST_BIT, false);
    SPI_HOST_HW(spi)->CONTROL = ctrl_reg;
    
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_enable(spi_host_t* spi, bool enable) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t ctrl_reg = SPI_HOST_HW(spi)->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, BIT_MASK_1, SPI_HOST_CONTROL_SPIEN_BIT, enable);
    SPI_HOST_HW(spi)->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_tx_watermark(spi_host_t* spi, uint8_t watermark) {
    spi_return_flags_e flags = SPI_FLAG_OK;
    if (spi == NULL)                    flags |= SPI_FLAG_NULL_PTR;
    if (watermark > SPI_HOST_PARAM_TX_DEPTH) flags |= SPI_FLAG_WATERMARK_EXCEEDS;
    if (flags) return flags;

    volatile uint32_t ctrl_reg = SPI_HOST_HW(spi)->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, SPI_HOST_CONTROL_TX_WATERMARK_MASK, SPI_HOST_CONTROL_TX_WATERMARK_OFFSET, watermark);
    SPI_HOST_HW(spi)->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_rx_watermark(spi_host_t* spi, uint8_t watermark) {
    spi_return_flags_e flags = SPI_FLAG_OK;
    if (spi == NULL)                    flags |= SPI_FLAG_NULL_PTR;
    if (watermark > SPI_HOST_PARAM_RX_DEPTH) flags |= SPI_FLAG_WATERMARK_EXCEEDS;
    if (flags) return flags;

    volatile uint32_t ctrl_reg = SPI_HOST_HW(spi)->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, SPI_HOST_CONTROL_RX_WATERMARK_MASK, SPI_HOST_CONTROL_RX_WATERMARK_OFFSET, watermark);
    SPI_HOST_HW(spi)->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_configopts(spi_host_t* spi, uint32_t csid, const uint32_t conf_reg) {
    // TODO: check if this could be generalized to more than 2 CSIDs... because right 
    // now not very consistent with spi_set_csid which uses SPI_HOST_PARAM_NUM_C_S
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    switch (csid)
    {
    case 0:
        SPI_HOST_HW(spi)->CONFIGOPTS0 = conf_reg;
        break;
    case 1:
        SPI_HOST_HW(spi)->CONFIGOPTS1 = conf_reg;
        break;
    default:
        return SPI_FLAG_CSID_INVALID;
    }
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_get_configopts(spi_host_t* spi, uint32_t csid, uint32_t* conf_reg) {
    // TODO: check if this could be generalized to more than 2 CSIDs... because right 
    // now not very consistent with spi_set_csid which uses SPI_HOST_PARAM_NUM_C_S
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    switch (csid)
    {
    case 0:
        *conf_reg = SPI_HOST_HW(spi)->CONFIGOPTS0;
        break;
    case 1:
        *conf_reg = SPI_HOST_HW(spi)->CONFIGOPTS1;
        break;
    default:
        return SPI_FLAG_CSID_INVALID;
    }
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_csid(spi_host_t* spi, uint32_t csid) {
    spi_return_flags_e flags = SPI_FLAG_OK;
    if (spi == NULL)       flags |= SPI_FLAG_NULL_PTR;
    if (SPI_CSID_INVALID(csid)) flags |= SPI_FLAG_CSID_INVALID;
    if (flags) return flags;

    SPI_HOST_HW(spi)->CSID = csid;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_command(spi_host_t* spi, const uint32_t cmd_reg) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;

    spi_return_flags_e flags = SPI_FLAG_OK;
    spi_speed_e speed   = bitfield_read(cmd_reg, SPI_HOST_COMMAND_SPEED_MASK, SPI_HOST_COMMAND_SPEED_OFFSET);
    spi_dir_e direction = bitfield_read(cmd_reg, SPI_HOST_COMMAND_DIRECTION_MASK, SPI_HOST_COMMAND_DIRECTION_OFFSET);

    if (!spi_validate_cmd(direction, speed))     flags |= SPI_FLAG_SPEED_INVALID;
    if (spi_get_ready(spi) != SPI_TRISTATE_TRUE) flags |= SPI_FLAG_NOT_READY;
    if (flags) return flags;

    SPI_HOST_HW(spi)->COMMAND = cmd_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_write_word(spi_host_t* spi, uint32_t wdata) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    if (spi_get_tx_queue_depth(spi) >= SPI_HOST_PARAM_TX_DEPTH) return SPI_FLAG_TX_QUEUE_FULL;
    SPI_HOST_HW(spi)->TXDATA = wdata;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_read_word(spi_host_t* spi, uint32_t* dst) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    if (spi_get_rx_queue_depth(spi) == 0) return SPI_FLAG_RX_QUEUE_EMPTY;
    *dst = SPI_HOST_HW(spi)->RXDATA;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_evt_intr(spi_host_t* spi, bool enable) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = SPI_HOST_HW(spi)->INTR_ENABLE;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_ENABLE_SPI_EVENT_BIT, enable);
    SPI_HOST_HW(spi)->INTR_ENABLE = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_error_intr(spi_host_t* spi, bool enable) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = SPI_HOST_HW(spi)->INTR_ENABLE;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_STATE_ERROR_BIT, enable);
    SPI_HOST_HW(spi)->INTR_ENABLE = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_output_enable(spi_host_t* spi, bool enable){
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t output_enable_reg = SPI_HOST_HW(spi)->CONTROL;
    output_enable_reg = bitfield_write(output_enable_reg, BIT_MASK_1, SPI_HOST_CONTROL_OUTPUT_EN_BIT, enable);
    SPI_HOST_HW(spi)->CONTROL = output_enable_reg;
    return SPI_FLAG_OK;
}

void handler_irq_spi(uint32_t id)
{
    spi_error_e errors;
    spi_get_errors(spi_host2, &errors);
    if (errors) {
        spi_intr_handler_error_host2(errors);
        spi_acknowledge_errors(spi_host2);
    }
    else {
        spi_event_e events;
        spi_acknowledge_event(spi_host2);
        spi_get_events(spi_host2, &events);
        spi_intr_handler_event_host2(events);
    }
}

void fic_irq_spi(void)
{
    spi_error_e errors;
    spi_get_errors(spi_host1, &errors);
    if (errors) {
        spi_intr_handler_error_host(errors);
        spi_acknowledge_errors(spi_host1);
    }
    else {
        spi_event_e events;
        spi_acknowledge_event(spi_host1);
        spi_get_events(spi_host1, &events);
        spi_intr_handler_event_host(events);
    }
}

void fic_irq_spi_flash(void)
{
    spi_error_e errors;
    spi_get_errors(spi_flash, &errors);
    if (errors) {
        spi_intr_handler_error_flash(errors);
        spi_acknowledge_errors(spi_flash);
    }
    else {
        spi_event_e events;
        spi_acknowledge_event(spi_flash);
        spi_get_events(spi_flash, &events);
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

spi_return_flags_e spi_get_events(spi_host_t* spi, spi_event_e* events) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile spi_status_t* status = spi_get_status(spi);
    *events = bitfield_write(*events, BIT_MASK_1, SPI_HOST_EVENT_ENABLE_RXFULL_BIT, status->rxfull);
    *events = bitfield_write(*events, BIT_MASK_1, SPI_HOST_EVENT_ENABLE_TXEMPTY_BIT, status->txempty);
    *events = bitfield_write(*events, BIT_MASK_1, SPI_HOST_EVENT_ENABLE_RXWM_BIT, status->rxwm);
    *events = bitfield_write(*events, BIT_MASK_1, SPI_HOST_EVENT_ENABLE_TXWM_BIT, status->txwm);
    *events = bitfield_write(*events, BIT_MASK_1, SPI_HOST_EVENT_ENABLE_READY_BIT, status->ready);
    *events = bitfield_write(*events, BIT_MASK_1, SPI_HOST_EVENT_ENABLE_IDLE_BIT, ~status->active);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_acknowledge_event(spi_host_t* spi) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    SPI_HOST_HW(spi)->INTR_STATE = bitfield_write(SPI_HOST_HW(spi)->INTR_STATE, BIT_MASK_1, SPI_HOST_INTR_STATE_SPI_EVENT_BIT, 1);
    return SPI_FLAG_OK;
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
