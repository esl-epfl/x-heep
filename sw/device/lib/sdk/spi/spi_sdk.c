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
#include "spi_host_regs.h"
#include "soc_ctrl_structs.h"
#include "bitfield.h"
#include "fast_intr_ctrl.h"
#include "hart.h"
#include "csr.h"
#include "csr_registers.h"

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

#define DATA_MODE_CPOL_OFFS 1
#define DATA_MODE_CPHA_OFFS 0

#define MAX_COMMAND_LENGTH SPI_HOST_COMMAND_LEN_MASK
// #define BYTES_PER_WORD     SPI_HOST_PARAM_REG_WIDTH / sizeof(uint8_t)
#define BYTES_PER_WORD     4

#define TX_WATERMARK SPI_HOST_PARAM_TX_DEPTH / 4
#define RX_WATERMARK SPI_HOST_PARAM_RX_DEPTH - 12

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

typedef enum {
    SPI_PERI_FLAG_NONE          = 0,
    SPI_PERI_FLAG_BUSY          = 1,
    SPI_PERI_FLAG_CMD_NOT_DONE      = 2,
    SPI_PERI_FLAG_WRITE_NOT_DONE    = 4
} spi_peri_flags_e;

typedef struct {
    const spi_segment_t* segments;
    uint8_t              seglen;
    const uint32_t*      txbuffer;
    uint32_t             txlen;
    uint32_t*            rxbuffer;
    uint32_t             rxlen;
} spi_transaction_t;

typedef struct {
    spi_host_t*       instance;
    bool              busy;
    spi_peri_flags_e  flags;
    spi_transaction_t txn;
    uint32_t          scnt;
    uint32_t          wcnt;
    uint32_t          rcnt;
    spi_cb_t          evt_cb;
    spi_cb_t          err_cb;
} spi_peripheral_t;

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

spi_codes_e spi_check_valid(spi_t* spi);
spi_codes_e spi_validate_slave(spi_slave_t slave);
spi_codes_e spi_set_slave(spi_t* spi);
spi_codes_e spi_prepare_for_xfer(spi_t* spi);
void spi_fill_tx(spi_peripheral_t* peri);
void spi_empty_rx(spi_peripheral_t* peri);
void spi_launch(spi_peripheral_t* peri, spi_transaction_t txn);
void spi_issue_cmd(const spi_peripheral_t* peri, spi_segment_t seg, bool csaat);
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

static volatile spi_peripheral_t _peripherals[] = {
    (spi_peripheral_t) {
        .instance  = spi_flash,
        .busy      = false,
        .flags     = SPI_PERI_FLAG_NONE,
        .txn       = {0},
        .scnt      = 0,
        .wcnt      = 0,
        .rcnt      = 0,
        .evt_cb    = NULL,
        .err_cb    = NULL
    },
    (spi_peripheral_t) {
        .instance  = spi_host1,
        .busy      = false,
        .flags     = SPI_PERI_FLAG_NONE,
        .txn       = {0},
        .scnt      = 0,
        .wcnt      = 0,
        .rcnt      = 0,
        .evt_cb    = NULL,
        .err_cb    = NULL
    },
    (spi_peripheral_t) {
        .instance  = spi_host2,
        .busy      = false,
        .flags     = SPI_PERI_FLAG_NONE,
        .txn       = {0},
        .scnt      = 0,
        .wcnt      = 0,
        .rcnt      = 0,
        .evt_cb    = NULL,
        .err_cb    = NULL
    }
};

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

spi_t spi_init(spi_idx_e idx, spi_slave_t slave) {
    spi_codes_e error = spi_validate_slave(slave);
    if (SPI_IDX_INVALID(idx)) error |= SPI_CODE_IDX_INVAL;
    if (error)
        return (spi_t) {
            .idx   = -1,
            .init  = false,
            .slave = {0}
        };
    spi_set_enable(_peripherals[idx].instance, true);
    spi_output_enable(_peripherals[idx].instance, true);
    spi_set_errors_enabled(_peripherals[idx].instance, SPI_ERROR_IRQALL, true);
    return (spi_t) {
        .idx   = idx,
        .init  = true,
        .slave = slave
    };
}

spi_codes_e spi_deinit(spi_t* spi) {
    spi->idx   = -1;
    spi->init  = false;
    spi->slave = (spi_slave_t) {0};
    return SPI_CODE_OK;
}

spi_codes_e spi_reset(spi_t* spi) {
    spi_codes_e error = spi_check_valid(spi);
    if (error) return error;

    spi_sw_reset(_peripherals[spi->idx].instance);

    return SPI_CODE_OK;
}

spi_codes_e spi_is_busy(spi_t* spi) {
    spi_codes_e error = spi_check_valid(spi);
    if (error) return error;

    return _peripherals[spi->idx].busy ? SPI_CODE_IS_BUSY : SPI_CODE_OK;
}

spi_codes_e spi_transmit(spi_t* spi, const uint32_t* src_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_for_xfer(spi);
    if (len > MAX_COMMAND_LENGTH) error |= SPI_CODE_TXN_LEN_INVAL;
    if (error) return error;

    spi_segment_t seg = SPI_SEG_TX(len);

    spi_transaction_t txn = {
        .segments = &seg,
        .seglen   = 1,
        .txbuffer = src_buffer,
        .txlen    = (len / BYTES_PER_WORD) + (len % BYTES_PER_WORD ? 1 : 0),
        .rxbuffer = NULL,
        .rxlen    = 0
    };

    spi_launch(&_peripherals[spi->idx], txn);

    while (_peripherals[spi->idx].busy) wait_for_interrupt();

    return SPI_CODE_OK;
}

spi_codes_e spi_receive(spi_t* spi, uint32_t* dest_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_for_xfer(spi);
    if (len > MAX_COMMAND_LENGTH) error |= SPI_CODE_TXN_LEN_INVAL;
    if (error) return error;

    spi_segment_t seg = SPI_SEG_RX(len);

    spi_transaction_t txn = {
        .segments = &seg,
        .seglen   = 1,
        .txbuffer = NULL,
        .txlen    = 0,
        .rxbuffer = dest_buffer,
        .rxlen    = (len / BYTES_PER_WORD) + (len % BYTES_PER_WORD ? 1 : 0)
    };

    spi_launch(&_peripherals[spi->idx], txn);

    while (_peripherals[spi->idx].busy) wait_for_interrupt();

    return SPI_CODE_OK;
}

spi_codes_e spi_transceive(spi_t* spi, const uint32_t* src_buffer, uint32_t* dest_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_for_xfer(spi);
    if (len > MAX_COMMAND_LENGTH) error |= SPI_CODE_TXN_LEN_INVAL;
    if (error) return error;

    spi_segment_t seg = SPI_SEG_BIDIR(len);

    spi_transaction_t txn = {
        .segments = &seg,
        .seglen   = 1,
        .txbuffer = src_buffer,
        .txlen    = (len / BYTES_PER_WORD) + (len % BYTES_PER_WORD ? 1 : 0),
        .rxbuffer = dest_buffer,
        .rxlen    = (len / BYTES_PER_WORD) + (len % BYTES_PER_WORD ? 1 : 0)
    };

    spi_launch(&_peripherals[spi->idx], txn);

    while (_peripherals[spi->idx].busy) wait_for_interrupt();

    return SPI_CODE_OK;
}

spi_codes_e spi_execute(spi_t* spi, const spi_segment_t* segments, uint32_t segments_len, const uint32_t* src_buffer, uint32_t* dest_buffer) {
    spi_codes_e error = spi_prepare_for_xfer(spi);
    if (error) return error;

    uint32_t wcnt = 0;
    uint32_t rcnt = 0;

    // Validate all segments before commencing with transfer
    for (int i = 0; i < segments_len; i++)
    {
        uint8_t direction = bitfield_read(segments[i].mode, 0b11, 0);
        uint8_t speed     = bitfield_read(segments[i].mode, 0b11, 2);
        if (!spi_validate_cmd(direction, speed)) return SPI_CODE_SEGMENT_INVAL;
        uint32_t word_len = (segments[i].len / 4) + (segments[i].len % 4 ? 1 : 0);
        if (direction == SPI_DIR_TX_ONLY || direction == SPI_DIR_BIDIR) wcnt += word_len;
        if (direction == SPI_DIR_RX_ONLY || direction == SPI_DIR_BIDIR) rcnt += word_len;
    }
    // if (wcnt > txn.txlen || rcnt > txn.rxlen) return SPI_CODE_TXN_LEN_INVAL;

    spi_transaction_t txn = {
        .segments = segments,
        .seglen   = segments_len,
        .txbuffer = src_buffer,
        .txlen    = wcnt,
        .rxbuffer = dest_buffer,
        .rxlen    = rcnt
    };

    spi_launch(&_peripherals[spi->idx], txn);

    while (_peripherals[spi->idx].busy) wait_for_interrupt();

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
    if (SPI_CSID_INVALID(slave.csid))                        error |= SPI_CODE_SLAVE_CSID_INVAL;
    // if (slave.freq > soc_ctrl_peri->SYSTEM_FREQUENCY_HZ / 2) error |= SPI_CODE_SLAVE_FREQ_INVAL;
    return error;
}

spi_codes_e spi_set_slave(spi_t* spi) {
    if (spi_get_active(_peripherals[spi->idx].instance) == SPI_TRISTATE_TRUE) return SPI_CODE_NOT_IDLE;

    const uint32_t sys_freq = soc_ctrl_peri->SYSTEM_FREQUENCY_HZ;
    uint16_t clk_div = 1; // TODO: Should be 0...
    if (2 * spi->slave.freq < sys_freq) {
        clk_div = sys_freq / (2 * spi->slave.freq) - 1;
        if (sys_freq > spi->slave.freq * (2 * (clk_div + 1))) clk_div++;
    }
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
    if (config_error) return SPI_CODE_SLAVE_INVAL;
    spi_set_csid(_peripherals[spi->idx].instance, spi->slave.csid);
    return SPI_CODE_OK;
}

spi_codes_e spi_prepare_for_xfer(spi_t* spi) {
    spi_codes_e error = spi_check_valid(spi);
    if (error) return error;

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
        ) peri->wcnt++;
    }
}

void spi_empty_rx(spi_peripheral_t* peri) {
    if (peri->txn.rxbuffer != NULL && peri->rcnt < peri->txn.rxlen)
    {
        while (
            peri->rcnt < peri->txn.rxlen 
            && !spi_read_word(peri->instance, &peri->txn.rxbuffer[peri->rcnt])
        ) peri->rcnt++;
    }
}

void spi_launch(spi_peripheral_t* peri, spi_transaction_t txn) {
    peri->busy = true;
    peri->txn  = txn;

    spi_set_tx_watermark(peri->instance, TX_WATERMARK);
    spi_set_rx_watermark(peri->instance, RX_WATERMARK);

    spi_fill_tx(peri);

    spi_set_events_enabled(peri->instance, SPI_EVENT_IDLE | SPI_EVENT_READY | SPI_EVENT_TXWM | SPI_EVENT_RXWM, true);
    spi_enable_evt_intr   (peri->instance, true);

    spi_wait_for_ready(peri->instance);
    peri->scnt++;
    spi_issue_cmd(peri, txn.segments[0], 0 < txn.seglen - 1 ? true : false);
}

void spi_issue_cmd(const spi_peripheral_t* peri, spi_segment_t seg, bool csaat) {
    uint32_t cmd_reg = spi_create_command((spi_command_t) {
        .direction = bitfield_read(seg.mode, 0b11, 0),
        .speed     = bitfield_read(seg.mode, 0b11, 2),
        .csaat     = csaat,
        .len       = seg.len - 1
    });
    return spi_set_command(peri->instance, cmd_reg) == SPI_FLAG_OK ? true : false;
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
    if (events & SPI_EVENT_READY) {
        // If SPI is ready and there are still commands to add, add them to queue
        if (peri->txn.segments != NULL && peri->scnt < peri->txn.seglen)
        {
            spi_issue_cmd(peri, peri->txn.segments[peri->scnt], peri->scnt < peri->txn.seglen - 1 ? 1 : 0);
            peri->scnt++;
        }
        // If no more commands and SPI is idle, then the transaction is over
        else if (events & SPI_EVENT_IDLE) {
            spi_set_events_enabled(peri->instance, SPI_EVENT_ALL, false);
            spi_enable_evt_intr(peri->instance, false);
            spi_empty_rx(peri);
            if (peri->evt_cb != NULL) peri->evt_cb();
            spi_reset_peri(peri);
            return;
        }
    }
    if (events & SPI_EVENT_TXWM) {
        spi_fill_tx(peri);
    }
    if (events & SPI_EVENT_RXWM) {
        spi_empty_rx(peri);
    }
}

/****************************************************************************/
/**                                                                        **/
/*                               INTERRUPTS                                 */
/**                                                                        **/
/****************************************************************************/

void spi_intr_handler_event_flash(spi_event_e events) {
    // if (!_peripherals[SPI_IDX_FLASH].busy) return;
    spi_event_handler(&_peripherals[SPI_IDX_FLASH], events);
}

void spi_intr_handler_error_flash(spi_error_e errors) {
    _peripherals[SPI_IDX_FLASH].busy = false;
    if (!_peripherals[SPI_IDX_FLASH].busy) return;
    // Abort transaction
    // Error Callback
}

void spi_intr_handler_event_host(spi_event_e events) {
    if (!_peripherals[SPI_IDX_HOST].busy) return;
    spi_event_handler(&_peripherals[SPI_IDX_HOST], events);
}

void spi_intr_handler_error_host(spi_error_e errors) {
    if (!_peripherals[SPI_IDX_HOST].busy) return;
    // Abort transaction
    // Error Callback
}

void spi_intr_handler_event_host2(spi_event_e events) {
    if (!_peripherals[SPI_IDX_HOST_2].busy) return;
    spi_event_handler(&_peripherals[SPI_IDX_HOST_2], events);
}

void spi_intr_handler_error_host2(spi_error_e errors) {
    if (!_peripherals[SPI_IDX_HOST_2].busy) return;
    // Abort transaction
    // Error Callback
}

/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
