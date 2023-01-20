// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Modified version for core-v-mini-mcu
// original at: https://github.com/lowRISC/opentitan/blob/master/sw/

#ifndef GPIO_H_
#define GPIO_H_

/**
 * @file
 * @brief <a href="/hw/ip/gpio/doc/">GPIO</a> Device Interface Functions
 */

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

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
