// Copyright lowRISC contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

// Modified version for core-v-mini-mcu
// original at: https://github.com/lowRISC/opentitan/blob/master/sw/

#include "gpio.h"

#include "gpio_regs.h"  // Generated.

/**
 * Gives the mask that corresponds to the given bit index.
 *
 * @param index Bit index in a 32-bit register.
 */
static uint32_t index_to_mask(uint32_t index) { return 1u << index; }

/**
 * Perform a masked write to a single bit of a GPIO register. (For the new GPIO)
 *
 * The GPIO device provides masked bit-level atomic writes to its DIRECT_OUT
 * and DIRECT_OE registers. This allows software to modify half of the bits
 * at a time without requiring a read-modify-write. This function is guaranteed
 * to perform only one write since it never needs to access both halves of a
 * register.
 *
 * See also `gpio_masked_write()`.
 *
 * @param gpio GPIO instance.
 * @param reg_lower_offset Offset of the masked access register that corresponds
 * to the lower half of the actual register.
 * @param reg_upper_offset Offset of the masked access register that corresponds
 * to the upper half of the actual register.
 * @param index Zero-based index of the bit to write to.
 * @param val Value to write.
 */
static gpio_result_t gpio_masked_bit_write(const gpio_t *gpio,
                                               ptrdiff_t reg_offset,
                                               uint32_t index, bool val) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }
  uint32_t temp = mmio_region_read32(gpio->params.base_addr, reg_offset);
  if (val == true){
   uint32_t mask = index_to_mask(index);
   uint32_t mask_val = mask | temp;
   mmio_region_write32(gpio->params.base_addr, reg_offset,
                       mask_val);

  }else{
   uint32_t mask = index_to_mask(index);
   mask = ~mask;
   uint32_t mask_val = mask & temp;
   mmio_region_write32(gpio->params.base_addr, reg_offset,
                       mask_val);
  }

  return kGpioOk;
}

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

  // We don't need to write to `GPIO_MASKED_OE_*` and `GPIO_MASKED_OUT_*`
  // since we directly reset `GPIO_DIRECT_OE` and `GPIO_DIRECT_OUT` below.
  mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_EN_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr, GPIO_DATA_IN_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr, GPIO_DIRECT_OUT_REG_OFFSET, 0);
  // Clear all the interrupt enable
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTR_CTRL_EN_RISING_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTR_CTRL_EN_FALLING_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTR_CTRL_EN_LVLHIGH_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTR_CTRL_EN_LVLLOW_REG_OFFSET, 0);
  // Also clear all the interrupt
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTRPT_RISE_STATUS_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTRPT_FALL_STATUS_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTRPT_LVL_HIGH_STATUS_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTRPT_LVL_LOW_STATUS_OFFSET, 0);
  // Also clear all pending interrupts
  mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_STATUS_OFFSET,
                      0xFFFFFFFFu);

  return kGpioOk;
}

gpio_result_t gpio_irq_is_pending(const gpio_t *gpio,
                                          gpio_pin_t pin,
                                          bool *is_pending) {
  if (gpio == NULL || is_pending == NULL) {
    return kGpioBadArg;
  }

  *is_pending = mmio_region_get_bit32(gpio->params.base_addr,
                                      GPIO_INTRPT_STATUS_OFFSET, pin);

  return kGpioOk;
}

gpio_result_t gpio_irq_is_pending_all(const gpio_t *gpio,
                                              gpio_state_t *is_pending) {
  if (gpio == NULL || is_pending == NULL) {
    return kGpioBadArg;
  }

  *is_pending =
      mmio_region_read32(gpio->params.base_addr, GPIO_INTRPT_STATUS_OFFSET);

  return kGpioOk;
}

gpio_result_t gpio_irq_acknowledge(const gpio_t *gpio,
                                           gpio_pin_t pin) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_INTRPT_STATUS_OFFSET,
                      index_to_mask(pin));

  return kGpioOk;
}

gpio_result_t gpio_irq_set_trigger(const gpio_t *gpio,
                                           gpio_mask_t mask,
                                           gpio_irq_trigger_t trigger) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  // Disable all interrupt triggers for the given mask.
  mmio_region_nonatomic_clear_mask32(
      gpio->params.base_addr, GPIO_INTR_CTRL_EN_RISING_REG_OFFSET, mask, 0);
  mmio_region_nonatomic_clear_mask32(
      gpio->params.base_addr, GPIO_INTR_CTRL_EN_FALLING_REG_OFFSET, mask, 0);
  mmio_region_nonatomic_clear_mask32(
      gpio->params.base_addr, GPIO_INTR_CTRL_EN_LVLHIGH_REG_OFFSET, mask, 0);
  mmio_region_nonatomic_clear_mask32(
      gpio->params.base_addr, GPIO_INTR_CTRL_EN_LVLLOW_REG_OFFSET, mask, 0);

  switch (trigger) {
    case kGpioIrqTriggerEdgeRising:
      mmio_region_nonatomic_set_mask32(
          gpio->params.base_addr, GPIO_INTR_CTRL_EN_RISING_REG_OFFSET, mask, 0);
      break;
    case kGpioIrqTriggerEdgeFalling:
      mmio_region_nonatomic_set_mask32(gpio->params.base_addr,
                                       GPIO_INTR_CTRL_EN_FALLING_REG_OFFSET,
                                       mask, 0);
      break;
    case kGpioIrqTriggerLevelLow:
      mmio_region_nonatomic_set_mask32(
          gpio->params.base_addr, GPIO_INTR_CTRL_EN_LVLLOW_REG_OFFSET, mask, 0);
      break;
    case kGpioIrqTriggerLevelHigh:
      mmio_region_nonatomic_set_mask32(gpio->params.base_addr,
                                       GPIO_INTR_CTRL_EN_LVLHIGH_REG_OFFSET,
                                       mask, 0);
      break;
    case kGpioIrqTriggerEdgeRisingFalling:
      mmio_region_nonatomic_set_mask32(
          gpio->params.base_addr, GPIO_INTR_CTRL_EN_RISING_REG_OFFSET, mask, 0);
      mmio_region_nonatomic_set_mask32(gpio->params.base_addr,
                                       GPIO_INTR_CTRL_EN_FALLING_REG_OFFSET,
                                       mask, 0);
      break;
    case kGpioIrqTriggerEdgeRisingLevelLow:
      mmio_region_nonatomic_set_mask32(
          gpio->params.base_addr, GPIO_INTR_CTRL_EN_RISING_REG_OFFSET, mask, 0);
      mmio_region_nonatomic_set_mask32(
          gpio->params.base_addr, GPIO_INTR_CTRL_EN_LVLLOW_REG_OFFSET, mask, 0);
      break;
    case kGpioIrqTriggerEdgeFallingLevelHigh:
      mmio_region_nonatomic_set_mask32(gpio->params.base_addr,
                                       GPIO_INTR_CTRL_EN_FALLING_REG_OFFSET,
                                       mask, 0);
      mmio_region_nonatomic_set_mask32(gpio->params.base_addr,
                                       GPIO_INTR_CTRL_EN_LVLHIGH_REG_OFFSET,
                                       mask, 0);
      break;
    default:
      return kGpioError;
  }

  return kGpioOk;
}

gpio_result_t gpio_read_all(const gpio_t *gpio,
                                    gpio_state_t *state) {
  if (gpio == NULL || state == NULL) {
    return kGpioBadArg;
  }

  *state = mmio_region_read32(gpio->params.base_addr, GPIO_DATA_IN_REG_OFFSET);

  return kGpioOk;
}

gpio_result_t gpio_read(const gpio_t *gpio, gpio_pin_t pin,
                                bool *state) {
  if (gpio == NULL || state == NULL) {
    return kGpioBadArg;
  }

  *state = mmio_region_get_bit32(gpio->params.base_addr,
                                 GPIO_DATA_IN_REG_OFFSET, pin);

  return kGpioOk;
}

gpio_result_t gpio_write_all(const gpio_t *gpio,
                                     gpio_state_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_DIRECT_OUT_REG_OFFSET,
                      state);

  return kGpioOk;
}

gpio_result_t gpio_write(const gpio_t *gpio, gpio_pin_t pin,
                                 bool state) {
  if (state == true){
    return gpio_masked_bit_write(gpio, GPIO_GPIO_SET_OFFSET,
                               pin, state);  
  }else{ 
    return gpio_masked_bit_write(gpio, GPIO_GPIO_CLEAR_OFFSET,
                               pin, state); 
  }
}
gpio_result_t gpio_input_set_enabled_all(const gpio_t *gpio,
                                                  gpio_state_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_EN_OFFSET, state);

  return kGpioOk;
}

gpio_result_t gpio_input_set_enabled(const gpio_t *gpio,
                                                  gpio_pin_t pin,
                                                  gpio_state_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }
  gpio_masked_bit_write(gpio, GPIO_GPIO_EN_OFFSET,
                               pin, state);

  return kGpioOk;
}

gpio_result_t gpio_set(const gpio_t *gpio, gpio_state_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_SET_OFFSET,
                      state);

  return kGpioOk;
}

gpio_result_t gpio_clear(const gpio_t *gpio, gpio_state_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_CLEAR_OFFSET,
                      state);

  return kGpioOk;
}

gpio_result_t gpio_toggle(const gpio_t *gpio, gpio_state_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_TOGGLE_OFFSET,
                      state);

  return kGpioOk;
}


gpio_result_t gpio_set_mode(const gpio_t *gpio, gpio_pin_t pin,
                                 bool state1, bool state2) {
  if(pin < 16){
    gpio_masked_bit_write(gpio, GPIO_GPIO_MODE_0_OFFSET, 2*pin, state2);
    return gpio_masked_bit_write(gpio, GPIO_GPIO_MODE_0_OFFSET, 2*pin+1, state1);
  }else{
    gpio_masked_bit_write(gpio, GPIO_GPIO_MODE_1_OFFSET, 2*(pin-16), state2);
    return gpio_masked_bit_write(gpio, GPIO_GPIO_MODE_1_OFFSET, 2*(pin-16)+1, state1);
  }
}
gpio_result_t gpio_set_mode_all(const gpio_t *gpio,
                                 bool state1, bool state2) {
  mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_MODE_1_OFFSET,
                      state1);
  mmio_region_write32(gpio->params.base_addr, GPIO_GPIO_MODE_0_OFFSET,
                      state2);
  return kGpioOk;

}