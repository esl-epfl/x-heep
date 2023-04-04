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

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

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
    GpioModeoutOpenDrain0 = 2,  /*!< open_drain0 (0->High-Z, 1->Drive High) mode. */
    GpioModeoutOpenDrain1 = 3,  /*!< open_drain1 (0->Drive Low, 1->High-Z) mode. */
} gpio_mode_t;

/**
 * This type is used in almost all the operations, and it is returned when
 * there is a problem with functions given parameters or operation for example 
 * pin number not being in the range of accepting pins.
 */
typedef enum gpio_result 
{
    GpioOk = 0,     /*!< The operation was ok. */
    GpioError = 1,  /*!< There is a problem. */
} gpio_result_t;

// /**
//  * Gpio pin values: low or high
//  */
// typedef enum gpio_value
// {
//     GpioLow = 0, 
//     GpioHigh = 1, 
// } gpio_value_t;

/**
 * 
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


/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/


/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/


/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

/**
 * @brief setting the a pins mode by writing in GPIO_MODE0 and GPIO_MODE1.
 * @param gpio_pin_number_t specify the pin.
 * @param gpio_mode_t specify pin mode: 0 as input, 1 push-pull as output, 
 * 2 as open_drain0, and 3 as open_drain1.
 * @retval GpioOk if there is no problem with the given parameters or 
 * operation.
 * @retval GpioError if there is an error with the given parameters or 
 * operation.
 */
gpio_result_t gpio_set_mode (gpio_pin_number_t pin, gpio_mode_t mode);

/**
 * @brief enable sampling as input by writing one in GPIO_EN. If disables 
 * (0) the corresponding GPIO will not sample the inputs (saves power) and 
 * will not generate any interrupts.
 */
gpio_result_t gpio_en_input (gpio_pin_number_t pin);

/**
 * @brief disable sampling as input by writing zero in GPIO_EN.
 */
gpio_result_t gpio_dis_input (gpio_pin_number_t pin);

/**
 * @brief reset completely all the configurations set for the pin
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_pin_reset (gpio_pin_number_t pin);

/**
 * @brief reset completely all the configurations for all the pins
 */
gpio_result_t gpio_reset_all (void);


/**
 * @brief reading from a gpio pin, which is done by reading from GPIO_IN reg
 * @param gpio_pin_number_t specify pin number
 * @return gpio value as GpioLow (false) or GpioHigh (true)
 */
gpio_result_t gpio_pin_read (gpio_pin_number_t pin);

// /**
//  * @brief set a pin as high. Using masking through GPIO_SET, and then writing
//  * to GPIO_OUT 
//  * @param gpio_pin_number_t specify pin number
//  */
// gpio_result_t gpio_pin_set (gpio_pin_number_t pin);

// /**
//  * @brief clear a pin as low. Using masking through GPIO_CLEAR, and then 
//  * writing to GPIO_OUT 
//  * @param gpio_pin_number_t specify pin number
//  */
// gpio_result_t gpio_pin_clear (gpio_pin_number_t pin);

/**
 * @brief toggle a pin. Using masking through GPIO_TOGGLE, and then 
 * writing to GPIO_OUT 
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_pin_toggle (gpio_pin_number_t pin);

/**
 * @brief write to a pin. using gpio_set and gpio_clear functions.
 * @param gpio_pin_number_t specify pin number
 */
gpio_result_t gpio_pin_write (gpio_pin_number_t pin, gpio_value val);

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

// /**
//  * @brief disable the given type of interrupt on the given pin.
//  * @param gpio_pin_number_t specify pin number
//  */
// gpio_result_t gpio_intr_dis (gpio_pin_number_t pin, gpio_intr_type_t type);

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
gpio_result_t gpio_intr_mode (bool gpio_intr_mode);





/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/



/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  // _GPIO_H_






/**
 * A toggle state: enabled, or disabled.
 *
 * This enum may be used instead of a `bool` when describing an enabled/disabled
 * state.
 *
 * This enum may be used with `gpio_toggle_vec_t` to set individual bits
 * within it; `gpio_toggle_t`'s variants are guaranteed to be compatible
 * with `gpio_toggle_vec_t`.
 */
typedef enum gpio_toggle {
  /*
   * The "enabled" state.
   */
  kGpioToggleEnabled = true,
  /**
   * The "disabled" state.
   */
  kGpioToggleDisabled = false,
} gpio_toggle_t;

/**
 * Hardware instantiation parameters for GPIO.
 *
 * This struct describes information about the underlying hardware that is
 * not determined until the hardware design is used as part of a top-level
 * design.
 */
typedef struct gpio_params {
  /**
   * The base address for the GPIO hardware registers.
   */
  mmio_region_t base_addr;
} gpio_params_t;

/**
 * A handle to GPIO.
 *
 * This type should be treated as opaque by users.
 */
typedef struct gpio { gpio_params_t params; } gpio_t;

/**
 * The result of a GPIO operation.
 */
typedef enum gpio_result {
  /**
   * Indicates that the operation succeeded.
   */
  kGpioOk = 0,
  /**
   * Indicates some unspecified failure.
   */
  kGpioError = 1,
  /**
   * Indicates that some parameter passed into a function failed a
   * precondition.
   *
   * When this value is returned, no hardware operations occurred.
   */
  kGpioBadArg = 2,
} gpio_result_t;

/**
 * A GPIO interrupt request trigger.
 *
 * Each GPIO pin has an associated interrupt that can be independently
 * configured
 * to be edge and/or level sensitive. This enum defines supported configurations
 * for
 * these interrupts.
 */
typedef enum gpio_irq_trigger {
  /**
   * Trigger on rising edge.
   */
  kGpioIrqTriggerEdgeRising,
  /**
   * Trigger on falling edge.
   */
  kGpioIrqTriggerEdgeFalling,
  /**
   * Trigger when input is low.
   */
  kGpioIrqTriggerLevelLow,
  /**
   * Trigger when input is high.
   */
  kGpioIrqTriggerLevelHigh,
  /**
   * Trigger on rising and falling edges.
   */
  kGpioIrqTriggerEdgeRisingFalling,
  /**
   * Trigger on rising edge or when the input is low.
   */
  kGpioIrqTriggerEdgeRisingLevelLow,
  /**
   * Trigger on falling edge or when the input is high.
   */
  kGpioIrqTriggerEdgeFallingLevelHigh,
} gpio_irq_trigger_t;

/**
 * A GPIO pin index, ranging from 0 to 31.
 *
 * This type serves as the GPIO interrupt request type.
 */
typedef uint32_t gpio_pin_t;

/**
 * State for all 32 GPIO pins, given as bit fields.
 *
 * The Nth bit represents the state of the Nth pin.
 *
 * This type is also used as a vector of `gpio_toggle_t`s, to indicate
 * toggle state across all 32 pins. A set bit corresponds to
 * `kGpioToggleEnabled`.
 *
 * It is also used with `gpio_irq_disable_all()` and
 * `gpio_irq_restore_all()`.
 */
typedef uint32_t gpio_state_t;

/**
 * A mask for selecting GPIO pins.
 *
 * If the Nth bit is enabled, then the Nth pin is selected by the mask.
 */
typedef uint32_t gpio_mask_t;

/**
 * Creates a new handle for GPIO.
 *
 * This function does not actuate the hardware.
 *
 * @param params Hardware instantiation parameters.
 * @param[out] gpio Out param for the initialized handle.
 * @return The result of the operation.
 */
gpio_result_t gpio_init(gpio_params_t params, gpio_t *gpio);

/**
 * Resets a GPIO device.
 *
 * Resets the given GPIO device by setting its configuration registers to
 * reset values. Disables interrupts, output, and input filter.
 *
 * @param gpio A GPIO handle.
 * @return The result of the operation.
 */
gpio_result_t gpio_reset(const gpio_t *gpio);

/**
 * Returns whether a particular pin's interrupt is currently pending.
 *
 * @param gpio A GPIO handle.
 * @param pin A GPIO pin.
 * @param[out] is_pending Out-param for whether the interrupt is pending.
 * @return The result of the operation.
 */
gpio_result_t gpio_irq_is_pending(const gpio_t *gpio,
                                          gpio_pin_t pin, bool *is_pending);

/**
 * Returns a GPIO state representing which pins have interrupts enabled.
 *
 * @param gpio A GPIO handle.
 * @param[out] is_pending Out-param for which interrupts are pending.
 * @return The result of the operation.
 */
gpio_result_t gpio_irq_is_pending_all(const gpio_t *gpio,
                                              gpio_state_t *is_pending);

/**
 * Acknowledges a particular pin's interrupt, indicating to the hardware that it
 * has
 * been successfully serviced.
 *
 * @param gpio A GPIO handle.
 * @param pin A GPIO pin.
 * @return The result of the operation.
 */
gpio_result_t gpio_irq_acknowledge(const gpio_t *gpio,
                                           gpio_pin_t pin);

/**
 * Configures interrupt triggers for a set of pins.
 *
 * This function configures interrupt triggers, i.e. rising-edge, falling-edge,
 * level-high, and level-low, for the pins given by the mask. Note that
 * interrupt of the pin must also be enabled to generate interrupts.
 *
 * @param gpio A GPIO handle.
 * @param mask Mask that identifies the pins whose interrupt triggers will be
 * configured.
 * @param trigger New configuration of interrupt triggers.
 * @return The result of the operation.
 */
gpio_result_t gpio_irq_set_trigger(const gpio_t *gpio,
                                           gpio_pin_t pin,
                                           bool state,
                                           gpio_irq_trigger_t trigger);

/**
 * Reads from a pin.
 *
 * The value returned by this function is independent of the output enable
 * setting and includes the effects of the input noise filter and the load on
 * the pin.
 *
 * @param gpio A GPIO handle.
 * @param pin A GPIO pin.
 * @param[out] state Pin value.
 * @return The result of the operation.
 */
gpio_result_t gpio_read(const gpio_t *gpio, gpio_pin_t pin,
                                bool *state);

/**
 * Writes to a pin.
 *
 * The actual value on the pin depends on the output enable setting.
 *
 * @param gpio A GPIO handle.
 * @param pin A GPIO pin.
 * @param state Value to write.
 * @return The result of the operation.
 */
gpio_result_t gpio_write(const gpio_t *gpio, gpio_pin_t pin,
                                 bool state);
/**
 * Sets output modes of all pins.
 *
 * @param gpio A GPIO handle.
 * @param pin A GPIO pin.
 * @param state Output modes of the pins.
 * @return The result of the operation.
 */
gpio_result_t gpio_input_set_enabled(const gpio_t *gpio,
                                                  gpio_pin_t pin,
                                                  gpio_state_t state);
/**
 * Enable sampling on GPIO
 * *
 * @param gpio A GPIO handle.
 * @param pin A GPIO pin.
 * @param state Value to write.
 */
gpio_result_t gpio_input_enabled(const gpio_t *gpio,
                                          gpio_pin_t pin, gpio_toggle_t state);


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // GPIO_H_
