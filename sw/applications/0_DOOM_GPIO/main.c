// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
//#include "csr.h"
//#include "hart.h"
//#include "handler.h"
//#include "core_v_mini_mcu.h"
//#include "rv_plic.h"
//#include "rv_plic_regs.h"
#include "gpio.h"
//#include "pad_control.h"
//#include "pad_control_regs.h"  // Generated.
//#include "x-heep.h"
//#include <limits.h> //todo: remove



/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#define TARGET_PYNQ_Z2 1


#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

/*
#ifndef RV_PLIC_IS_INCLUDED
  //#error ( "This app does NOT work as the RV_PLIC peripheral is not included" )
#endif
*/

#define GPIO_TB_IN_UP  9 //RASP PI 15
#define GPIO_INTR_UP GPIO_INTR_9

#define GPIO_TB_IN_DOWN 10 //RASP PI 13
#define GPIO_INTR_DOWN GPIO_INTR_10

#define GPIO_TB_IN_LEFT 11 //RASP PI 11
#define GPIO_INTR_LEFT GPIO_INTR_11

#define GPIO_TB_IN_RIGHT 12 //RASP PI 8
#define GPIO_INTR_RIGHT GPIO_INTR_12

#define GPIO_TB_IN_A 13 //AR2
#define GPIO_INTR_A GPIO_INTR_13

#define GPIO_TB_IN_B 14 //AR3
#define GPIO_INTR_B GPIO_INTR_14


//local functions
int initGPIO(/*uint32_t pinNum, */uint32_t gpio_tb);

uint32_t gpio_tb[6] = {GPIO_TB_IN_UP, GPIO_TB_IN_DOWN, GPIO_TB_IN_LEFT, GPIO_TB_IN_RIGHT, GPIO_TB_IN_A, GPIO_TB_IN_B};
bool button_prev_state[6] = {1,1,1,1,1,1,1};  //UP, DOWN, LEFT, RIGHT, A, B
bool button_state[6];       //UP, DOWN, LEFT, RIGHT, A, B

void poll_handler(uint8_t iter, bool isPressed);

void handler_up()
{
    PRINTF("UP Button pressed\n");
}

void handler_down()
{
    PRINTF("DOWN Button pressed\n");
}
void handler_left()
{
    PRINTF("LEFT Button pressed\n");
}

void handler_right()
{
    PRINTF("RIGHT Button pressed\n");
}

void handler_a()
{
    PRINTF("A Button pressed\n");
}

void handler_b()
{
    PRINTF("B Button pressed\n");
}

int main(int argc, char *argv[])
{

/*
    plic_result_t plic_result;
    pad_control_t pad_control;
    pad_control.base_addr = mmio_region_from_addr((uintptr_t)PAD_CONTROL_START_ADDRESS);
    plic_result = plic_Init();
    if (plic_result != kPlicOk) {
        PRINTF("Init PLIC failed\n\r;");
        return -1;
    }
*/
    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    //CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    //const uint32_t mask = 1 << 11;
    //CSR_SET_BITS(CSR_REG_MIE, mask);


    initGPIO(GPIO_TB_IN_UP);
    initGPIO(GPIO_TB_IN_DOWN);
    initGPIO(GPIO_TB_IN_LEFT);
    initGPIO(GPIO_TB_IN_RIGHT);
    initGPIO(GPIO_TB_IN_A);
    initGPIO(GPIO_TB_IN_B);
/*
    gpio_assign_irq_handler( GPIO_INTR_UP, &handler_up );
    gpio_assign_irq_handler( GPIO_INTR_DOWN, &handler_down );
    gpio_assign_irq_handler( GPIO_INTR_LEFT, &handler_left );
    gpio_assign_irq_handler( GPIO_INTR_RIGHT, &handler_right );
    gpio_assign_irq_handler( GPIO_INTR_A, &handler_a );
    gpio_assign_irq_handler( GPIO_INTR_B, &handler_b );
*/
    PRINTF("Press Buttons for interrupt...\n\r");
    while(1) {

        for(uint8_t iter=0; iter<6; iter++)
        {
            gpio_read(gpio_tb[iter], &button_state[iter]);
            if(button_state[iter] != button_prev_state[iter])
            {
                if(button_state[iter] == true)
                {
                    poll_handler(iter, false);
                }
                else
                {
                    poll_handler(iter, true);
                }
                button_prev_state[iter] = button_state[iter];
            }
        }
    }

    return EXIT_SUCCESS;
}


int initGPIO(/*uint32_t pinNum,*/ uint32_t gpio_tb)
{
    /*
    plic_result_t plic_res = kPlicOk;

        plic_res = plic_irq_set_priority(pinNum, 1);
    if (plic_res != kPlicOk) {
        PRINTF("Failed\n\r;");
        return -1;
    }

    plic_res = plic_irq_set_enabled(pinNum, kPlicToggleEnabled);
    if (plic_res != kPlicOk) {
        PRINTF("Failed\n\r;");
        return -1;
    }
*/
    gpio_result_t gpio_res;

    gpio_cfg_t cfg_in = {
        .pin = gpio_tb,
        .mode = GpioModeIn,
        .en_input_sampling = true,
        .en_intr = true,
        .intr_type = GpioIntrEdgeFalling
    };

    gpio_res = gpio_config(cfg_in);
    if (gpio_res != GpioOk) {
        PRINTF("Failed\n;");
        return -1;
    }
}

void poll_handler(uint8_t iter, bool isPressed)
{
    if(isPressed)
    {
        switch (iter)
        {
        case 0:
            PRINTF("UP Button pressed\n");
            break;
        case 1:
            PRINTF("DOWN Button pressed\n");
            break;
        case 2:
            PRINTF("LEFT Button pressed\n");
            break;
        case 3:
            PRINTF("RIGHT Button pressed\n");
            break;
        case 4:
            PRINTF("A Button pressed\n");
            break;
        case 5:
            PRINTF("B Button pressed\n");
            break;       
        default:
            break;
        }
    }
    else
    {
        switch (iter)
        {
        case 0:
            PRINTF("UP Button released\n");
            break;
        case 1:
            PRINTF("DOWN Button released\n");
            break;
        case 2:
            PRINTF("LEFT Button released\n");
            break;
        case 3:
            PRINTF("RIGHT Button released\n");
            break;
        case 4:
            PRINTF("A Button released\n");
            break;
        case 5:
            PRINTF("B Button released\n");
            break;
        default:
            break;
        }
    }
}