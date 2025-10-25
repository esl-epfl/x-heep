/*
 * Copyright 2025 EPFL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Davide Schiavone <davide.schiavone@epfl.ch>, EPFL, STI-SEL (2025)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "soc_ctrl.h"
#include "core_v_mini_mcu.h"

int main(int argc, char *argv[])
{

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

    printf("Hello from X-HEEP %d\n", get_xheep_instance_id(&soc_ctrl));

    soc_ctrl_xheep_ao_peripheral_config_t ao_peri = get_xheep_ao_peripheral_config(&soc_ctrl);
    soc_ctrl_xheep_peripheral_config_t    peri = get_xheep_peripheral_config(&soc_ctrl);

    printf("AO Peripheral: \n"
        "SPI Flash: %d\n"
        "DMA: %d\n"
        "PAD CONTROL: %d\n"
        "GPIO AO: %d\n", ao_peri.has_spi_flash, ao_peri.has_dma, ao_peri.has_pad_control, ao_peri.has_gpio_ao
    );

    printf("Peripheral: \n"
        "RV PLIC: %d\n"
        "SPI HOST: %d\n"
        "GPIO: %d\n"
        "I2C: %d\n"
        "RV_TIMER: %d\n"
        "SPI2: %d\n"
        "PDM2PCM: %d\n"
        "I2S: %d\n"
        "UART: %d\n", peri.has_rv_plic, peri.has_spi_host, peri.has_gpio, peri.has_i2c,
                     peri.has_rv_timer, peri.has_spi2, peri.has_pdm2pcm, peri.has_i2s,
                     peri.has_uart
    );

    return EXIT_SUCCESS;
}
