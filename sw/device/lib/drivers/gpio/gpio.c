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
 * Perform a masked write to a GPIO register.
 *
 * The GPIO device provides masked bit-level atomic writes to its DIRECT_OUT
 * and DIRECT_OE registers. This allows software to modify half of the bits
 * at a time without requiring a read-modify-write. Note that depending on the
 * value of the `mask`, this function may perform two writes.
 *
 * For instance, DIRECT_OUT's lower and upper halves can be modified by
 * MASKED_OUT_LOWER and MASKED_OUT_UPPER, respectively. Upper half of
 * MASKED_OUT_LOWER is the mask that determines which bits in the lower half of
 * DIRECT_OUT will be modified, while lower half of MASKED_OUT_LOWER determines
 * their values. MASKED_OUT_UPPER behaves in the same way for modifying the
 * upper half of DIRECT_OUT.
 *
 * @param gpio GPIO instance.
 * @param reg_lower_offset Offset of the masked access register that corresponds
 * to the lower half of the actual register.
 * @param reg_upper_offset Offset of the masked access register that corresponds
 * to the upper half of the actual register.
 * @param mask Mask that identifies the bits to write to.
 * @param val Value to write.
 */
static gpio_result_t gpio_masked_write(const gpio_t *gpio,
                                           ptrdiff_t reg_lower_offset,
                                           ptrdiff_t reg_upper_offset,
                                           uint32_t mask, uint32_t val) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  const uint32_t mask_lower_half = mask & 0x0000FFFFu;
  const uint32_t mask_upper_half = mask & 0xFFFF0000u;
  if (mask_lower_half != 0) {
    mmio_region_write32(gpio->params.base_addr, reg_lower_offset,
                        (mask_lower_half << 16) | (val & 0x0000FFFFu));
  }
  if (mask_upper_half != 0) {
    mmio_region_write32(gpio->params.base_addr, reg_upper_offset,
                        mask_upper_half | ((val & 0xFFFF0000u) >> 16));
  }

  return kGpioOk;
}

/**
 * Perform a masked write to a single bit of a GPIO register.
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
                                               ptrdiff_t reg_lower_offset,
                                               ptrdiff_t reg_upper_offset,
                                               uint32_t index, bool val) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  // Write to reg_lower_offset if the bit is in the lower half, write to
  // reg_upper_offset otherwise.
  const ptrdiff_t offset = (index < 16) ? reg_lower_offset : reg_upper_offset;
  // Since masked access writes to half of a register, index mod 16 gives the
  // index of the bit in the half-word mask.
  const uint32_t mask = index_to_mask(index % 16);
  mmio_region_write32(gpio->params.base_addr, offset,
                      (mask << 16) | (val ? mask : 0u));

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
  mmio_region_write32(gpio->params.base_addr, GPIO_INTR_ENABLE_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr, GPIO_DIRECT_OE_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr, GPIO_DIRECT_OUT_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTR_CTRL_EN_RISING_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTR_CTRL_EN_FALLING_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTR_CTRL_EN_LVLHIGH_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_INTR_CTRL_EN_LVLLOW_REG_OFFSET, 0);
  mmio_region_write32(gpio->params.base_addr,
                      GPIO_CTRL_EN_INPUT_FILTER_REG_OFFSET, 0);
  // Also clear all pending interrupts
  mmio_region_write32(gpio->params.base_addr, GPIO_INTR_STATE_REG_OFFSET,
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
                                      GPIO_INTR_STATE_REG_OFFSET, pin);

  return kGpioOk;
}

gpio_result_t gpio_irq_is_pending_all(const gpio_t *gpio,
                                              gpio_state_t *is_pending) {
  if (gpio == NULL || is_pending == NULL) {
    return kGpioBadArg;
  }

  *is_pending =
      mmio_region_read32(gpio->params.base_addr, GPIO_INTR_STATE_REG_OFFSET);

  return kGpioOk;
}

gpio_result_t gpio_irq_acknowledge(const gpio_t *gpio,
                                           gpio_pin_t pin) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_INTR_STATE_REG_OFFSET,
                      index_to_mask(pin));

  return kGpioOk;
}

gpio_result_t gpio_irq_get_enabled(const gpio_t *gpio,
                                           gpio_pin_t pin,
                                           gpio_toggle_t *state) {
  if (gpio == NULL || state == NULL) {
    return kGpioBadArg;
  }

  bool is_enabled = mmio_region_get_bit32(gpio->params.base_addr,
                                          GPIO_INTR_ENABLE_REG_OFFSET, pin);
  *state = is_enabled ? kGpioToggleEnabled : kGpioToggleDisabled;

  return kGpioOk;
}

gpio_result_t gpio_irq_set_enabled(const gpio_t *gpio,
                                           gpio_pin_t pin,
                                           gpio_toggle_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  switch (state) {
    case kGpioToggleEnabled:
      mmio_region_nonatomic_set_bit32(gpio->params.base_addr,
                                      GPIO_INTR_ENABLE_REG_OFFSET, pin);
      break;
    case kGpioToggleDisabled:
      mmio_region_nonatomic_clear_bit32(gpio->params.base_addr,
                                        GPIO_INTR_ENABLE_REG_OFFSET, pin);
      break;
    default:
      return kGpioBadArg;
  }

  return kGpioOk;
}

gpio_result_t gpio_irq_set_enabled_masked(const gpio_t *gpio,
                                                  gpio_mask_t mask,
                                                  gpio_toggle_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  switch (state) {
    case kGpioToggleEnabled:
      mmio_region_nonatomic_set_mask32(gpio->params.base_addr,
                                       GPIO_INTR_ENABLE_REG_OFFSET, mask, 0);
      break;
    case kGpioToggleDisabled:
      mmio_region_nonatomic_clear_mask32(gpio->params.base_addr,
                                         GPIO_INTR_ENABLE_REG_OFFSET, mask, 0);
      break;
    default:
      return kGpioBadArg;
  }

  return kGpioOk;
}

gpio_result_t gpio_irq_force(const gpio_t *gpio,
                                     gpio_pin_t pin) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_INTR_TEST_REG_OFFSET,
                      index_to_mask(pin));

  return kGpioOk;
}

gpio_result_t gpio_irq_disable_all(const gpio_t *gpio,
                                           gpio_state_t *snapshot) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  if (snapshot != NULL) {
    *snapshot =
        mmio_region_read32(gpio->params.base_addr, GPIO_INTR_ENABLE_REG_OFFSET);
  }
  mmio_region_write32(gpio->params.base_addr, GPIO_INTR_ENABLE_REG_OFFSET, 0);

  return kGpioOk;
}

gpio_result_t gpio_irq_restore_all(const gpio_t *gpio,
                                           const gpio_state_t *snapshot) {
  if (gpio == NULL || snapshot == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_INTR_ENABLE_REG_OFFSET,
                      *snapshot);

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
  return gpio_masked_bit_write(gpio, GPIO_MASKED_OUT_LOWER_REG_OFFSET,
                               GPIO_MASKED_OUT_UPPER_REG_OFFSET, pin, state);
}

gpio_result_t gpio_write_masked(const gpio_t *gpio,
                                        gpio_mask_t mask,
                                        gpio_state_t state) {
  return gpio_masked_write(gpio, GPIO_MASKED_OUT_LOWER_REG_OFFSET,
                           GPIO_MASKED_OUT_UPPER_REG_OFFSET, mask, state);
}

gpio_result_t gpio_output_set_enabled_all(const gpio_t *gpio,
                                                  gpio_state_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  mmio_region_write32(gpio->params.base_addr, GPIO_DIRECT_OE_REG_OFFSET, state);

  return kGpioOk;
}

gpio_result_t gpio_output_set_enabled(const gpio_t *gpio,
                                              gpio_pin_t pin,
                                              gpio_toggle_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  return gpio_masked_bit_write(gpio, GPIO_MASKED_OE_LOWER_REG_OFFSET,
                               GPIO_MASKED_OE_UPPER_REG_OFFSET, pin, state);

  return kGpioOk;
}

gpio_result_t gpio_output_set_enabled_masked(const gpio_t *gpio,
                                                     gpio_mask_t mask,
                                                     gpio_state_t state) {
  return gpio_masked_write(gpio, GPIO_MASKED_OE_LOWER_REG_OFFSET,
                           GPIO_MASKED_OE_UPPER_REG_OFFSET, mask, state);
}

gpio_result_t gpio_input_noise_filter_set_enabled(
    const gpio_t *gpio, gpio_mask_t mask, gpio_toggle_t state) {
  if (gpio == NULL) {
    return kGpioBadArg;
  }

  switch (state) {
    case kGpioToggleEnabled:
      mmio_region_nonatomic_set_mask32(gpio->params.base_addr,
                                       GPIO_CTRL_EN_INPUT_FILTER_REG_OFFSET,
                                       mask, 0);
      break;
    case kGpioToggleDisabled:
      mmio_region_nonatomic_clear_mask32(gpio->params.base_addr,
                                         GPIO_CTRL_EN_INPUT_FILTER_REG_OFFSET,
                                         mask, 0);
      break;
    default:
      return kGpioBadArg;
  }

  return kGpioOk;
}
