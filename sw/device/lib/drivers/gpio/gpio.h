/*
                              *******************
******************************* H SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : X-HEEP                                                       **
** filename : gpio.h                                             **
**                                                                         **
*****************************************************************************
**
** Copyright (c) EPFL contributors.
** All rights reserved.
**
*****************************************************************************
*/

/***************************************************************************/
/***************************************************************************/
/**
* @file     gpio.h
* @date     30/03/2023
* @author   Hossein taji
* @version  1
* @brief    GPIO driver
*/

#ifndef GPIO_H_
#define GPIO_H_

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

/**
 * The format use for pin number. As the gpio pins are only 32 number,
 * uint8_t would be enough.
 */
typedef uint8_t gpio_pin_number_t;

/**
 * gpio mode.
 * 0 configure GPIO as input. 1 configures GPIO as a push-pull output.
 * 2 configures the GPIO to be in open_drain0 (0 ->  High-Z, 1 -> Drive High)
 * mode. 3 configures the GPIO to be in open_drain1 (0 -> Drive Low, 1 ->
 * High-Z) mode.
 */
typedef enum gpio_mode
{
    GpioModeIn            = 0,  /*!< input. */
    GpioModeOutPushPull   = 1,  /*!< push-pull output. */
    GpioModeoutOpenDrain0 = 2,  /*!< open_drain0 (0->High-Z, 1->Drive High)
    mode. */
    GpioModeoutOpenDrain1 = 3,  /*!< open_drain1 (0->Drive Low, 1->High-Z)
    mode. */
} gpio_mode_t;

/**
 * gpio_intr_general_mode
 * 1 keep the interrupt line asserted until all interrupts for all GPIOs
 * are cleared.
 * 0 generate one cycle wide pulses for every new interrupt.
 */
typedef enum gpio_intr_general_mode
{
    IntrOnePulse    = 0, /*!< keep the interrupt line asserted until all
    interrupts for all GPIOs are cleared. */
    IntrAsserted    = 1, /*!< generate one cycle wide pulses for every
    new interrupt. */
} gpio_intr_general_mode_t;

/**
 * This type is used in almost all the operations, and it is returned when
 * there is a problem with functions given parameters or operation for example
 * pin number not being in the range of accepting pins.
 */
typedef enum gpio_result
{
    GpioOk = 0,     /*!< The operation was ok. */
    GpioError = 1,  /*!< There is a problem. */
    GpioPinNotAcceptable = 2, /*!< Given pin is not inside of acceptable range. */
    GpioModeNotAcceptable = 3, /*!< Given pin mode is not one of the defined modes
    in gpio_mode_t. */
    GpioIntrTypeNotAcceptable = 4, /*!< Given interrupt type is not one of the
    defined types in gpio_intr_type_t. */
} gpio_result_t;

/**
 * The different interrupt types can be set by user
 */
typedef enum gpio_intr_type
{
    GpioIntrEdgeRising,         /*!< Trigger on rising edge. */
    GpioIntrEdgeFalling,        /*!< Trigger on falling edge. */
    GpioIntrLevelLow,           /*!< Trigger when input is low. */
    GpioIntrLevelHigh,          /*!< rigger when input is high. */
    GpioIntrEdgeRisingFalling,  /*!< Trigger on rising and falling edges. */
    GpioIntrEdgeRisingLevelLow, /*!< Trigger on rising edge or when the
    input is low. */
    GpioIntrEdgeFallingLevelHigh,/*!< Trigger on falling edge or when the
    input is high. */
} gpio_intr_type_t;

/**
 * The structure should be used by user for configuring gpio.
 * it includes pin number, mode, enable sampling, enable intr, and its mode.
 */
typedef struct gpio_cfg
{
    gpio_pin_number_t pin;      /*!< pin number. */
    gpio_mode_t mode;     /*!< pin mode. */
    bool en_input_sampling;     /*!< enable sampling (being input is req). */
    bool en_intr;               /*!< enable intr (being input is req). */
    gpio_intr_type_t intr_type; /*!< intr type (enabling intr is req). */
} gpio_cfg_t;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Adds a handler function for a gpio interrupt to the handlers list.
 * @param intr_id The interrupt ID of a gpio interrupt (from core_v_mini_mcu.h)
 * @param handler A pointer to a function that will be called upon interrupt.
 * @return The result of the operation
 */
gpio_result_t gpio_assign_irq_handler( uint32_t intr_id,
                                       void (*handler)() );

/**
 * @brief Resets all handlers to the dummy handler.
 */
void gpio_reset_handlers_list( );

/**
 * @brief Attends the plic interrupt.
 */
void handler_irq_gpio( uint32_t id );

/**
 * @brief gpio configuration. It first reset the pin configuration.
 * @param gpio_struct_t contatining pin, mode, en_input_sampling, en_intr,
 * intr_type
 * @return GpioOk: no problem, GpioError: there is an error.
 */
gpio_result_t gpio_config (gpio_cfg_t cfg);

/**
 * @brief reset completely all the configurations set for the pin
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_reset (gpio_pin_number_t pin);

/**
 * @brief reset completely all the configurations for all the pins
 */
gpio_result_t gpio_reset_all (void);

/**
 * @brief setting the a pins mode by writing in GPIO_MODE0 and GPIO_MODE1.
 * @param gpio_pin_number_t specify the pin.
 * @param gpio_mode_t specify pin mode: 0 as input, 1 push-pull as output,
 * 2 as open_drain0, and 3 as open_drain1.
 */
gpio_result_t gpio_set_mode (gpio_pin_number_t pin, gpio_mode_t mode);

/**
 * @brief enable sampling as input by writing one in GPIO_EN. If disables
 * (0) the corresponding GPIO will not sample the inputs (saves power) and
 * will not generate any interrupts.
 */
gpio_result_t gpio_en_input_sampling (gpio_pin_number_t pin);

/**
 * @brief disable sampling as input by writing zero in GPIO_EN.
 */
gpio_result_t gpio_dis_input_sampling (gpio_pin_number_t pin);

/**
 * @brief reading from a gpio pin, which is done by reading from GPIO_IN reg
 * @param gpio_pin_number_t specify pin number
 * @return gpio value as GpioLow (false) or GpioHigh (true)
 */
gpio_result_t gpio_read (gpio_pin_number_t pin, bool *val);

/**
 * @brief toggle a pin. Using masking through GPIO_TOGGLE, and then
 * writing to GPIO_OUT
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_toggle (gpio_pin_number_t pin);

/**
 * @brief write to a pin. using gpio_set and gpio_clear functions.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_write (gpio_pin_number_t pin, bool val);

/**
 * @brief enable rising edge interrupt for the given pin.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_en_rise (gpio_pin_number_t pin);

/**
 * @brief disable rising edge interrupt for the given pin.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_dis_rise (gpio_pin_number_t pin);

/**
 * @brief enable falling edge interrupt for the given pin.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_en_fall (gpio_pin_number_t pin);

/**
 * @brief disable falling edge interrupt for the given pin.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_dis_fall (gpio_pin_number_t pin);

/**
 * @brief enable logic-high level-sensitive interrupt for the given pin.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_en_lvl_high (gpio_pin_number_t pin);

/**
 * @brief disable logic-high level-sensitive interrupt for the given pin.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_dis_lvl_high (gpio_pin_number_t pin);

/**
 * @brief enable logic-low level-sensitive interrupt for the given pin.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_en_lvl_low (gpio_pin_number_t pin);

/**
 * @brief disable logic-low level-sensitive interrupt for the given pin.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_dis_lvl_low (gpio_pin_number_t pin);

/**
 * @brief enable the interrupt for the given pin. Type of interrupt
 * specified by user.
 * @param gpio_pin_number_t specify pin number
 * @param gpio_intr_type_t specify the type of the interrupt
 */
gpio_result_t gpio_intr_en (gpio_pin_number_t pin, gpio_intr_type_t type);

/**
 * @brief disable all the types of interrupt on the given pin.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_dis_all (gpio_pin_number_t pin);

/**
 * @brief Each bit indicates if there is a pending rising-edge interrupt
 * on the corresponding GPIO.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_check_stat_rise (gpio_pin_number_t pin, bool *is_pending);

/**
 * @brief Each bit indicates if there is a pending falling-edge interrupt
 * on the corresponding GPIO.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_check_stat_fall (gpio_pin_number_t pin, bool *is_pending);

/**
 * @brief Each bit indicates if there is a pending low level-sensitive
 * interrupt on the corresponding GPIO.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_check_stat_lvl_low (gpio_pin_number_t pin, bool *is_pending);

/**
 * @brief Each bit indicates if there is a pending high level-sensitive
 * interrupt on the corresponding GPIO.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_check_stat_lvl_high (gpio_pin_number_t pin, bool *is_pending);

/**
 * @brief Each bit indicates if there is a pending interrupt on the
 * corresponding GPIO in any form (rise, fall, low level, and high level)
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_check_stat (gpio_pin_number_t pin, bool *is_pending);

/**
 * @brief Clearing interrupt rise status. Writing a 1 to a specific bit
 * clears the interrupt for the corresponding GPIO.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_clear_stat_rise (gpio_pin_number_t pin);

/**
 * @brief Clearing interrupt fall status. Writing a 1 to a specific bit
 * clears the interrupt for the corresponding GPIO.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_clear_stat_fall (gpio_pin_number_t pin);

/**
 * @brief Clearing interrupt low level-sensitive status. Writing a 1 to a
 * specific bit clears the interrupt for the corresponding GPIO.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_clear_stat_lvl_low (gpio_pin_number_t pin);

/**
 * @brief Clearing interrupt high level-sensitive status. Writing a 1 to a
 * specific bit clears the interrupt for the corresponding GPIO.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_clear_stat_lvl_high (gpio_pin_number_t pin);

/**
 * @brief Clearing interrupt status regardless of its type. Writing a 1 to
 * a specific bit clears the interrupt for the corresponding GPIO.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_intr_clear_stat (gpio_pin_number_t pin);

/**
 * @brief Controls the interrupt mode of the gpios.
 * @gpio_intr_mode If true, keep the interrupt line asserted until all
 * interrupts for all GPIOs are cleared. If false, generate one cycle wide
 * pulses for every new interrupt.
 */
void gpio_intr_set_mode (gpio_pin_number_t pin, gpio_intr_general_mode_t mode);

#endif  // _GPIO_H_
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/