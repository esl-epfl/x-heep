/*
 * Copyright 2020 ETH Zurich
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
 * Author: Robert Balas <balasr@iis.ee.ethz.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "error.h"

int main(int argc, char *argv[])
{

    uart_t uart;
    uart.base_addr   = mmio_region_from_addr((uintptr_t)UART_START_ADDRESS);
    uart.baudrate    = 256000;
    uart.clk_freq_hz = REFERENCE_CLOCK_KHz*1000;

    if (uart_init(&uart) != kErrorOk) {
        return -1;
    }


    /* write something to stdout */
    printf("hello world!\r\n");


    char mychar[10+1];

    uart_read(&uart,mychar,3);
    mychar[3] = 0;

    printf("written %x %x %x\n",mychar[0],mychar[1], mychar[2]);
    printf("written %s\n",mychar);

    return EXIT_SUCCESS;
}
