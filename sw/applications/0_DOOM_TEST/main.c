/**
 * Copyright (c) 2016 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "core_v_mini_mcu.h"
#include "gpio.h"
#include "x-heep.h"


/*
#include "nrf.h"

#include "FT810.h"
#include "n_display.h"

#include "n_uart.h"
#include "n_qspi.h"
#include "n_fs.h"
#include "x_buttons.h"
#include "n_rjoy.h"
#include "n_i2s.h"

#include "nrf_delay.h"
#include "nrf_clock.h"
#include "nrf_gpio.h"
#include "nrfx_clock.h"

#include "board_config.h"
*/

#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#define WAD_START_ADDRESS 1024*1024


#include "x-heep.h"
#include "w25q128jw.h"

spi_host_t spi_flash;


//public function definitions
void X_init_spi()
{
    int error = 0;
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

        if ( get_spi_flash_mode(&soc_ctrl) == SOC_CTRL_SPI_FLASH_MODE_SPIMEMIO ) {
        PRINTF("This application cannot work with the memory mapped SPI FLASH"
            "module - do not use the FLASH_EXEC linker script for this application\n");
        return EXIT_SUCCESS;
    }

    
    PRINTF("init1\n");
    spi_flash.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    PRINTF("init2\n");
    error = w25q128jw_init(spi_flash);
    PRINTF("init3 w25q128jw_init = %d\n", error);
    X_test_read();
}

void X_spi_read(uint32_t address, uint32_t *data, uint32_t len)
{
    w25q128jw_read_standard(address, data, len);
}

//private function definitions
void X_test_read()
{
    uint32_t data[4];
    PRINTF("Reading data from WAD start address\n");
    X_spi_read(WAD_START_ADDRESS, data, 4);
    PRINTF("Data at WAD start address: %x %x %x %x\n", data[0], data[1], data[2], data[3]);
}

int main()
{
    PRINTF("Hello from DOOM!\n");
    X_init_spi();
    return 0;
}