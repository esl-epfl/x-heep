// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "ams_regs.h"
#include "mmio.h"


int main(int argc, char *argv[])
{
        unsigned int AMS_START_ADDRESS = EXT_PERIPHERAL_START_ADDRESS + 0x30000;
        mmio_region_t ams_base_addr = mmio_region_from_addr((uintptr_t)AMS_START_ADDRESS);
//        uint32_t response;

        printf("0");
        mmio_region_write32(ams_base_addr, AMS_SEL_REG_OFFSET, 0);

        printf("1");
        mmio_region_write32(ams_base_addr, AMS_SEL_REG_OFFSET, 1);

        printf("2");
        mmio_region_write32(ams_base_addr, AMS_SEL_REG_OFFSET, 2);

        printf("3");
        mmio_region_write32(ams_base_addr, AMS_SEL_REG_OFFSET, 3);

//        response = mmio_region_read32(ams_base_addr, AMS_GET_REG_OFFSET);

        printf("END");

        return EXIT_SUCCESS;
}

