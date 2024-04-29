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
    bool busy;
    bool interrupt;
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
uint32_t spi_fill_tx(spi_t* spi, const uint32_t* src_buffer, uint32_t* counter, uint32_t len);
uint32_t spi_empty_rx(spi_t* spi, uint32_t* dest_buffer, uint32_t* counter, uint32_t len);

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
        .interrupt = false
    },
    (spi_peripheral_t) {
        .instance  = SPI_HOST_INIT,
        .busy      = false,
        .interrupt = false
    },
    (spi_peripheral_t) {
        .instance  = SPI_HOST2_INIT,
        .busy      = false,
        .interrupt = false
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
            .slave = slave,
            .event_callback = NULL,
            .error_callback = NULL
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
        .slave = slave,
        .event_callback = NULL,
        .error_callback = NULL
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

    _peripherals[spi->idx].busy = true;
    uint32_t count = 0;

    spi_fill_tx(spi, src_buffer, &count, len);

    // Issue command
    uint32_t cmd = spi_create_command((spi_command_t) {
        .direction = SPI_DIR_TX_ONLY,
        .speed     = SPI_SPEED_STANDARD,
        .csaat     = 0,
        .len       = len * BYTES_PER_WORD - 1
    });
    spi_set_command(_peripherals[spi->idx].instance, cmd); // TODO: Verify this doesn't return error...

    while (count < len)
        spi_fill_tx(spi, src_buffer, &count, len);
    
    spi_wait_for_idle(_peripherals[spi->idx].instance);

    _peripherals[spi->idx].busy = false;
    return SPI_CODE_OK;
}

spi_codes_e spi_receive(spi_t* spi, uint32_t* dest_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_for_xfer(spi, len);
    if (error) return error;

    _peripherals[spi->idx].busy = true;
    uint32_t count = 0;

    // Issue command
    uint32_t cmd = spi_create_command((spi_command_t) {
        .direction = SPI_DIR_RX_ONLY,
        .speed     = SPI_SPEED_STANDARD,
        .csaat     = 0,
        .len       = len * BYTES_PER_WORD - 1
    });
    spi_set_command(_peripherals[spi->idx].instance, cmd); // TODO: Verify this doesn't return error...

    while (count < len)
        spi_empty_rx(spi, dest_buffer, &count, len);

    _peripherals[spi->idx].busy = false;
    return SPI_CODE_OK;
}

spi_codes_e spi_transceive(spi_t* spi, const uint32_t* src_buffer, uint32_t* dest_buffer, uint32_t len) {
    spi_codes_e error = spi_prepare_for_xfer(spi, len);
    if (error) return error;

    _peripherals[spi->idx].busy = true;
    uint32_t w_count = 0;
    uint32_t r_count = 0;

    spi_fill_tx(spi, src_buffer, &w_count, len);

    // Issue command
    uint32_t cmd = spi_create_command((spi_command_t) {
        .direction = SPI_DIR_BIDIR,
        .speed     = SPI_SPEED_STANDARD,
        .csaat     = 0,
        .len       = len * BYTES_PER_WORD - 1
    });
    spi_set_command(_peripherals[spi->idx].instance, cmd); // TODO: Verify this doesn't return error...

    while (w_count < len || r_count < len)
    {
        spi_fill_tx(spi, src_buffer, &w_count, len);
        spi_empty_rx(spi, dest_buffer, &r_count, len);
    }
    spi_wait_for_idle(_peripherals[spi->idx].instance);

    _peripherals[spi->idx].busy = false;
    return SPI_CODE_OK;
}

spi_codes_e spi_execute(spi_t* spi, const spi_transaction_t* transaction) {
    // TODO: replace this 1, good thing is we don't care about len because segments already limit length
    spi_codes_e error = spi_prepare_for_xfer(spi, 1);
    if (error) return error;

    // Validate all segments before commencing with transfer
    for (int i = 0; i < transaction->seglen; i++)
    {
        uint8_t direction = bitfield_read(transaction->segments[i].mode, BIT_MASK_2, 2);
        uint8_t speed     = bitfield_read(transaction->segments[i].mode, BIT_MASK_2, 0);
        if (!spi_validate_cmd(direction, speed)) return SPI_CODE_SEGMENT_INVAL;
    }
    
    _peripherals[spi->idx].busy = true;
    uint32_t wcnt = 0;
    uint32_t rcnt = 0;
    uint32_t scnt = 0;

    while (wcnt < transaction->txlen || rcnt < transaction->rxlen || scnt < transaction->seglen)
    {
        spi_fill_tx(spi, transaction->txbuffer, &wcnt, transaction->txlen);
        spi_empty_rx(spi, transaction->rxbuffer, &rcnt, transaction->rxlen);
        if (scnt < transaction->seglen && spi_get_ready(_peripherals[spi->idx].instance) == SPI_TRISTATE_TRUE)
        {
            uint32_t cmd_reg = spi_create_command((spi_command_t) {
                .direction = bitfield_read(transaction->segments[scnt].mode, BIT_MASK_2, 2),
                .speed     = bitfield_read(transaction->segments[scnt].mode, BIT_MASK_2, 0),
                .csaat     = wcnt < transaction->seglen - 1 ? 1 : 0,
                .len       = transaction->segments[scnt].len
            });
            spi_set_command(_peripherals[spi->idx].instance, cmd_reg);
            scnt++;
        }
    }
    spi_wait_for_idle(_peripherals[spi->idx].instance);

    _peripherals[spi->idx].busy = false;
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

    error = spi_set_slave(spi);
    if (error) return error;

    return SPI_CODE_OK;
}

uint32_t spi_fill_tx(spi_t* spi, const uint32_t* src_buffer, uint32_t* counter, uint32_t len) {
    while (*counter < len && !spi_write_word(_peripherals[spi->idx].instance, src_buffer[*counter])) *counter++;
}

uint32_t spi_empty_rx(spi_t* spi, uint32_t* dest_buffer, uint32_t* counter, uint32_t len) {
    while (*counter < len && !spi_read_word(_peripherals[spi->idx].instance, &dest_buffer[*counter])) *counter++;
}

/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
