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
* @author Llorenç Muela
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

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

#define DATA_MODE_CPOL_OFFS 1
#define DATA_MODE_CPHA_OFFS 0

#define MAX_COMMAND_LENGTH SPI_HOST_COMMAND_LEN_MASK
#define BYTES_PER_WORD     4
#define LEN_WORDS(bytes)   ((bytes / BYTES_PER_WORD) + (bytes % BYTES_PER_WORD ? 1 : 0))

#define TX_WATERMARK (SPI_HOST_PARAM_TX_DEPTH / 4)  // Arbirarily chosen
#define RX_WATERMARK (SPI_HOST_PARAM_RX_DEPTH - 12) // Arbirarily chosen

#define SPI_BUSY(peri)     peri.state == SPI_STATE_BUSY
#define SPI_NOT_BUSY(peri) peri.state != SPI_STATE_BUSY

#define SPI_INVALID_LEN(len) (len == 0 || len > MAX_COMMAND_LENGTH)

#define SPI_TXN_TX(segment, txbuff, len) (spi_transaction_t) { \
    .segments = segment, \
    .seglen   = 1, \
    .txbuffer = txbuff, \
    .txlen    = len, \
    .rxbuffer = NULL, \
    .rxlen    = 0 \
}

#define SPI_TXN_RX(segment, rxbuff, len) (spi_transaction_t) { \
    .segments = segment, \
    .seglen   = 1, \
    .txbuffer = NULL, \
    .txlen    = 0, \
    .rxbuffer = rxbuff, \
    .rxlen    = len \
}

#define SPI_TXN_BIDIR(segment, txbuff, rxbuff, len) (spi_transaction_t) { \
    .segments = segment, \
    .seglen   = 1, \
    .txbuffer = txbuff, \
    .txlen    = len, \
    .rxbuffer = rxbuff, \
    .rxlen    = len \
}

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

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
    spi_state_e       state;
    spi_transaction_t txn;
    uint32_t          scnt;
    uint32_t          wcnt;
    uint32_t          rcnt;
    spi_cb_t          done_cb;
    spi_cb_t          error_cb;
} spi_peripheral_t;

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

spi_codes_e spi_check_valid(spi_t* spi);
spi_codes_e spi_validate_slave(spi_slave_t slave);
spi_codes_e spi_set_slave(spi_t* spi);
spi_codes_e spi_prepare_transfer(spi_t* spi);
bool spi_validate_segments(const spi_segment_t* segments, uint32_t segments_len, uint32_t* tx_count, uint32_t* rx_count);
void spi_fill_tx(spi_peripheral_t* peri);
void spi_empty_rx(spi_peripheral_t* peri);
void spi_launch(spi_peripheral_t* peri, spi_t* spi, spi_transaction_t txn, spi_cb_t done_cb, spi_cb_t error_cb);
void spi_issue_cmd(const spi_peripheral_t* peri, spi_segment_t seg, bool csaat);
void spi_reset_peri(spi_peripheral_t* peri);
void spi_event_handler(spi_peripheral_t* peri, spi_event_e events);
void spi_error_handler(spi_peripheral_t* peri, spi_error_e error);

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
        .state     = SPI_STATE_NONE,
        .txn       = {0},
        .scnt      = 0,
        .wcnt      = 0,
        .rcnt      = 0,
        .done_cb   = NULL,
        .error_cb  = NULL
    },
    (spi_peripheral_t) {
        .instance  = spi_host1,
        .state     = SPI_STATE_NONE,
        .txn       = {0},
        .scnt      = 0,
        .wcnt      = 0,
        .rcnt      = 0,
        .done_cb   = NULL,
        .error_cb  = NULL
    },
    (spi_peripheral_t) {
        .instance  = spi_host2,
        .state     = SPI_STATE_NONE,
        .txn       = {0},
        .scnt      = 0,
        .wcnt      = 0,
        .rcnt      = 0,
        .done_cb   = NULL,
        .error_cb  = NULL
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
            .idx      = -1,
            .init     = false,
            .slave    = {0}
        };
    spi_set_enable(_peripherals[idx].instance, true);
    spi_output_enable(_peripherals[idx].instance, true);
    spi_set_errors_enabled(_peripherals[idx].instance, SPI_ERROR_IRQALL, true);
    return (spi_t) {
        .idx      = idx,
        .init     = true,
        .slave    = slave
    };
}

spi_codes_e spi_deinit(spi_t* spi) {
    spi->idx      = -1;
    spi->init     = false;
    spi->slave    = (spi_slave_t) {0};
    return SPI_CODE_OK;
}

spi_codes_e spi_reset(spi_t* spi) {
    spi_codes_e error = spi_check_valid(spi);
    if (error) return error;

    spi_sw_reset(_peripherals[spi->idx].instance);

    return SPI_CODE_OK;
}

spi_state_e spi_get_state(spi_t* spi) {
    spi_codes_e error = spi_check_valid(spi);
    if (error) return SPI_STATE_ARG_INVAL;

    return _peripherals[spi->idx].state;
}

spi_codes_e spi_transmit(spi_t* spi, const uint32_t* src_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_transfer(spi);
    if (SPI_INVALID_LEN(len)) error |= SPI_CODE_TXN_LEN_INVAL;
    if (error) return error;

    spi_segment_t     seg = SPI_SEG_TX(len);
    spi_transaction_t txn = SPI_TXN_TX(&seg, src_buffer, LEN_WORDS(len));

    spi_launch(&_peripherals[spi->idx], spi, txn, NULL, NULL);

    while (SPI_BUSY(_peripherals[spi->idx])) wait_for_interrupt();

    return SPI_CODE_OK;
}

spi_codes_e spi_receive(spi_t* spi, uint32_t* dest_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_transfer(spi);
    if (SPI_INVALID_LEN(len)) error |= SPI_CODE_TXN_LEN_INVAL;
    if (error) return error;

    spi_segment_t     seg = SPI_SEG_RX(len);
    spi_transaction_t txn = SPI_TXN_RX(&seg, dest_buffer, LEN_WORDS(len));

    spi_launch(&_peripherals[spi->idx], spi, txn, NULL, NULL);

    while (SPI_BUSY(_peripherals[spi->idx])) wait_for_interrupt();

    return SPI_CODE_OK;
}

spi_codes_e spi_transceive(spi_t* spi, const uint32_t* src_buffer, uint32_t* dest_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_transfer(spi);
    if (SPI_INVALID_LEN(len)) error |= SPI_CODE_TXN_LEN_INVAL;
    if (error) return error;

    spi_segment_t     seg = SPI_SEG_BIDIR(len);
    spi_transaction_t txn = SPI_TXN_BIDIR(&seg, src_buffer, dest_buffer, LEN_WORDS(len));

    spi_launch(&_peripherals[spi->idx], spi, txn, NULL, NULL);

    while (SPI_BUSY(_peripherals[spi->idx])) wait_for_interrupt();

    return SPI_CODE_OK;
}

spi_codes_e spi_execute(spi_t* spi, 
                        const spi_segment_t* segments, 
                        uint32_t segments_len, 
                        const uint32_t* src_buffer, 
                        uint32_t* dest_buffer
                       ) {
    spi_codes_e error = spi_prepare_transfer(spi);
    if (error) return error;

    spi_transaction_t txn = SPI_TXN(segments, segments_len, src_buffer, dest_buffer);

    if (!spi_validate_segments(txn.segments, txn.seglen, &txn.txlen, &txn.rxlen)) return SPI_CODE_SEGMENT_INVAL;

    spi_launch(&_peripherals[spi->idx], spi, txn, NULL, NULL);

    while (SPI_BUSY(_peripherals[spi->idx])) wait_for_interrupt();

    return SPI_CODE_OK;
}

spi_codes_e spi_transmit_nb(spi_t* spi, const uint32_t* src_buffer, uint32_t len, spi_cb_t done_cb, spi_cb_t error_cb) {
    spi_codes_e error = spi_prepare_transfer(spi);
    if (SPI_INVALID_LEN(len)) error |= SPI_CODE_TXN_LEN_INVAL;
    if (error) return error;

    spi_segment_t     seg = SPI_SEG_TX(len);
    spi_transaction_t txn = SPI_TXN_TX(&seg, src_buffer, LEN_WORDS(len));

    spi_launch(&_peripherals[spi->idx], spi, txn, done_cb, error_cb);

    return SPI_CODE_OK;
}

spi_codes_e spi_receive_nb(spi_t* spi, uint32_t* dest_buffer, uint32_t len, spi_cb_t done_cb, spi_cb_t error_cb) {
    spi_codes_e error = spi_prepare_transfer(spi);
    if (SPI_INVALID_LEN(len)) error |= SPI_CODE_TXN_LEN_INVAL;
    if (error) return error;

    spi_segment_t     seg = SPI_SEG_RX(len);
    spi_transaction_t txn = SPI_TXN_RX(&seg, dest_buffer, LEN_WORDS(len));

    spi_launch(&_peripherals[spi->idx], spi, txn, done_cb, error_cb);

    return SPI_CODE_OK;
}

spi_codes_e spi_transceive_nb(spi_t* spi, const uint32_t* src_buffer, uint32_t* dest_buffer, uint32_t len, spi_cb_t done_cb, spi_cb_t error_cb) {
    spi_codes_e error = spi_prepare_transfer(spi);
    if (SPI_INVALID_LEN(len)) error |= SPI_CODE_TXN_LEN_INVAL;
    if (error) return error;

    spi_segment_t     seg = SPI_SEG_BIDIR(len);
    spi_transaction_t txn = SPI_TXN_BIDIR(&seg, src_buffer, dest_buffer, LEN_WORDS(len));

    spi_launch(&_peripherals[spi->idx], spi, txn, NULL, NULL);

    return SPI_CODE_OK;
}

spi_codes_e spi_execute_nb(spi_t* spi, 
                           const spi_segment_t* segments, 
                           uint32_t segments_len, 
                           const uint32_t* src_buffer, 
                           uint32_t* dest_buffer, 
                           spi_cb_t done_cb, 
                           spi_cb_t error_cb
                          ) {
    spi_codes_e error = spi_prepare_transfer(spi);
    if (error) return error;

    spi_transaction_t txn = SPI_TXN(segments, segments_len, src_buffer, dest_buffer);

    if (!spi_validate_segments(txn.segments, txn.seglen, &txn.txlen, &txn.rxlen)) return SPI_CODE_SEGMENT_INVAL;

    spi_launch(&_peripherals[spi->idx], spi, txn, done_cb, error_cb);

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

spi_codes_e spi_prepare_transfer(spi_t* spi) {
    spi_codes_e error = spi_check_valid(spi);
    if (error) return error;

    if (SPI_BUSY(_peripherals[spi->idx])) return SPI_CODE_IS_BUSY;

    error = spi_set_slave(spi);
    if (error) return error;

    return SPI_CODE_OK;
}

bool spi_validate_segments(const spi_segment_t* segments, uint32_t segments_len, uint32_t* tx_count, uint32_t* rx_count) {
    *tx_count = 0;
    *rx_count = 0;

    for (int i = 0; i < segments_len; i++)
    {
        uint8_t direction = bitfield_read(segments[i].mode, 0b11, 0);
        uint8_t speed     = bitfield_read(segments[i].mode, 0b11, 2);
        if (!spi_validate_cmd(direction, speed)) return false;
        uint32_t word_len = LEN_WORDS(segments[i].len);
        if (direction == SPI_DIR_TX_ONLY || direction == SPI_DIR_BIDIR) *tx_count += word_len;
        if (direction == SPI_DIR_RX_ONLY || direction == SPI_DIR_BIDIR) *rx_count += word_len;
    }

    return true;
}

void spi_fill_tx(spi_peripheral_t* peri) {
    if (peri->txn.txbuffer != NULL && peri->wcnt < peri->txn.txlen) {
        while (
            peri->wcnt < peri->txn.txlen 
            && !spi_write_word(peri->instance, peri->txn.txbuffer[peri->wcnt])
        ) peri->wcnt++;
    }
}

void spi_empty_rx(spi_peripheral_t* peri) {
    if (peri->txn.rxbuffer != NULL && peri->rcnt < peri->txn.rxlen) {
        while (
            peri->rcnt < peri->txn.rxlen 
            && !spi_read_word(peri->instance, &peri->txn.rxbuffer[peri->rcnt])
        ) peri->rcnt++;
    }
}

void spi_launch(spi_peripheral_t* peri, spi_t* spi, spi_transaction_t txn, spi_cb_t done_cb, spi_cb_t error_cb) {
    peri->state    = SPI_STATE_BUSY;
    peri->txn      = txn;
    peri->done_cb  = done_cb;
    peri->error_cb = error_cb;

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
    spi_set_command(peri->instance, cmd_reg);
}

void spi_reset_peri(spi_peripheral_t* peri) {
    peri->scnt     = 0;
    peri->wcnt     = 0;
    peri->rcnt     = 0;
    peri->txn      = (spi_transaction_t) {0};
    peri->done_cb  = NULL;
    peri->error_cb = NULL;
}

void spi_event_handler(spi_peripheral_t* peri, spi_event_e events) {
    if (events & SPI_EVENT_READY) {
        // If SPI is ready and there are still commands to add, add them to queue
        if (peri->txn.segments != NULL && peri->scnt < peri->txn.seglen) {
            spi_issue_cmd(peri, peri->txn.segments[peri->scnt], peri->scnt < peri->txn.seglen - 1 ? 1 : 0);
            peri->scnt++;
        }
        // If no more commands and SPI is idle, then the transaction is over
        else if (events & SPI_EVENT_IDLE) {
            spi_set_events_enabled(peri->instance, SPI_EVENT_ALL, false);
            spi_enable_evt_intr   (peri->instance, false);
            spi_empty_rx(peri);
            peri->state = SPI_STATE_DONE;
            if (peri->done_cb != NULL) 
                peri->done_cb(peri->txn.txbuffer, peri->txn.txlen, peri->txn.rxbuffer, peri->txn.rxlen);
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

void spi_error_handler(spi_peripheral_t* peri, spi_error_e error) {
    spi_set_events_enabled(peri->instance, SPI_EVENT_ALL, false);
    spi_enable_evt_intr   (peri->instance, false);
    peri->state = SPI_STATE_ERROR;
    if (peri->error_cb != NULL) 
        peri->error_cb(peri->txn.txbuffer, peri->txn.txlen, peri->txn.rxbuffer, peri->txn.rxlen);
    spi_reset_peri(peri);
}

/****************************************************************************/
/**                                                                        **/
/*                               INTERRUPTS                                 */
/**                                                                        **/
/****************************************************************************/

void spi_intr_handler_event_flash(spi_event_e events) {
    if (SPI_NOT_BUSY(_peripherals[SPI_IDX_FLASH])) return;
    spi_event_handler(&_peripherals[SPI_IDX_FLASH], events);
}

void spi_intr_handler_error_flash(spi_error_e errors) {
    if (SPI_NOT_BUSY(_peripherals[SPI_IDX_FLASH])) return;
    spi_error_handler(&_peripherals[SPI_IDX_FLASH], errors);
}

void spi_intr_handler_event_host(spi_event_e events) {
    if (SPI_NOT_BUSY(_peripherals[SPI_IDX_HOST])) return;
    spi_event_handler(&_peripherals[SPI_IDX_HOST], events);
}

void spi_intr_handler_error_host(spi_error_e errors) {
    if (SPI_NOT_BUSY(_peripherals[SPI_IDX_HOST])) return;
    spi_error_handler(&_peripherals[SPI_IDX_HOST], errors);
}

void spi_intr_handler_event_host2(spi_event_e events) {
    if (SPI_NOT_BUSY(_peripherals[SPI_IDX_HOST_2])) return;
    spi_event_handler(&_peripherals[SPI_IDX_HOST_2], events);
}

void spi_intr_handler_error_host2(spi_error_e errors) {
    if (SPI_NOT_BUSY(_peripherals[SPI_IDX_HOST_2])) return;
    spi_error_handler(&_peripherals[SPI_IDX_HOST_2], errors);
}

/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
