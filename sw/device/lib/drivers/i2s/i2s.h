/*
                              *******************
******************************* C SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : i2s.h                                                        **
** date     : 18/04/2023                                                   **
**                                                                         **
*****************************************************************************
**                                                                         **
** Copyright (c) EPFL contributors.                                        **
** All rights reserved.                                                    **
**                                                                         **
*****************************************************************************

*/

/***************************************************************************/
/***************************************************************************/

/**
* @file   i2s.h
* @date   08/05/2023
* @author Tim Frey
* @brief  HAL of the I2S peripheral
*
*/

#ifndef _DRIVERS_I2S_H_
#define _DRIVERS_I2S_H_

/**
 * Address of the I2S data of the read channel to be passed as address to the DMA
 */
#define I2S_RX_DATA_ADDRESS (uint32_t)(I2S_RXDATA_REG_OFFSET+I2S_START_ADDRESS)


/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "bitfield.h"
#include "i2s_regs.h"

#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************/
/**                                                                        **/
/*                        TYPEDEFS AND STRUCTURES                           */
/**                                                                        **/
/****************************************************************************/

typedef enum i2s_word_length {
  I2S_08_BITS = I2S_CONTROL_DATA_WIDTH_VALUE_8_BITS,
  I2S_16_BITS = I2S_CONTROL_DATA_WIDTH_VALUE_16_BITS,
  I2S_24_BITS = I2S_CONTROL_DATA_WIDTH_VALUE_24_BITS,
  I2S_32_BITS = I2S_CONTROL_DATA_WIDTH_VALUE_32_BITS
} i2s_word_length_t;


typedef enum i2s_channel_sel {
  I2S_DISABLE = I2S_CONTROL_EN_RX_VALUE_DISABLED,
  I2S_LEFT_CH = I2S_CONTROL_EN_RX_VALUE_ONLY_LEFT,
  I2S_RIGHT_CH = I2S_CONTROL_EN_RX_VALUE_ONLY_RIGHT,
  I2S_BOTH_CH = I2S_CONTROL_EN_RX_VALUE_BOTH_CHANNELS
} i2s_channel_sel_t;


/****************************************************************************/
/**                                                                        **/
/*                          EXPORTED FUNCTIONS                              */
/**                                                                        **/
/****************************************************************************/

/**
 * Initialize I2S peripheral
 * 
 * Generates SCK and WS 
 * (with the given parameters frequency and word length)
 * 
 * @note to change clock freq or word length call `i2s_terminate` first 
 * 
 * @param div_value Divider value = src_clk_freq / gen_clk_freq 
 *        (odd values are allowed, for 0 and 1 the src clock is used)
 * @param word_length (see i2s_word_length_t)
 * @return false if the periperal was on
 */
bool    i2s_init(uint16_t div_value, i2s_word_length_t word_length);

/**
 * Terminate I2S peripheral 
 * 
 * Stops SCK and WS
 */
void i2s_terminate();


/**
 * check if i2s peripheral has been initialized
 * 
 * @return true if i2s peripheral enable
 */
bool i2s_is_init();


//
// RX Channel
// 

/**
 * I2S start rx channels 
 * 
 * @note this function might take some time to complete
 * 
 * @param channels to be enabled (see i2s_channel_sel_t)
 * @return false if i2s not init
 */
bool    i2s_rx_start(i2s_channel_sel_t channels);

/**
 * I2S stop rx channels 
 * 
 */
void    i2s_rx_stop(void);


/**
 * I2S check RX data availability
 * 
 * @return true if RX data is available 
 */
bool    i2s_rx_data_available(void);

/**
 * I2S read RX word
 * 
 * @note The MSBs outside of word_length are 0.
 *
 * @return uint32_t RX word 
 */
uint32_t i2s_rx_read_data(void);

/**
 * I2S check RX FIFO overflow
 * 
 * @note call i2s_rx_start(...) to reset overflow and cleanly restart
 * 
 * @return true if RX FIFO overflowed  
 * @return false 
 */
bool    i2s_rx_overflow(void);



// Watermark

/**
 * I2S enable and configure watermark counter
 * 
 * @param watermark number to trigger interrupt 
 * @param interrupt_en enable/disable interrupt
 */
void    i2s_rx_enable_watermark(uint16_t watermark, bool interrupt_en);

/**
 * I2S disable watermark counter
 */
void    i2s_rx_disable_watermark(void);


/**
 * I2S read value of watermark counter
 * 
 * @return uint16_t current counter value
 */
uint16_t i2s_rx_read_waterlevel(void);


/**
 * I2S reset RX Watermark counter to 0
 * 
 */
void    i2s_rx_reset_waterlevel(void);


#ifdef __cplusplus
}
#endif

#endif // _DRIVERS_I2S_H_

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
