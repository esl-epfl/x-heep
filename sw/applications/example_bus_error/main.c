/*
 * Copyright 2023 EPFL
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
 * Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL
 */

#include <stdio.h>
#include <stdlib.h>
#include "soc_ctrl.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "pdm2pcm_regs.h"
#include "csr.h"
#include "hart.h"

#ifdef PDM2PCM_IS_INCLUDED
  #error ( "This app does NOT work as the PDM2PCM peripheral is included" )
#endif

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   1

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

volatile int32_t bus_error_flag;

void handler_irq_bus_error(void) {
  soc_ctrl_t soc_ctrl;
  soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
  PRINTF("Bus Error IRQ triggered! - Bus error %d, Bus Address %x\n", get_bus_error(&soc_ctrl), get_bus_error_address(&soc_ctrl));
  clear_bus_error(&soc_ctrl);
  bus_error_flag++;
}


int main(int argc, char *argv[])
{


    /* Enable global interrupt for machine-level interrupts. */
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8 );
    /* Enable external interrupts */
    CSR_SET_BITS(CSR_REG_MIE, (1 << 11 ) );

    mmio_region_t pdm2pcm_base_addr = mmio_region_from_addr((uintptr_t)PDM2PCM_START_ADDRESS);
    //this write raise a bus error interrupt
    mmio_region_write32(pdm2pcm_base_addr, PDM2PCM_CLKDIVIDX_REG_OFFSET ,15);




    // disable_interrupts
    // this does not prevent waking up the core as this is controlled by the MIP register
    while(bus_error_flag == 0) {
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( bus_error_flag == 0 ) {
            wait_for_interrupt();
            //from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    return bus_error_flag ? EXIT_SUCCESS : EXIT_FAILURE;

}

