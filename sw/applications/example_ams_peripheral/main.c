// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "ams_regs.h"
#include "mmio.h"

#ifdef TARGET_IS_FPGA
  #error ( "This app does NOT work on the FPGA as it relies on the simulator testbench" )
#endif

int main(int argc, char *argv[])
{
        unsigned int AMS_START_ADDRESS = EXT_PERIPHERAL_START_ADDRESS + 0x1000;
        mmio_region_t ams_base_addr = mmio_region_from_addr((uintptr_t)AMS_START_ADDRESS);

        printf("0");
        mmio_region_write32(ams_base_addr, AMS_SEL_REG_OFFSET, 0);

        printf("1");
        mmio_region_write32(ams_base_addr, AMS_SEL_REG_OFFSET, 1);

        for (int i = 0 ; i < 10000 ; i++) {
                asm volatile ("nop");
        }

        printf("2");
        mmio_region_write32(ams_base_addr, AMS_SEL_REG_OFFSET, 2);

        for (int i = 0 ; i < 10000 ; i++) {
                asm volatile ("nop");
        }

        printf("3");
        mmio_region_write32(ams_base_addr, AMS_SEL_REG_OFFSET, 3);


        printf("END\n");

        return EXIT_SUCCESS;
}

