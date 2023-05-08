/*
                              *******************
******************************* C SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : i2s.c                                                        **
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
* @file   i2s.c
* @date   08/05/2023
* @author Tim Frey
* @brief  HAL of the I2S peripheral
*
*/


/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/


#include "i2s.h"
#include "i2s_structs.h"


/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

void i2s_configure(uint16_t div_value, i2s_word_length_t word_length)
{
  if (div_value == 0) { 
    // there is a bug in the clock divider for div_value = 0
    // therfore the hardware disables the clock divider for 0 so provide 1
    div_value = 1;
  }
  // write clock divider value to register
  i2s_peri->CLKDIVIDX = div_value;

  // write word_length to register
  uint32_t control = i2s_peri->CONTROL;
  bitfield_field32_write(control, I2S_CONTROL_DATA_WIDTH_FIELD, word_length);
  i2s_peri->CONTROL = control;
}

void i2s_rx_enable_watermark(uint16_t watermark, bool interrupt_en)
{
  i2s_peri->WATERMARK = watermark;

  uint32_t control = i2s_peri->CONTROL;
  // enable/disable interrupt
  control = bitfield_bit32_write(control, I2S_CONTROL_INTR_EN_BIT, interrupt_en);
  // enable counter
  control |= (1 << I2S_CONTROL_EN_WATERMARK_BIT); 
  i2s_peri->CONTROL = control;
}

void i2s_rx_disable_watermark()
{
  // disable interrupt and disable watermark counter
  i2s_peri->CONTROL &= ~((1 << I2S_CONTROL_INTR_EN_BIT) + (1 << I2S_CONTROL_EN_WATERMARK_BIT));
}

bool i2s_rx_data_available()
{
  // read data ready bit from STATUS register
  return (i2s_peri->STATUS & (1 << I2S_STATUS_RX_DATA_READY_BIT));
}

uint32_t i2s_rx_read_data()
{
  // read RXDATA register
  return i2s_peri->RXDATA;
}

uint16_t i2s_rx_read_waterlevel()
{
  // read WATERLEVEL register
  return (uint16_t) i2s_peri->WATERLEVEL;
}

bool i2s_rx_overflow()
{
  // read overflow bit from STATUS register
  return (i2s_peri->STATUS & (1 << I2S_STATUS_RX_OVERFLOW_BIT));
}

void i2s_rx_start(i2s_channel_sel_t channels)
{
  if (channels == I2S_DISABLE) {
    // no channels selected -> disable
    i2s_rx_stop();
    return;
  }

  uint32_t control = i2s_peri->CONTROL;

  // check overflow before changing state
  bool overflow = i2s_rx_overflow(); 


  control &= ~(
    (I2S_CONTROL_EN_RX_MASK << I2S_CONTROL_EN_RX_OFFSET) // disable rx
    + (1 << I2S_CONTROL_EN_WS_BIT)                       // disable ws gen
    + (1 << I2S_CONTROL_EN_IO_BIT)                       // disable ios
  );

  // enable clock
  control |= (1 << I2S_CONTROL_EN_BIT); // enable peripheral and clock domain
  i2s_peri->CONTROL = control;

  // check if overflow has occurred
  if (overflow) {
    // overflow bit is going to be reset by rx channel if the sck is running
    while (i2s_rx_overflow()) ; // wait for one SCK rise - this might take some time as the SCK can be much slower
    
    // cdc_2phase FIFO is not clearable, so we have to empty the FIFO manually
    // note: this uses much less resources
    while (i2s_rx_data_available()) {
      i2s_rx_read_data(); // read to empty FIFO
    }
  }

  // now we can start the RX channels and ws generation

  // enable WS generation and IOs  
  control |= ((1 << I2S_CONTROL_EN_WS_BIT) | (1 << I2S_CONTROL_EN_IO_BIT)); 

  // enable selected rx channels
  control |= (channels << I2S_CONTROL_EN_RX_OFFSET);

  i2s_peri->CONTROL = control;
}

void i2s_rx_stop()
{
  // disable all modules
  uint32_t control = i2s_peri->CONTROL;
  control &= ~(
    + (1 << I2S_CONTROL_EN_BIT)                             // disable peripheral and clock domain
    + (I2S_CONTROL_EN_RX_MASK << I2S_CONTROL_EN_RX_OFFSET)  // disable rx
    + (1 << I2S_CONTROL_EN_WS_BIT)                          // disable ws gen
    + (1 << I2S_CONTROL_EN_IO_BIT)                           // disable ios
  );
  i2s_peri->CONTROL = control;
}

void    i2s_rx_reset_waterlevel(void)
{
  // set "reset watermark" bit in CONTROL register
  i2s_peri->CONTROL |= (1 << I2S_CONTROL_RESET_WATERMARK_BIT);
}


/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
