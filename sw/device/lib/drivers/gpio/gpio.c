/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************                                                                                               
**                                                                         
** project  : X-HEEP                                                       
** filename : gpio.c                                                                                                        
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
* @file     gpio.c
* @date     30/03/2023
* @author   Hossein taji
* @version  1
* @brief    GPIO driver 
*/

/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "gpio.h"
#include "gpio_regs.h"  // Generated.
#include "gpio_structs.h"
#include "mmio.h"

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

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/
/**
 * @brief get some bitfield of a 32b reg.
 * @param reg the register that bitwise field selects from.
 * @param mask the mask w/o offset.
 * @param sel this the offset used for mask and value.
 */
static inline uint32_t getBitfield( uint32_t reg, uint32_t  mask, uint8_t sel )
{
    return (reg & ( mask << sel ));
}

/**
 * @brief do a bitwise operation on 32b register.
 * @param reg the register pointer that bitwise operation is done on it.
 * @param val the value should be written in part of reg w/o offset.
 * @param mask the mask w/o offset.
 * @param sel this the offset used for mask and value.
 */
static inline void setBitfield( uint32_t *reg, uint32_t  val, 
    uint32_t mask, uint8_t sel )
{
    *reg           &= ~( mask << sel );
    *reg           |= (val & mask) << sel;
}
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

gpio_result_t gpio_set_mode (gpio_pin_number_t pin, gpio_mode_t mode)
{
    if (pin >= 0 && pin < 16)
    {
        setBitfield(&(gpio_peri->GPIO_MODE0), mode, 0b11, 2*pin);
        return GpioOk;
    }
    else if (pin >= 16 && pin <32)
    {
        reg = setBit(&(gpio_peri->GPIO_MODE1), mode, 0b11, 2*(pin-16));
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_en_input (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->GPIO_EN0), 1, 0b1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_dis_input (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->GPIO_EN0), 0, 0b1, pin);
        return GpioOk;
    }
    else
    { 
        return GpioError;
    }
}

gpio_result_t gpio_pin_reset (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        gpio_intr_mode (0);
        gpio_set_mode (pin, GpioModeIn);
        gpio_dis_input (pin);
        setBitfield(&(gpio_peri->GPIO_CLEAR0), 0, 0b1, pin);
        setBitfield(&(gpio_peri->GPIO_SET0),   0, 0b1, pin);
        gpio_intr_dis_all(pin);
        gpio_intr_clear_stat(pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_reset_all (void)
{
    setBitfield(&(gpio_peri->GPIO_CLEAR0), 0, 0b1, 0);
    gpio_peri->GPIO_MODE0 = 0;
    gpio_peri->GPIO_MODE1 = 0;
    gpio_peri->GPIO_EN0 = 0;
    gpio_peri->GPIO_CLEAR0 = 0;
    gpio_peri->GPIO_SET0 = 0;
    gpio_peri->GPIO_TOGGLE0 = 0;
    gpio_peri->INTRPT_RISE_EN0 = 0;
    gpio_peri->INTRPT_FALL_EN0 = 0;
    gpio_peri->INTRPT_LVL_HIGH_EN0 = 0;
    gpio_peri->INTRPT_LVL_LOW_EN0 = 0;
    gpio_peri->INTRPT_STATUS0 = 0xFFFFFFFF;
}

gpio_result_t gpio_pin_read (gpio_pin_number_t pin, bool *val)
{
    if (pin >= 0 && pin < 32)
    {
        if ( getBitfield(gpio_peri->GPIO_IN0, 0b1, pin) == 1)
            *val = true;
        else
            *val = false;
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

// gpio_result_t gpio_pin_set (gpio_pin_number_t pin)
// {
//     if (pin >= 0 && pin < 32)
//     {
//         gpio_peri->GPIO_SET0 = 1 << pin;
//         gpio_peri->GPIO_OUT0 = 1 << pin;
//         return GpioOk;
//     }
//     else
//     {
//         return GpioError;
//     } 
// }

// gpio_result_t gpio_pin_clear (gpio_pin_number_t pin)
// {
//     if (pin >= 0 && pin < 32)
//     {
//         gpio_peri->GPIO_CLEAR0 = 1 << pin;
//         gpio_peri->GPIO_OUT0 &= ~(1 << pin);
//         return GpioOk;
//     }
//     else
//     {
//         return GpioError;
//     } 
// }

//todo: check to see its toggling or just writing one
gpio_result_t gpio_pin_toggle (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        gpio_peri->GPIO_TOGGLE0 = 1 << pin;
        gpio_peri->GPIO_OUT0 = 1 << pin;
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_pin_write (gpio_pin_number_t pin, gpio_value val)
{
    if (pin >= 0 && pin < 32)
    {
        if (val == true)
            setBitfield(&(gpio_peri->GPIO_OUT0), 1, 1, pin);
        else
            setBitfield(&(gpio_peri->GPIO_OUT0), 0, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_en_rise (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_RISE_EN0), 1, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_en_fall (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_FALL_EN0), 1, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_en_lvl_high (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_LVL_HIGH_EN0), 1, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_en_lvl_low (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_LVL_LOW_EN0), 1, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_dis_rise (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_RISE_EN0), 0, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_dis_fall (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_FALL_EN0), 0, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_dis_lvl_high (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_LVL_HIGH_EN0), 0, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_dis_lvl_low (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_LVL_LOW_EN0), 0, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_dis_all (gpio_pin_number_t pin){
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_RISE_EN0),     0, 1, pin);
        setBitfield(&(gpio_peri->INTRPT_FALL_EN0),     0, 1, pin);
        setBitfield(&(gpio_peri->INTRPT_LVL_HIGH_EN0), 0, 1, pin);
        setBitfield(&(gpio_peri->INTRPT_LVL_LOW_EN0),  0, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }

}

gpio_result_t gpio_intr_en (gpio_pin_number_t pin, gpio_intr_type_t type)
{
    if (pin >= 0 && pin < 32)
    { 
        gpio_intr_dis_all(pin);

        switch(type)
        {
        case GpioIntrEdgeRising:
            gpio_intr_en_rise(pin);
            break;

        case GpioIntrEdgeFalling:
            gpio_intr_en_fall(pin);
            break;

        case GpioIntrLevelLow:
            gpio_intr_en_lvl_low(pin);
            break;

        case GpioIntrLevelHigh:
            gpio_intr_en_lvl_high(pin);
            break;

        case GpioIntrEdgeRisingFalling:
            gpio_intr_en_rise(pin);
            gpio_intr_en_fall(pin);
            break;

        case GpioIntrEdgeRisingLevelLow:
            gpio_intr_en_rise(pin);
            gpio_intr_en_lvl_low(pin);
            break;

        case GpioIntrEdgeFallingLevelHigh:
            gpio_intr_en_fall(pin);
            gpio_intr_en_lvl_high(pin);
            break;

        default:
            return GpioError;
        }
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

// gpio_result_t gpio_intr_dis (gpio_pin_number_t pin, gpio_intr_type_t type)
// {
//     if (pin >= 0 && pin < 32)
//     {    
//         switch(type)
//         {
//         case GpioIntrEdgeRising:
//             gpio_intr_dis_rise(pin);
//             break;

//         case GpioIntrEdgeFalling:
//             gpio_intr_dis_fall(pin);
//             break;

//         case GpioIntrLevelLow:
//             gpio_intr_dis_lvl_low(pin);
//             break;

//         case GpioIntrLevelHigh:
//             gpio_intr_dis_lvl_high(pin);
//             break;

//         case GpioIntrEdgeRisingFalling:
//             gpio_intr_dis_rise(pin);
//             gpio_intr_dis_fall(pin);
//             break;

//         case GpioIntrEdgeRisingLevelLow:
//             gpio_intr_dis_rise(pin);
//             gpio_intr_dis_lvl_low(pin);
//             break;

//         case GpioIntrEdgeFallingLevelHigh:
//             gpio_intr_dis_fall(pin);
//             gpio_intr_dis_lvl_high(pin);
//             break;

//         default:
//             return GpioError;
//         }
//         return GpioOk;  
//     }
//     else
//     {
//         return GpioError;
//     }
// }

gpio_result_t gpio_intr_check_stat_rise (gpio_pin_number_t pin, bool *is_pending)
{
    if (pin >= 0 && pin < 32)
    {
        if (getBitfield(gpio_peri->INTRPT_RISE_STATUS0, 1, pin) == 1)
            *is_pending = true;
        else
            *is_pending = false;
        return GpioOk;
    }
    else
    {
        *is_pending = false;
        return GpioError;
    }
}

gpio_result_t gpio_intr_check_stat_fall (gpio_pin_number_t pin, bool *is_pending)
{
    if (pin >= 0 && pin < 32)
    {
        if (getBitfield(gpio_peri->INTRPT_FALL_STATUS0, 1, pin) == 1)
            *is_pending = true;
        else
            *is_pending = false;
        return GpioOk;
    }
    else
    {
        *is_pending = false;
        return GpioError;
    }
}

gpio_result_t gpio_intr_check_stat_lvl_low (gpio_pin_number_t pin, bool *is_pending)
{
    if (pin >= 0 && pin < 32)
    {
        if (getBitfield(gpio_peri->INTRPT_LVL_LOW_STATUS0, 1, pin) == 1)
            *is_pending = true;
        else
            *is_pending = false;
        return GpioOk;
    }
    else
    {
        *is_pending = false;
        return GpioError;
    }
}

gpio_result_t gpio_intr_check_stat_lvl_high (gpio_pin_number_t pin, bool *is_pending)
{
    if (pin >= 0 && pin < 32)
    {
        if (getBitfield(gpio_peri->INTRPT_LVL_HIGH_STATUS0, 1, pin) == 1)
            *is_pending = true;
        else
            *is_pending = false;
        return GpioOk;
    }
    else
    {
        *is_pending = false;
        return GpioError;
    }
}

gpio_result_t gpio_intr_check_stat (gpio_pin_number_t pin, bool *is_pending)
{
    if (pin >= 0 && pin < 32)
    {
        if (getBitfield(gpio_peri->INTRPT_STATUS0, 1, pin) == 1)
            *is_pending = true;
        else
            *is_pending = false;
        return GpioOk;
    }
    else
    {
        *is_pending = false;
        return GpioError;
    }
}

gpio_result_t gpio_intr_clear_stat_rise (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_RISE_STATUS0), 1, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_clear_stat_fall (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_FALL_STATUS0), 1, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_clear_stat_lvl_low (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_LVL_LOW_STATUS0), 1, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_clear_stat_lvl_high (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_LVL_HIGH_STATUS0), 1, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_clear_stat (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        setBitfield(&(gpio_peri->INTRPT_STATUS0), 1, 1, pin);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
}

gpio_result_t gpio_intr_mode (bool gpio_intr_mode)
{
    if (pin >= 0 && pin < 32)
    {
        if (gpio_intr_mode)
            setBitfield(&(gpio_peri->CFG), 1, 1, 0);
        else
            setBitfield(&(gpio_peri->CFG), 0, 1, 0);
        return GpioOk;
    }
    else
    {
        return GpioError;
    }
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

// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Modified version for core-v-mini-mcu
// original at: https://github.com/lowRISC/opentitan/blob/master/sw/



/**
 * Gives the mask that corresponds to the given bit index.
 *
 * @param index Bit index in a 32-bit register.
 */
static uint32_t index_to_mask(uint32_t index) { return 1u << index; }

gpio_result_t gpio_init(gpio_params_t params, gpio_t *gpio) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  gpio->params = params;

  return kGpioOk;
}

gpio_result_t gpio_reset(const gpio_t *gpio) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr,
                      GPIO_CFG_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_GPIO_MODE_0_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_GPIO_MODE_1_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_GPIO_EN_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_GPIO_CLEAR_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_GPIO_SET_REG_OFFSET, 0);
  // Also clear all pending interrupts
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTRPT_STATUS_REG_OFFSET, 0);

  return kGpioOk;
}

gpio_result_t gpio_irq_is_pending(const gpio_t *gpio,
                                          gpio_pin_t pin,
                                          bool *is_pending) {
  if (gpio == NULL || is_pending == NULL) {
    return kGpioBadArg;
  }

  *is_pending = mmio_region_get_bit32(gpio->params.base_addr,
                                      GPIO_INTRPT_STATUS_REG_OFFSET, pin);

  return kGpioOk;
}

gpio_result_t gpio_irq_is_pending_all(const gpio_t *gpio,
                                              gpio_state_t *is_pending) {
  if (gpio == NULL || is_pending == NULL) {
    return kGpioBadArg;
  }

  *is_pending =
      mmio_region_read32(gpio->params.base_addr, GPIO_INTRPT_STATUS_REG_OFFSET);

  return kGpioOk;
}

gpio_result_t gpio_irq_acknowledge(const gpio_t *gpio,
                                           gpio_pin_t pin) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_STATUS_REG_OFFSET,
                      pin);

  return kGpioOk;
}

gpio_result_t gpio_irq_get_enabled(const gpio_t *gpio,
                                           gpio_pin_t pin,
                                           gpio_toggle_t *state) {
  if (gpio == NULL || state == NULL) {
    return kGpioBadArg;
  }

  bool is_enabled = mmio_region_get_bit32(gpio->params.base_addr,
                                          GPIO_INTRPT_LVL_HIGH_EN_REG_OFFSET, pin);
  *state = is_enabled ? kGpioToggleEnabled : kGpioToggleDisabled;

  return kGpioOk;
}

gpio_result_t gpio_irq_set_trigger(const gpio_t *gpio,
                                           gpio_pin_t pin,
                                           bool state,
                                           gpio_irq_trigger_t trigger) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  // Disable all interrupt triggers for the given mask.
  mmio_region_write32(
      gpio->params.base_addr, GPIO_INTRPT_RISE_EN_REG_OFFSET, 0);
  mmio_region_write32(
      gpio->params.base_addr, GPIO_INTRPT_FALL_EN_REG_OFFSET, 0);
  mmio_region_write32(
      gpio->params.base_addr, GPIO_INTRPT_LVL_HIGH_EN_REG_OFFSET, 0);
  mmio_region_write32(
      gpio->params.base_addr, GPIO_INTRPT_LVL_LOW_EN_REG_OFFSET, 0);

  switch (trigger) {
    case kGpioIrqTriggerEdgeRising:
      mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_RISE_EN_REG_OFFSET, 1 << pin);
      break;
    case kGpioIrqTriggerEdgeFalling:
      mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_FALL_EN_REG_OFFSET, 1 << pin);
      break;
    case kGpioIrqTriggerLevelLow:
      mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_LVL_LOW_EN_REG_OFFSET, 1 << pin);
      break;
    case kGpioIrqTriggerLevelHigh:
      mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_LVL_HIGH_EN_REG_OFFSET, 1 << pin);
      break;
    case kGpioIrqTriggerEdgeRisingFalling:
      mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_RISE_EN_REG_OFFSET, 1 << pin);
      mmio_region_write32(gpio->params.base_addr,GPIO_INTRPT_FALL_EN_REG_OFFSET, 1 << pin);
      break;
    case kGpioIrqTriggerEdgeRisingLevelLow:
      mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_RISE_EN_REG_OFFSET, 1 << pin);
      mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_LVL_LOW_EN_REG_OFFSET, 1 << pin);
      break;
    case kGpioIrqTriggerEdgeFallingLevelHigh:
      mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_RISE_EN_REG_OFFSET, 1 << pin);
      mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_FALL_EN_REG_OFFSET, 1 << pin);
      mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_LVL_LOW_EN_REG_OFFSET, 1 << pin);
      break;
    default:
      return kGpioError;
  }

  return kGpioOk;
}

gpio_result_t gpio_read(const gpio_t *gpio, gpio_pin_t pin,
                                bool *state) {
  if (gpio == NULL || state == NULL) {
    return kGpioBadArg;
  }

  *state = mmio_region_get_bit32(gpio->params.base_addr,
                                 GPIO_GPIO_IN_REG_OFFSET, pin);

  return kGpioOk;
}

gpio_result_t gpio_write(const gpio_t *gpio, gpio_pin_t pin,
                                 bool state) {

  if (gpio == NULL) {
    return kGpioBadArg;
  }

  uint32_t reg_value = mmio_region_read32(gpio->params.base_addr, GPIO_GPIO_OUT_REG_OFFSET);
  reg_value = bitfield_bit32_write(reg_value, pin, state);
  mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_OUT_REG_OFFSET, reg_value);

  return kGpioOk;
}


gpio_result_t gpio_output_set_enabled(const gpio_t *gpio,
                                              gpio_pin_t pin,
                                              gpio_toggle_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  if(pin<16) {
    uint32_t reg_value = mmio_region_read32(gpio->params.base_addr, GPIO_GPIO_MODE_0_REG_OFFSET);
    reg_value = bitfield_bit32_write(reg_value, 2*pin, state);
    mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_MODE_0_REG_OFFSET, reg_value);
  } else {
    uint32_t reg_value = mmio_region_read32(gpio->params.base_addr, GPIO_GPIO_MODE_1_REG_OFFSET);
    reg_value = bitfield_bit32_write(reg_value, 2*(pin-16), state);
    mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_MODE_1_REG_OFFSET, reg_value);
  }

  return kGpioOk;
}

gpio_result_t gpio_input_enabled(const gpio_t *gpio,
                                          gpio_pin_t pin, gpio_toggle_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  uint32_t reg_value = mmio_region_read32(gpio->params.base_addr, GPIO_GPIO_EN_REG_OFFSET);
  reg_value = bitfield_bit32_write(reg_value, pin, state);
  mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_EN_REG_OFFSET, reg_value);

  return kGpioOk;
}