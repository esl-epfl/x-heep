// Copyright 2025 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

// Author: Francesco Poluzzi

#include <stdio.h>
#include <stdlib.h>
#include "core_v_mini_mcu.h"
#include "add_two_number_regs.h"

#include "mmio.h"

int main(int argc, char *argv[])
{
    printf("app started!\n");

    mmio_region_t add_two_number_base_addr = mmio_region_from_addr((uintptr_t)ADD_TWO_NUMBER_START_ADDRESS);

    uint32_t operand0 = 27;
    uint32_t operand1 = 30;

    // write inputs
    mmio_region_write32(add_two_number_base_addr, ADD_TWO_NUMBER_ADD_TWO_NUMBER_OPERAND0_REG_OFFSET ,operand0);
    mmio_region_write32(add_two_number_base_addr, ADD_TWO_NUMBER_ADD_TWO_NUMBER_OPERAND1_REG_OFFSET ,operand1);
    // set start bit
    mmio_region_write32(add_two_number_base_addr, ADD_TWO_NUMBER_ADD_TWO_NUMBER_CTRL_REG_OFFSET , 1);
    // read_result
    uint32_t result = mmio_region_read32(add_two_number_base_addr, ADD_TWO_NUMBER_ADD_TWO_NUMBER_RESULT_REG_OFFSET);

    printf("%d + %d =: %d\n", operand0, operand1, result);

    return EXIT_SUCCESS;
}
