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
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "gpio.h"
#include "gpio_regs.h"  // Generated.
#include "gpio_structs.h"
#include "core_v_mini_mcu.h"
#include "bitfield.h"
#include "x-heep.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/



/**
 * GPIO_EN values
 */
#define GPIO_EN__ENABLED     1
#define GPIO_EN__DISABLED    0

/**
 * GPIO_CLEAR and GPIO_SET putting and removing mask
 */
#define GPIO_PUT_MASK 0x01
#define GPIO_REMOVE_MASK 0x00

/**
 * GPIO is set or reset. The value read from gpio_perif->GPIO_IN0 or
 * write to gpio_perif->GPIO_OUT0
 */
#define GPIO_IS_SET     1
#define GPIO_IS_RESET   0

/**
 * To enable and disable intr by wrting to INTR_EN registers
 */
#define GPIO_INTR_ENABLE    1
#define GPIO_INTR_DISABLE   0

/**
 * Values read after checking intr status registers to see they are
 * triggered or not
 */
#define GPIO_INTR_IS_TRIGGERED      1
#define GPIO_INTR_IS_NOT_TRIGGERED  0

/**
 * Clearing the status bit by writing one into int
 */
#define GPIO_INTR_CLEAR     1

/**
 * GPIO intr mode configration index inside GPIO_CFG register.
 */
#define GPIO_CFG_INTR_MODE_INDEX    0

/**
 * GPIO mode, number of pins it includes
 */
#define GPIO_MODE_NUM_PINS      16

/**
 * Array of handlers for the different GPIO interrupts.
 */
//void (*gpio_handlers[ GPIO_INTR_QTY ])( void );


/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

/**
 * A dummy function to prevent unassigned irq to access a null pointer.
 */

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

gpio_result_t gpio_config (volatile gpio *gpio_perif, gpio_cfg_t cfg)
{
    /* check that pin is in acceptable range. */
    if (cfg.pin > (MAX_PIN-1) || cfg.pin < 0)
        return GpioPinNotAcceptable;
    /* reset pin coniguration first.*/
    gpio_reset (gpio_perif, cfg.pin);
    /* check mode. */
    if ((cfg.mode < GpioModeIn) || (cfg.mode > GpioModeoutOpenDrain1))
        return GpioModeNotAcceptable;
    /* set mode. */
    gpio_set_mode (gpio_perif, cfg.pin, cfg.mode);
    /* if input sampling is enabled */
    if (cfg.en_input_sampling == true)
        gpio_en_input_sampling (gpio_perif, cfg.pin);
    /* if interrupt is enabled. Also after enabling check for error */
    if (cfg.en_intr == true)
        if (gpio_intr_en (gpio_perif, cfg.pin, cfg.intr_type) != GpioOk)
            return GpioIntrTypeNotAcceptable;
    return GpioOk;
}

gpio_result_t gpio_set_mode (volatile gpio *gpio_perif, gpio_pin_number_t pin, gpio_mode_t mode)
{
    if (pin < 0 && pin > (MAX_PIN-1))
        return GpioModeNotAcceptable;

    if (pin >= 0 && pin < 1*GPIO_MODE_NUM_PINS)
        gpio_perif->GPIO_MODE0 = bitfield_write(gpio_perif->GPIO_MODE0,
                                               BIT_MASK_3, 2*pin, mode);
    else
        gpio_perif->GPIO_MODE1 = bitfield_write(gpio_perif->GPIO_MODE1,
                                               BIT_MASK_3, 2*(pin-GPIO_MODE_NUM_PINS), mode);
    return GpioOk;
}

gpio_result_t gpio_en_input_sampling (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->GPIO_EN0 = bitfield_write(gpio_perif->GPIO_EN0,
                                         BIT_MASK_1, pin, GPIO_EN__ENABLED);
    return GpioOk;
}

gpio_result_t gpio_dis_input_sampling (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->GPIO_EN0 = bitfield_write(gpio_perif->GPIO_EN0,
                                         BIT_MASK_1, pin, GPIO_EN__DISABLED);
    return GpioOk;
}

gpio_result_t gpio_reset (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;

    gpio_intr_set_mode (gpio_perif, pin, 0);
    gpio_set_mode (gpio_perif, pin, GpioModeIn);
    gpio_dis_input_sampling (gpio_perif, pin);
    gpio_perif->GPIO_CLEAR0 = bitfield_write(gpio_perif->GPIO_CLEAR0,
                                         BIT_MASK_1, pin, GPIO_REMOVE_MASK);
    gpio_perif->GPIO_SET0 = bitfield_write(gpio_perif->GPIO_SET0,
                                         BIT_MASK_1, pin, GPIO_REMOVE_MASK);
    gpio_intr_dis_all(gpio_perif, pin);
    gpio_intr_clear_stat(gpio_perif, pin);
    return GpioOk;
}

gpio_result_t gpio_reset_all (volatile gpio *gpio_perif)
{
    gpio_perif->GPIO_MODE0 = 0;
    gpio_perif->GPIO_MODE1 = 0;
    gpio_perif->GPIO_EN0 = 0;
    gpio_perif->GPIO_CLEAR0 = 0;
    gpio_perif->GPIO_SET0 = 0;
    gpio_perif->GPIO_TOGGLE0 = 0;
    gpio_perif->INTRPT_RISE_EN0 = 0;
    gpio_perif->INTRPT_FALL_EN0 = 0;
    gpio_perif->INTRPT_LVL_HIGH_EN0 = 0;
    gpio_perif->INTRPT_LVL_LOW_EN0 = 0;
    gpio_perif->INTRPT_STATUS0 = 0xFFFFFFFF;

    gpio_reset_handlers_list( );
}

gpio_result_t gpio_read (volatile gpio *gpio_perif, gpio_pin_number_t pin, bool *val)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    *val = bitfield_read(gpio_perif->GPIO_IN0, BIT_MASK_1, pin);
    return GpioOk;
}

gpio_result_t gpio_toggle (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->GPIO_OUT0 = bitfield_write(gpio_perif->GPIO_OUT0, BIT_MASK_1,
        pin, !(bitfield_read(gpio_perif->GPIO_OUT0, BIT_MASK_1, pin)));
    return GpioOk;
}

gpio_result_t gpio_write (volatile gpio *gpio_perif, gpio_pin_number_t pin, bool val)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->GPIO_OUT0 = bitfield_write(gpio_perif->GPIO_OUT0,
        BIT_MASK_1, pin, val);
    return GpioOk;

}

gpio_result_t gpio_intr_en_rise (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_RISE_EN0 =  bitfield_write(
        gpio_perif->INTRPT_RISE_EN0, BIT_MASK_1, pin, GPIO_INTR_ENABLE);
    return GpioOk;
}

gpio_result_t gpio_intr_en_fall (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_FALL_EN0 =  bitfield_write(gpio_perif->INTRPT_FALL_EN0,
        BIT_MASK_1, pin, GPIO_INTR_ENABLE);
    return GpioOk;
}

gpio_result_t gpio_intr_en_lvl_high (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_LVL_HIGH_EN0 =  bitfield_write(
        gpio_perif->INTRPT_LVL_HIGH_EN0, BIT_MASK_1, pin, GPIO_INTR_ENABLE);
    return GpioOk;
}

gpio_result_t gpio_intr_en_lvl_low (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_LVL_LOW_EN0 =  bitfield_write(
        gpio_perif->INTRPT_LVL_LOW_EN0, BIT_MASK_1, pin, GPIO_INTR_ENABLE);
    return GpioOk;
}

gpio_result_t gpio_intr_dis_rise (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_RISE_EN0 =  bitfield_write(
        gpio_perif->INTRPT_RISE_EN0, BIT_MASK_1, pin, GPIO_INTR_DISABLE);
    return GpioOk;
}

gpio_result_t gpio_intr_dis_fall (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_FALL_EN0 =  bitfield_write(
        gpio_perif->INTRPT_FALL_EN0, BIT_MASK_1, pin, GPIO_INTR_DISABLE);
    return GpioOk;
}

gpio_result_t gpio_intr_dis_lvl_high (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_LVL_HIGH_EN0 =  bitfield_write(
        gpio_perif->INTRPT_LVL_HIGH_EN0, BIT_MASK_1, pin, GPIO_INTR_DISABLE);
    return GpioOk;
}

gpio_result_t gpio_intr_dis_lvl_low (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_LVL_LOW_EN0 =  bitfield_write(
        gpio_perif->INTRPT_LVL_LOW_EN0, BIT_MASK_1, pin, GPIO_INTR_DISABLE);
    return GpioOk;
}

gpio_result_t gpio_intr_dis_all (volatile gpio *gpio_perif, gpio_pin_number_t pin){
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_RISE_EN0 =  bitfield_write(
        gpio_perif->INTRPT_RISE_EN0, BIT_MASK_1, pin, GPIO_INTR_DISABLE);
    gpio_perif->INTRPT_FALL_EN0 =  bitfield_write(
        gpio_perif->INTRPT_FALL_EN0, BIT_MASK_1, pin, GPIO_INTR_DISABLE);
    gpio_perif->INTRPT_LVL_HIGH_EN0 =  bitfield_write(
        gpio_perif->INTRPT_LVL_HIGH_EN0, BIT_MASK_1, pin, GPIO_INTR_DISABLE);
    gpio_perif->INTRPT_LVL_LOW_EN0 =  bitfield_write(
        gpio_perif->INTRPT_LVL_LOW_EN0, BIT_MASK_1, pin, GPIO_INTR_DISABLE);
    return GpioOk;
}

gpio_result_t gpio_intr_en (volatile gpio *gpio_perif, gpio_pin_number_t pin, gpio_intr_type_t type)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_intr_dis_all(gpio_perif, pin);
    switch(type)
    {
    case GpioIntrEdgeRising:
        gpio_intr_en_rise(gpio_perif, pin);
        break;
    case GpioIntrEdgeFalling:
        gpio_intr_en_fall(gpio_perif, pin);
        break;
    case GpioIntrLevelLow:
        gpio_intr_en_lvl_low(gpio_perif, pin);
        break;
    case GpioIntrLevelHigh:
        gpio_intr_en_lvl_high(gpio_perif, pin);
        break;
    case GpioIntrEdgeRisingFalling:
        gpio_intr_en_rise(gpio_perif, pin);
        gpio_intr_en_fall(gpio_perif, pin);
        break;
    case GpioIntrEdgeRisingLevelLow:
        gpio_intr_en_rise(gpio_perif, pin);
        gpio_intr_en_lvl_low(gpio_perif, pin);
        break;
    case GpioIntrEdgeFallingLevelHigh:
        gpio_intr_en_fall(gpio_perif, pin);
        gpio_intr_en_lvl_high(gpio_perif, pin);
        break;
    default:
        return GpioIntrTypeNotAcceptable;
    }
    return GpioOk;
}

gpio_result_t gpio_intr_check_stat_rise (volatile gpio *gpio_perif, gpio_pin_number_t pin, bool *is_pending)
{
    if (pin > (MAX_PIN-1) || pin < 0)
    {
        *is_pending = GPIO_INTR_IS_NOT_TRIGGERED;
        return GpioPinNotAcceptable;
    }
    *is_pending = bitfield_read(gpio_perif->INTRPT_RISE_STATUS0, BIT_MASK_1, pin);
    return GpioOk;
}

gpio_result_t gpio_intr_check_stat_fall (volatile gpio *gpio_perif, gpio_pin_number_t pin, bool *is_pending)
{
    if (pin > (MAX_PIN-1) || pin < 0)
    {
        *is_pending = GPIO_INTR_IS_NOT_TRIGGERED;
        return GpioPinNotAcceptable;
    }
    *is_pending = bitfield_read(gpio_perif->INTRPT_FALL_STATUS0, BIT_MASK_1, pin);
    return GpioOk;
}

gpio_result_t gpio_intr_check_stat_lvl_low (volatile gpio *gpio_perif, gpio_pin_number_t pin, bool *is_pending)
{
    if (pin > (MAX_PIN-1) || pin < 0)
    {
        *is_pending = GPIO_INTR_IS_NOT_TRIGGERED;
        return GpioPinNotAcceptable;
    }
    *is_pending = bitfield_read(gpio_perif->INTRPT_LVL_LOW_STATUS0, BIT_MASK_1, pin);
    return GpioOk;
}

gpio_result_t gpio_intr_check_stat_lvl_high (volatile gpio *gpio_perif, gpio_pin_number_t pin, bool *is_pending)
{
    if (pin > (MAX_PIN-1) || pin < 0)
    {
        *is_pending = GPIO_INTR_IS_NOT_TRIGGERED;
        return GpioPinNotAcceptable;
    }
     *is_pending = bitfield_read(gpio_perif->INTRPT_LVL_HIGH_STATUS0, BIT_MASK_1, pin);
    return GpioOk;
}

gpio_result_t gpio_intr_check_stat (volatile gpio *gpio_perif, gpio_pin_number_t pin, bool *is_pending)
{
    if (pin > (MAX_PIN-1) || pin < 0)
    {
        *is_pending = GPIO_INTR_IS_NOT_TRIGGERED;
        return GpioPinNotAcceptable;
    }
    *is_pending = bitfield_read(gpio_perif->INTRPT_STATUS0, BIT_MASK_1, pin);
    return GpioOk;
}

gpio_result_t gpio_intr_clear_stat_rise (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_RISE_STATUS0 = bitfield_write(
        gpio_perif->INTRPT_RISE_STATUS0, BIT_MASK_1, pin, GPIO_INTR_CLEAR);
    return GpioOk;

}

gpio_result_t gpio_intr_clear_stat_fall (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_FALL_STATUS0 = bitfield_write(
        gpio_perif->INTRPT_FALL_STATUS0, BIT_MASK_1, pin, GPIO_INTR_CLEAR);
    return GpioOk;
}

gpio_result_t gpio_intr_clear_stat_lvl_low (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_LVL_LOW_STATUS0 = bitfield_write(
        gpio_perif->INTRPT_LVL_LOW_STATUS0, BIT_MASK_1, pin, GPIO_INTR_CLEAR);
    return GpioOk;
}

gpio_result_t gpio_intr_clear_stat_lvl_high (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_LVL_HIGH_STATUS0 = bitfield_write(
        gpio_perif->INTRPT_LVL_HIGH_STATUS0, BIT_MASK_1, pin, GPIO_INTR_CLEAR);
    return GpioOk;
}

gpio_result_t gpio_intr_clear_stat (volatile gpio *gpio_perif, gpio_pin_number_t pin)
{
    if (pin > (MAX_PIN-1) || pin < 0)
        return GpioPinNotAcceptable;
    gpio_perif->INTRPT_STATUS0 = bitfield_write(
        gpio_perif->INTRPT_STATUS0, BIT_MASK_1, pin, GPIO_INTR_CLEAR);
    return GpioOk;
}

void gpio_intr_set_mode (volatile gpio *gpio_perif, gpio_pin_number_t pin, gpio_intr_general_mode_t mode)
{
    gpio_perif->CFG = bitfield_write(
        gpio_perif->CFG, BIT_MASK_1, GPIO_CFG_INTR_MODE_INDEX, mode);
}

/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

__attribute__((optimize("O0"))) static void gpio_handler_irq_dummy( uint32_t dummy )
{
  return;
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/


#ifdef __cplusplus
}
#endif  // __cplusplus