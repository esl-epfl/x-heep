/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************
**
** project  : X-HEEP
** filename : spi_sdk.c
** version  : 1
** date     : 18/04/24
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
* @file   spi_sdk.c
* @date   18/04/24
* @brief  The Serial Peripheral Interface (SPI) SDK to set up and use the
* SPI peripheral
*/

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include "spi_sdk.h"
#include "spi_host.h"
#include "soc_ctrl_structs.h"
#include "bitfield.h"

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

#define DATA_MODE_CPOL_OFFS 1
#define DATA_MODE_CPHA_OFFS 0

#define MAX_COMMAND_LENGTH 16777215 // 2^24 - 1 (in bytes)
#define BYTES_PER_WORD     4

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

typedef struct {
    spi_host_t instance;
    volatile bool busy;
    spi_transaction_t txn;
    uint32_t scnt;
    uint32_t wcnt;
    uint32_t rcnt;
    spi_cb_t evt_cb;
    spi_cb_t err_cb;
} spi_peripheral_t;

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

spi_codes_e spi_check_valid(spi_t* spi);
spi_codes_e spi_validate_slave(spi_slave_t slave);
spi_codes_e spi_set_slave(spi_t* spi);
spi_codes_e spi_prepare_for_xfer(spi_t* spi, uint32_t len);
void spi_fill_tx(spi_peripheral_t* peri);
void spi_empty_rx(spi_peripheral_t* peri);
void spi_reset_peri(spi_peripheral_t* peri);
void spi_event_handler(spi_peripheral_t* peri, spi_event_e events);

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                            GLOBAL VARIABLES                              */
/**                                                                        **/
/****************************************************************************/

static spi_peripheral_t _peripherals[] = {
    (spi_peripheral_t) {
        .instance  = SPI_FLASH_INIT,
        .busy      = false,
        .txn = {0},
        .scnt = 0,
        .wcnt = 0,
        .rcnt = 0,
        .evt_cb = NULL,
        .err_cb = NULL
    },
    (spi_peripheral_t) {
        .instance  = SPI_HOST_INIT,
        .busy      = false,
        .txn = {0},
        .scnt = 0,
        .wcnt = 0,
        .rcnt = 0,
        .evt_cb = NULL,
        .err_cb = NULL
    },
    (spi_peripheral_t) {
        .instance  = SPI_HOST2_INIT,
        .busy      = false,
        .txn = {0},
        .scnt = 0,
        .wcnt = 0,
        .rcnt = 0,
        .evt_cb = NULL,
        .err_cb = NULL
    }
};

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

spi_t spi_init(spi_idx_e idx, spi_slave_t slave) {
    spi_codes_e error = spi_validate_slave(slave);
    if (SPI_CSID_INVALID(idx)) error |= SPI_CODE_IDX_INVAL;
    if (error)
        return (spi_t) {
            .idx   = -1,
            .init  = false,
            .slave = slave
        };
    // NOTE: Not a bad idea: set idx to -1, init to false, and slave make pointer and
    // set to NULL. Advantage of pointer we can have NULL and we may have multiple
    // slaves, disadvantage user must keep the original version somewhere, but this
    // shouldn't be a major drawback since slaves should be configured at compile
    // time, hence they may just keep a global variable somewhere etc.
    spi_set_enable(_peripherals[idx].instance, true);
    spi_output_enable(_peripherals[idx].instance, true);
    return (spi_t) {
        .idx   = idx,
        .init  = true,
        .slave = slave
    };
}

spi_codes_e spi_deinit(spi_t* spi) {
    return SPI_CODE_OK;
}

spi_codes_e spi_reset(spi_t* spi) {
    spi_codes_e error = spi_check_valid(spi);
    if (error) return error;

    if (spi_sw_reset(_peripherals[spi->idx].instance)) return SPI_CODE_BASE_ERROR;

    return SPI_CODE_OK;
}

spi_codes_e spi_get_slave(spi_t* spi, uint8_t csid, spi_slave_t* slave) {
    spi_codes_e error = spi_check_valid(spi);
    if (error) return error;
    if (SPI_CSID_INVALID(csid)) return SPI_CODE_SLAVE_CSID_INVAL;

    uint32_t config_reg;
    if (spi_get_configopts(_peripherals[spi->idx].instance, csid, &config_reg)) return SPI_CODE_BASE_ERROR;
    spi_configopts_t configopts = spi_create_configopts_structure(config_reg);
    slave->csid       = csid;
    slave->csn_idle   = configopts.csnidle;
    slave->csn_lead   = configopts.csnlead;
    slave->csn_trail  = configopts.csntrail;
    slave->full_cycle = configopts.fullcyc;
    slave->data_mode  = (configopts.cpol << DATA_MODE_CPOL_OFFS)
                      | (configopts.cpha << DATA_MODE_CPHA_OFFS);
    slave->freq       = soc_ctrl_peri->SYSTEM_FREQUENCY_HZ / (2 * (configopts.clkdiv + 1));

    return SPI_CODE_OK;
}

spi_codes_e spi_transmit(spi_t* spi, const uint32_t* src_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_for_xfer(spi, len);
    if (error) return error;

    spi_transaction_t txn = {0};
    txn.txbuffer = src_buffer;
    txn.txlen    = len;

    _peripherals[spi->idx].busy = true;
    _peripherals[spi->idx].txn  = txn;

    spi_fill_tx(&_peripherals[spi->idx]);

    spi_set_events_enabled(_peripherals[spi->idx].instance, SPI_EVENT_IDLE | SPI_EVENT_TXWM, true);
    spi_enable_evt_intr(_peripherals[spi->idx].instance, true);

    // Issue command
    uint32_t cmd = spi_create_command((spi_command_t) {
        .direction = SPI_DIR_TX_ONLY,
        .speed     = SPI_SPEED_STANDARD,
        .csaat     = 0,
        .len       = len * BYTES_PER_WORD - 1
    });
    spi_set_command(_peripherals[spi->idx].instance, cmd); // TODO: Verify this doesn't return error...

    while (_peripherals[spi->idx].busy);

    return SPI_CODE_OK;
}

spi_codes_e spi_receive(spi_t* spi, uint32_t* dest_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_for_xfer(spi, len);
    if (error) return error;

    spi_transaction_t txn = {0};
    txn.rxbuffer = dest_buffer;
    txn.rxlen    = len;

    _peripherals[spi->idx].busy = true;
    _peripherals[spi->idx].txn  = txn;

    spi_set_events_enabled(_peripherals[spi->idx].instance, SPI_EVENT_IDLE | SPI_EVENT_RXWM, true);
    spi_enable_evt_intr(_peripherals[spi->idx].instance, true);

    // Issue command
    uint32_t cmd = spi_create_command((spi_command_t) {
        .direction = SPI_DIR_RX_ONLY,
        .speed     = SPI_SPEED_STANDARD,
        .csaat     = 0,
        .len       = len * BYTES_PER_WORD - 1
    });
    spi_set_command(_peripherals[spi->idx].instance, cmd); // TODO: Verify this doesn't return error...

    while (_peripherals[spi->idx].busy);

    return SPI_CODE_OK;
}

spi_codes_e spi_transceive(spi_t* spi, const uint32_t* src_buffer, uint32_t* dest_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_for_xfer(spi, len);
    if (error) return error;

    spi_transaction_t txn = {
        .segments = NULL,
        .seglen   = 0,
        .txbuffer = src_buffer,
        .txlen    = len,
        .rxbuffer = dest_buffer,
        .rxlen    = len
    };
    _peripherals[spi->idx].busy = true;
    _peripherals[spi->idx].txn  = txn;

    spi_fill_tx(&_peripherals[spi->idx]);

    spi_set_events_enabled(_peripherals[spi->idx].instance, SPI_EVENT_IDLE | SPI_EVENT_TXWM | SPI_EVENT_RXWM, true);
    spi_enable_evt_intr(_peripherals[spi->idx].instance, true);

    // Issue command
    uint32_t cmd = spi_create_command((spi_command_t) {
        .direction = SPI_DIR_BIDIR,
        .speed     = SPI_SPEED_STANDARD,
        .csaat     = 0,
        .len       = len * BYTES_PER_WORD - 1
    });
    spi_set_command(_peripherals[spi->idx].instance, cmd); // TODO: Verify this doesn't return error...

    while (_peripherals[spi->idx].busy);

    return SPI_CODE_OK;
}

spi_codes_e spi_execute(spi_t* spi, spi_transaction_t transaction) {
    // TODO: replace this 1, good thing is we don't care about len because segments already limit length
    spi_codes_e error = spi_prepare_for_xfer(spi, 1);
    if (error) return error;

    // Validate all segments before commencing with transfer
    for (int i = 0; i < transaction.seglen; i++)
    {
        uint8_t direction = bitfield_read(transaction.segments[i].mode, BIT_MASK_2, 2);
        uint8_t speed     = bitfield_read(transaction.segments[i].mode, BIT_MASK_2, 0);
        if (!spi_validate_cmd(direction, speed)) return SPI_CODE_SEGMENT_INVAL;
    }
    
    _peripherals[spi->idx].busy = true;
    _peripherals[spi->idx].txn  = transaction;

    spi_fill_tx(&_peripherals[spi->idx]);

    spi_set_events_enabled(_peripherals[spi->idx].instance, SPI_EVENT_IDLE | SPI_EVENT_READY | SPI_EVENT_TXWM | SPI_EVENT_RXWM, true);
    spi_enable_evt_intr(_peripherals[spi->idx].instance, true);

    uint32_t cmd_reg = spi_create_command((spi_command_t) {
        .direction = bitfield_read(transaction.segments[0].mode, BIT_MASK_2, 2),
        .speed     = bitfield_read(transaction.segments[0].mode, BIT_MASK_2, 0),
        .csaat     = 0 < transaction.seglen - 1 ? 1 : 0,
        .len       = transaction.segments[0].len
    });
    spi_set_command(_peripherals[spi->idx].instance, cmd_reg);
    _peripherals[spi->idx].scnt++;

    while (_peripherals[spi->idx].busy);

    return SPI_CODE_OK;
}

/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

spi_codes_e spi_check_valid(spi_t* spi) {
    if (SPI_IDX_INVALID(spi->idx)) return SPI_CODE_IDX_INVAL;
    if (!spi->init)                return SPI_CODE_NOT_INIT;
    return SPI_CODE_OK;
}

spi_codes_e spi_validate_slave(spi_slave_t slave) {
    spi_codes_e error = SPI_CODE_OK;
    if (SPI_CSID_INVALID(slave.csid)) error |= SPI_CODE_SLAVE_CSID_INVAL;
    if (slave.freq > soc_ctrl_peri->SYSTEM_FREQUENCY_HZ / 2) error |= SPI_CODE_SLAVE_FREQ_INVAL;
    return error;
}

spi_codes_e spi_set_slave(spi_t* spi) {
    if (spi_get_active(_peripherals[spi->idx].instance) == SPI_TRISTATE_TRUE) return SPI_CODE_NOT_IDLE;

    uint16_t clk_div = soc_ctrl_peri->SYSTEM_FREQUENCY_HZ / (2 * spi->slave.freq) - 1;
    if (soc_ctrl_peri->SYSTEM_FREQUENCY_HZ / (2 * (clk_div + 1)) > spi->slave.freq)
        clk_div++;
    spi_configopts_t config = {
        .clkdiv   = clk_div,
        .cpha     = bitfield_read(spi->slave.data_mode, BIT_MASK_1, DATA_MODE_CPHA_OFFS),
        .cpol     = bitfield_read(spi->slave.data_mode, BIT_MASK_1, DATA_MODE_CPOL_OFFS),
        .csnidle  = spi->slave.csn_idle,
        .csnlead  = spi->slave.csn_lead,
        .csntrail = spi->slave.csn_trail,
        .fullcyc  = spi->slave.full_cycle
    };
    spi_return_flags_e config_error = spi_set_configopts(_peripherals[spi->idx].instance, 
                                                         spi->slave.csid,
                                                         spi_create_configopts(config)
                                                        );
    if (config_error) return SPI_CODE_BASE_ERROR;
    return SPI_CODE_OK;
}

spi_codes_e spi_prepare_for_xfer(spi_t* spi, uint32_t len) {
    spi_codes_e error = spi_check_valid(spi) | spi_validate_slave(spi->slave);
    if (error) return error;
    if (len == 0) return SPI_CODE_OK; // TODO: must return error
    if (len * BYTES_PER_WORD - 1 > MAX_COMMAND_LENGTH) return SPI_CODE_OK; // TODO: must return error

    spi_wait_for_idle(_peripherals[spi->idx].instance);
    spi_set_tx_watermark(_peripherals[spi->idx].instance, SPI_HOST_PARAM_TX_DEPTH / 4);
    spi_set_rx_watermark(_peripherals[spi->idx].instance, SPI_HOST_PARAM_RX_DEPTH - 12);

    error = spi_set_slave(spi);
    if (error) return error;

    return SPI_CODE_OK;
}

void spi_fill_tx(spi_peripheral_t* peri) {
    if (peri->txn.txbuffer != NULL && peri->wcnt < peri->txn.txlen)
    {
        while (
            peri->wcnt < peri->txn.txlen 
            && !spi_write_word(peri->instance, peri->txn.txbuffer[peri->wcnt])
        ) 
        peri->wcnt++;
    }
}

void spi_empty_rx(spi_peripheral_t* peri) {
    if (peri->txn.rxbuffer != NULL && peri->rcnt < peri->txn.rxlen)
    {
        while (
            peri->rcnt < peri->txn.rxlen 
            && !spi_read_word(peri->instance, &peri->txn.rxbuffer[peri->rcnt])
        ) 
        peri->rcnt++;
    }
}

void spi_reset_peri(spi_peripheral_t* peri) {
    peri->busy   = false;
    peri->scnt   = 0;
    peri->wcnt   = 0;
    peri->rcnt   = 0;
    peri->txn    = (spi_transaction_t) {0};
    peri->evt_cb = NULL;
    peri->err_cb = NULL;
}

void spi_event_handler(spi_peripheral_t* peri, spi_event_e events) {
    switch (events)
    {
    case SPI_EVENT_READY:
        if (peri->txn.segments != NULL && peri->scnt < peri->txn.seglen)
        {
            uint32_t cmd_reg = spi_create_command((spi_command_t) {
                .direction = bitfield_read(peri->txn.segments[peri->scnt].mode, BIT_MASK_2, 2),
                .speed     = bitfield_read(peri->txn.segments[peri->scnt].mode, BIT_MASK_2, 0),
                .csaat     = peri->scnt < peri->txn.seglen - 1 ? 1 : 0,
                .len       = peri->txn.segments[peri->scnt].len
            });
            spi_set_command(peri->instance, cmd_reg);
            peri->scnt++;
        }
        break;
    
    case SPI_EVENT_IDLE:
        spi_set_events_enabled(peri->instance, SPI_EVENT_ALL, false);
        spi_enable_evt_intr(peri->instance, false);
        spi_empty_rx(peri);
        if (peri->evt_cb != NULL) peri->evt_cb();
        spi_reset_peri(peri);
        break;
    
    case SPI_EVENT_TXWM:
        spi_fill_tx(peri);
        break;
    
    case SPI_EVENT_RXWM:
        spi_empty_rx(peri);
        break;
    
    default:
        break;
    }
}

/****************************************************************************/
/**                                                                        **/
/*                               INTERRUPTS                                 */
/**                                                                        **/
/****************************************************************************/

void spi_intr_handler_event_flash(spi_event_e events) {
    if (!_peripherals[SPI_IDX_FLASH].busy) return;
    spi_event_handler(&_peripherals[SPI_IDX_FLASH], events);
}

void spi_intr_handler_error_flash(spi_error_e errors) {

}

void spi_intr_handler_event_host(spi_event_e events) {
    if (!_peripherals[SPI_IDX_HOST].busy) return;
    spi_event_handler(&_peripherals[SPI_IDX_HOST], events);
}

void spi_intr_handler_error_host(spi_error_e errors) {

}

void spi_intr_handler_event_host2(spi_event_e events) {
    if (!_peripherals[SPI_IDX_HOST_2].busy) return;
    spi_event_handler(&_peripherals[SPI_IDX_HOST_2], events);
}

void spi_intr_handler_error_host2(spi_error_e errors) {

}

/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
