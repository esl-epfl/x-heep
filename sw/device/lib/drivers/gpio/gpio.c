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

/**
 * @brief get some bitfield of a 32b reg.
 * @param reg the register that bitwise field selects from.
 * @param mask the mask w/o offset.
 * @param sel this the offset used for mask and value.
 */
static inline uint32_t getBitfield( uint32_t reg, uint32_t  mask, uint8_t sel )
{
    uint32_t ret_val = 0;
    ret_val = reg & ( mask << sel );
    ret_val = ret_val >> sel;
    return ret_val;
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
        setBitfield(&(gpio_peri->GPIO_MODE1), mode, 0b11, 2*(pin-16));
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

gpio_result_t gpio_reset (gpio_pin_number_t pin)
{
    if (pin >= 0 && pin < 32)
    {
        gpio_intr_set_mode (0);
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

gpio_result_t gpio_read (gpio_pin_number_t pin, bool *val)
{
    if (pin >= 0 && pin < 32)
    {
        if ( getBitfield(gpio_peri->GPIO_IN0, 0b1, pin) == 0b1)
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
gpio_result_t gpio_toggle (gpio_pin_number_t pin)
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

gpio_result_t gpio_write (gpio_pin_number_t pin, bool val)
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

gpio_result_t gpio_intr_set_mode (bool gpio_intr_mode)
{
    if (gpio_intr_mode)
        setBitfield(&(gpio_peri->CFG), 1, 1, 0);
    else
        setBitfield(&(gpio_peri->CFG), 0, 1, 0);
    return GpioOk;
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