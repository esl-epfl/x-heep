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