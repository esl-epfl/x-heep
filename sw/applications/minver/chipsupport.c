/*
**
** Copyright 2020 OpenHW Group
** 
** Licensed under the Solderpad Hardware Licence, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** 
**     https://solderpad.org/licenses/
** 
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
** 
*******************************************************************************
*/

#include <support.h>
#include <stdint.h>
#include <stdio.h>
#include "chipsupport.h"

#include "csr.h"
#include "x-heep.h"

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

#define FS_INITIAL 0x01

void
initialise_board ()
{
  PRINTF("Initialize board corev32 \n");

  //enable FP operations
  CSR_SET_BITS(CSR_REG_MSTATUS, (FS_INITIAL << 13));

}

void __attribute__ ((noinline)) __attribute__ ((externally_visible))
start_trigger ()
{
  PRINTF("start of test \n");

  // Enable mcycle counter and read value
  CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
  CSR_WRITE(CSR_REG_MCYCLE, 0);

}

void __attribute__ ((noinline)) __attribute__ ((externally_visible))
stop_trigger ()
{
  uint32_t cycle_cnt;
  CSR_READ(CSR_REG_MCYCLE, &cycle_cnt);
  PRINTF("end of test \n");
  PRINTF("Result is given in CPU cycles \n");
  PRINTF("RES: %d \n", cycle_cnt);

}
 
