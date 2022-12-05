// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "fast_intr_ctrl.h"

#include <stddef.h>
#include <stdint.h>

#include "mmio.h"

#include "core_v_mini_mcu.h"

#include "fast_intr_ctrl_regs.h"  // Generated.

fast_intr_ctrl_result_t clear_fast_interrupt(fast_intr_ctrl_t* fast_intr_ctrl, fast_intr_ctrl_fast_interrupt_t fast_interrupt)
{
    uint32_t reg = 0;

    reg = bitfield_bit32_write(reg, fast_interrupt, true);
    mmio_region_write32(fast_intr_ctrl->base_addr, (ptrdiff_t)(FAST_INTR_CTRL_FAST_INTR_CLEAR_REG_OFFSET), reg);

    return kFastIntrCtrlOk_e;
}
