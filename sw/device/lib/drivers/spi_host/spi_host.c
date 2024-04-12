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

#define SPI_CLEAR_ERRORS_STATE_VALUE (1 << SPI_ERRORS_MASK + 1) - 1

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

spi_return_flags_e spi_get_events(spi_host_t* spi, spi_event_e* events);

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED VARIABLES                             */
/**                                                                        **/
/****************************************************************************/

// volatile spi_host* const spi_peris[4] = {
//     ((volatile spi_host *) SPI_FLASH_START_ADDRESS),
//     ((volatile spi_host *) SPI_MEMIO_START_ADDRESS),
//     ((volatile spi_host *) SPI_HOST_START_ADDRESS),
//     ((volatile spi_host *) SPI2_START_ADDRESS)
// };

/****************************************************************************/
/**                                                                        **/
/*                            GLOBAL VARIABLES                              */
/**                                                                        **/
/****************************************************************************/

static spi_host_t _spi_flash = {
    .peri       = ((volatile spi_host *) SPI_FLASH_START_ADDRESS),
    .base_addr  = SPI_FLASH_START_ADDRESS,
    .is_init    = false
};

static spi_host_t _spi_host = {
    .peri       = ((volatile spi_host *) SPI_FLASH_START_ADDRESS),
    .base_addr  = SPI_FLASH_START_ADDRESS,
    .is_init    = false
};

static spi_host_t _spi_host2 = {
    .peri       = ((volatile spi_host *) SPI_FLASH_START_ADDRESS),
    .base_addr  = SPI_FLASH_START_ADDRESS,
    .is_init    = false
};

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/


spi_host_t* spi_init_flash(bool output_en) {
    if (_spi_flash.is_init) return &_spi_flash;
    spi_sw_reset(&_spi_flash);
    spi_set_enable(&_spi_flash, true);
    spi_output_enable(&_spi_flash, output_en);
    _spi_flash.is_init = true;
    return &_spi_flash;
}

spi_host_t* spi_init_host(bool output_en) {
    if (_spi_host.is_init) return &_spi_host;
    spi_sw_reset(&_spi_host);
    spi_set_enable(&_spi_host, true);
    spi_output_enable(&_spi_host, output_en);
    _spi_host.is_init = true;
    return &_spi_host;
}

spi_host_t* spi_init_host2(bool output_en) {
    if (_spi_host2.is_init) return &_spi_host2;
    spi_sw_reset(&_spi_host2);
    spi_set_enable(&_spi_host2, true);
    spi_output_enable(&_spi_host2, output_en);
    _spi_host2.is_init = true;
    return &_spi_host2;
}

spi_return_flags_e spi_get_events_enabled(spi_host_t* spi, spi_event_e* events) 
{
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    *events = bitfield_read(spi->peri->EVENT_ENABLE, SPI_EVENTS_MASK, SPI_EVENTS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_events_enabled(spi_host_t* spi, spi_event_e* events, bool enable) 
{
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    if (*events > SPI_EVENT_ALL) return SPI_FLAG_EVENT_INVALID;
    if (enable) spi->peri->EVENT_ENABLE |= *events;
    else        spi->peri->EVENT_ENABLE &= ~*events;

    *events = bitfield_read(spi->peri->EVENT_ENABLE, SPI_EVENTS_MASK, SPI_EVENTS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_get_errors_enabled(spi_host_t* spi, spi_error_e* errors) 
{
    if (spi == NULL) return SPI_FLAG_NULL_PTR;

    *errors = bitfield_read(spi->peri->ERROR_ENABLE, SPI_ERRORS_IRQ_MASK, SPI_ERRORS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_errors_enabled(spi_host_t* spi, spi_error_e* errors, bool enable)
{
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    if (*errors > SPI_ERROR_IRQALL) return SPI_FLAG_ERROR_INVALID;
    if (enable) spi->peri->ERROR_ENABLE |= *errors;
    else        spi->peri->ERROR_ENABLE &= ~*errors;

    *errors = bitfield_read(spi->peri->ERROR_ENABLE, SPI_ERRORS_IRQ_MASK, SPI_ERRORS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_get_errors(spi_host_t* spi, spi_error_e* errors)
{
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    *errors = bitfield_read(spi->peri->ERROR_STATUS, SPI_ERRORS_MASK, SPI_ERRORS_INDEX);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_acknowledge_errors(spi_host_t* spi) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    bitfield_write(spi->peri->ERROR_STATUS, SPI_ERRORS_MASK, SPI_ERRORS_INDEX, SPI_CLEAR_ERRORS_STATE_VALUE);
    bitfield_write(spi->peri->INTR_STATE, BIT_MASK_1, SPI_HOST_INTR_STATE_ERROR_BIT, 1);
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_write_byte(spi_host_t* spi, uint8_t bdata) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    if (spi_get_tx_queue_depth(spi) >= SPI_HOST_PARAM_TX_DEPTH) return SPI_FLAG_TX_QUEUE_FULL;
    spi->peri->TXDATA = bdata;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_error_intr_test(spi_host_t* spi, bool enable) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi->peri->INTR_TEST;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_TEST_ERROR_BIT, enable);
    spi->peri->INTR_TEST = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_evt_intr_test(spi_host_t* spi, bool enable) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi->peri->INTR_TEST;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_TEST_SPI_EVENT_BIT, enable);
    spi->peri->INTR_TEST = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_alert_test_fatal_fault_trigger(spi_host_t* spi) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi->peri->ALERT_TEST;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_ALERT_TEST_FATAL_FAULT_BIT, true);
    spi->peri->ALERT_TEST = intr_enable_reg;
    return SPI_FLAG_OK;
}

// SPI get functions

// return max(uint8_t) on NULL pointer
volatile uint8_t spi_get_tx_queue_depth(spi_host_t* spi) {
    if (spi == NULL) return -1;
    return spi_get_status(spi)->txqd;
}

volatile spi_ch_status_t spi_get_tx_channel_status(spi_host_t* spi) {
    // TODO: Find a good idea to return error flag if spi == NULL
    volatile spi_status_t* status = spi_get_status(spi);
    spi_ch_status_t ch_status = {
        .empty  = status->txempty,
        .full   = status->txfull,
        .stall  = status->txstall,
        .wm     = status->txwm
    };
    return ch_status;
}

// return max(uint8_t) on NULL pointer
volatile uint8_t spi_get_rx_queue_depth(spi_host_t* spi) {
    if (spi == NULL) return -1;
    return spi_get_status(spi)->rxqd;
}

volatile spi_ch_status_t spi_get_rx_channel_status(spi_host_t* spi) {
    // TODO: Find a good idea to return error flag if spi == NULL
    volatile spi_status_t* status = spi_get_status(spi);
    spi_ch_status_t ch_status = {
        .empty  = status->rxempty,
        .full   = status->rxfull,
        .stall  = status->rxstall,
        .wm     = status->rxwm
    };
    return ch_status;
}

// return max(uint32_t) on NULL pointer
volatile uint32_t spi_get_csid(spi_host_t* spi) {
    if (spi == NULL) return -1;
    return spi->peri->CSID;
}

// SPI set functions

spi_return_flags_e spi_sw_reset(spi_host_t* spi) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t ctrl_reg = spi->peri->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, BIT_MASK_1, SPI_HOST_CONTROL_SW_RST_BIT, 1);
    spi->peri->CONTROL = ctrl_reg;
    volatile spi_status_t* status = spi_get_status(spi);
    while (status->active | status->txqd | status->rxqd);
    // ctrl_reg = spi->peri->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, BIT_MASK_1, SPI_HOST_CONTROL_SW_RST_BIT, 0);
    spi->peri->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_enable(spi_host_t* spi, bool enable) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t ctrl_reg = spi->peri->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, BIT_MASK_1, SPI_HOST_CONTROL_SPIEN_BIT, enable);
    spi->peri->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_tx_watermark(spi_host_t* spi, uint8_t watermark) {
    spi_return_flags_e flags = SPI_FLAG_OK;
    if (spi == NULL) flags += SPI_FLAG_NULL_PTR;
    if (watermark > SPI_HOST_PARAM_TX_DEPTH) flags += SPI_FLAG_WATERMARK_EXCEEDS;
    if (flags) return flags;

    volatile uint32_t ctrl_reg = spi->peri->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, SPI_HOST_CONTROL_TX_WATERMARK_MASK, SPI_HOST_CONTROL_TX_WATERMARK_OFFSET, watermark);
    spi->peri->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_rx_watermark(spi_host_t* spi, uint8_t watermark) {
    spi_return_flags_e flags = SPI_FLAG_OK;
    if (spi == NULL) flags += SPI_FLAG_NULL_PTR;
    if (watermark > SPI_HOST_PARAM_RX_DEPTH) flags += SPI_FLAG_WATERMARK_EXCEEDS;
    if (flags) return flags;

    volatile uint32_t ctrl_reg = spi->peri->CONTROL;
    ctrl_reg = bitfield_write(ctrl_reg, SPI_HOST_CONTROL_RX_WATERMARK_MASK, SPI_HOST_CONTROL_RX_WATERMARK_OFFSET, watermark);
    spi->peri->CONTROL = ctrl_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_configopts(spi_host_t* spi, uint32_t csid, const uint32_t conf_reg) {
    // TODO: check if this could be generalized to more than 2 CSIDs... because right 
    // now not very consistent with spi_set_csid which uses SPI_HOST_PARAM_NUM_C_S
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    switch (csid)
    {
    case 0:
        spi->peri->CONFIGOPTS0 = conf_reg;
        break;
    case 1:
        spi->peri->CONFIGOPTS1 = conf_reg;
        break;
    default:
        return SPI_FLAG_CSID_INVALID;
        break;
    }
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_csid(spi_host_t* spi, uint32_t csid) {
    spi_return_flags_e flags = SPI_FLAG_OK;
    if (spi == NULL) flags += SPI_FLAG_NULL_PTR;
    if (SPI_CSID_INVALID(csid)) flags += SPI_FLAG_CSID_INVALID;
    if (flags) return flags;

    spi->peri->CSID = csid;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_set_command(spi_host_t* spi, const uint32_t cmd_reg) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;

    spi_return_flags_e flags = SPI_FLAG_OK;
    spi_speed_e speed = bitfield_read(cmd_reg, SPI_HOST_COMMAND_SPEED_MASK, SPI_HOST_COMMAND_SPEED_OFFSET);
    spi_dir_e direction = bitfield_read(cmd_reg, SPI_HOST_COMMAND_DIRECTION_MASK, SPI_HOST_COMMAND_DIRECTION_OFFSET);

    if (spi_get_status(spi)->cmdqd >= SPI_HOST_PARAM_CMD_DEPTH)
        flags += SPI_FLAG_COMMAND_FULL;
    if (speed > SPI_SPEED_QUAD || (direction == SPI_DIR_BIDIR && speed != SPI_SPEED_STANDARD))
        flags += SPI_FLAG_SPEED_INVALID;
    if (!spi_get_ready(spi)) flags += SPI_FLAG_NOT_READY;
    if (flags) return flags;

    spi->peri->COMMAND = cmd_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_write_word(spi_host_t* spi, uint32_t wdata) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    if (spi_get_tx_queue_depth(spi) >= SPI_HOST_PARAM_TX_DEPTH) return SPI_FLAG_TX_QUEUE_FULL;
    spi->peri->TXDATA = wdata;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_read_word(spi_host_t* spi, uint32_t* dst) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    if (spi_get_rx_queue_depth(spi) == 0) return SPI_FLAG_RX_QUEUE_EMPTY;
    *dst = spi->peri->RXDATA;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_evt_intr(spi_host_t* spi, bool enable) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi->peri->INTR_ENABLE;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_ENABLE_SPI_EVENT_BIT, enable);
    spi->peri->INTR_ENABLE = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_enable_error_intr(spi_host_t* spi, bool enable) {
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t intr_enable_reg = spi->peri->INTR_ENABLE;
    intr_enable_reg = bitfield_write(intr_enable_reg, BIT_MASK_1, SPI_HOST_INTR_STATE_ERROR_BIT, enable);
    spi->peri->INTR_ENABLE = intr_enable_reg;
    return SPI_FLAG_OK;
}

spi_return_flags_e spi_output_enable(spi_host_t* spi, bool enable){
    if (spi == NULL) return SPI_FLAG_NULL_PTR;
    volatile uint32_t output_enable_reg = spi->peri->CONTROL;
    output_enable_reg = bitfield_write(output_enable_reg, BIT_MASK_1, SPI_HOST_CONTROL_OUTPUT_EN_BIT, enable);
    spi->peri->CONTROL = output_enable_reg;
    return SPI_FLAG_OK;
}

void handler_irq_spi(uint32_t id)
{
    if (!_spi_host2.is_init) return;
    spi_error_e errors;
    spi_get_errors(&_spi_host2, &errors);
    if (errors) {
        spi_intr_handler_error_host2(errors);
        spi_acknowledge_errors(&_spi_host2);
    }
    else {
        spi_event_e events;
        spi_get_events(&_spi_host2, &events);
        spi_intr_handler_event_host2(events);
    }
}

void fic_irq_spi(void)
{
    if (!_spi_host.is_init) return;
    spi_error_e errors;
    spi_get_errors(&_spi_host, &errors);
    if (errors) {
        spi_intr_handler_error_host(errors);
        spi_acknowledge_errors(&_spi_host);
    }
    else {
        spi_event_e events;
        spi_get_events(&_spi_host, &events);
        spi_intr_handler_event_host(events);
    }
}

void fic_irq_spi_flash(void)
{
    if (!_spi_flash.is_init) return;
    spi_error_e errors;
    spi_get_errors(&_spi_flash, &errors);
    if (errors) {
        spi_intr_handler_error_flash(errors);
        spi_acknowledge_errors(&_spi_flash);
    }
    else {
        spi_event_e events;
        spi_get_events(&_spi_flash, &events);
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
    *events = status->rxfull  << SPI_HOST_EVENT_ENABLE_RXFULL_BIT
            | status->txempty << SPI_HOST_EVENT_ENABLE_TXEMPTY_BIT
            | status->rxwm    << SPI_HOST_EVENT_ENABLE_RXWM_BIT
            | status->txwm    << SPI_HOST_EVENT_ENABLE_TXWM_BIT
            | status->ready   << SPI_HOST_EVENT_ENABLE_READY_BIT
            | ~status->active << SPI_HOST_EVENT_ENABLE_IDLE_BIT;
    return SPI_FLAG_OK;
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
