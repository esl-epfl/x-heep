/*
 * Copyright 2024 EPFL
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
 * Author: Jérémie Moullet <jeremie.moullet@epfl.ch>, EPFL, STI-SEL
 */

 #include <stdio.h>
 #include <stdlib.h>
 
 #include "x-heep.h"
 #include "core_v_mini_mcu.h"
 #include "pdm2pcm_regs.h"
 #include "mmio.h"
 #include "gpio.h"
 
 
 #ifndef PDM2PCM_IS_INCLUDED
   #error ( "This app does NOT work as the PDM2PCM peripheral is not included" )
 #endif
 
 /* By default, printfs are activated for FPGA and disabled for simulation. */
 #define PRINTF_IN_FPGA  1
 #define PRINTF_IN_SIM   0
 
 #if TARGET_SIM && PRINTF_IN_SIM
         #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
 #elif PRINTF_IN_FPGA && !TARGET_SIM
     #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
 #else
     #define PRINTF(...)
 #endif
 
 #define OUTPUT_DATA_SIZE 4000
 #define GPIO_TOGGLE 2
 
 int main(int argc, char *argv[])
 {
     PRINTF("CIC DEMO\n\r");
     PRINTF(" > Start\n\r");
 
     mmio_region_t pdm2pcm_base_addr = mmio_region_from_addr((uintptr_t)PDM2PCM_START_ADDRESS);
 
     mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_CLKDIVIDX_REG_OFFSET ,15);
     mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_REACHCOUNT_REG_OFFSET, 1);
 
     mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_DECIMCIC_REG_OFFSET, 15);
     mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_DECIMHB1_REG_OFFSET, 31);
 
     mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_CONTROL_REG_OFFSET  , 1);
 
     uint32_t buffer[OUTPUT_DATA_SIZE] = {0};
 
     gpio_result_t gpio_res;
     gpio_cfg_t pin_cfg = {
         .pin = GPIO_TOGGLE,
         .mode = GpioModeOutPushPull
     };
     gpio_res = gpio_config (pin_cfg);
     if (gpio_res != GpioOk)
         PRINTF("Gpio initialization failed!\n");
 
     PRINTF("Start of CIC Testing.\n");
     gpio_write(GPIO_TOGGLE, true);
 
     int i = 0;
     while(i < OUTPUT_DATA_SIZE) {
         uint32_t status = mmio_region_read32(pdm2pcm_base_addr, PDM2PCM_STATUS_REG_OFFSET);
         if (!(status & 1)) {
             buffer[i] = mmio_region_read32(pdm2pcm_base_addr, PDM2PCM_RXDATA_REG_OFFSET);
             i++;
         }
     }
     PRINTF("Reading completed, printing result :\n");
 
     //Print the result in the console
     for(int i = 0; i < OUTPUT_DATA_SIZE; ++i) {
         PRINTF("%d\n", buffer[i]);
     }
     PRINTF("End of CIC Testing.\n");
     return EXIT_SUCCESS;
 }