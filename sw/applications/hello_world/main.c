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
           Davide Schiavone <davide.schiavone@epfl.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include "soc_ctrl.h"
#include "core_v_mini_mcu.h"

int main(int argc, char *argv[])
{

    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);

    /* write something to stdout */
    printf("Hello from X-HEEP %d\n", get_xheep_instance_id(&soc_ctrl));
    return EXIT_SUCCESS;
}
