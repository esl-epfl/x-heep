
#include <stdio.h>
#include <stdbool.h>

#include "buttons.h"

#include "gpio.h"



//private defines:
#define GPIO_TB_IN_UP  9 //RASP PI 15
#define GPIO_TB_IN_DOWN 10 //RASP PI 13
#define GPIO_TB_IN_LEFT 11 //RASP PI 11
#define GPIO_TB_IN_RIGHT 12 //RASP PI 8
#define GPIO_TB_IN_A 13 //AR2
#define GPIO_TB_IN_B 14 //AR3

uint32_t gpio_tb[6] = {GPIO_TB_IN_UP, GPIO_TB_IN_DOWN, GPIO_TB_IN_LEFT, GPIO_TB_IN_RIGHT, GPIO_TB_IN_A, GPIO_TB_IN_B};


//private functions
int initGPIO(uint32_t gpio_tb);
int buttonStateRaw(int id);

//private variables

/* Defined Buttons are:
 * 0: UP
 * 1: DOWN
 * 2: LEFT
 * 3: RIGHT
 * 4: SHOOT
 * 5: ACTIVATE (for example lift doors)
 */
bool button_prev_state[6] = {0,0,0,0,0,0}; //inverse logic ! ! !
bool button_state[6];
bool button_posedge[6];
bool button_negedge[6];

//public function definitions

void buttonsInit(void)
{
    initGPIO(GPIO_TB_IN_UP);
    initGPIO(GPIO_TB_IN_DOWN);
    initGPIO(GPIO_TB_IN_LEFT);
    initGPIO(GPIO_TB_IN_RIGHT);
    initGPIO(GPIO_TB_IN_A);
    initGPIO(GPIO_TB_IN_B);
}

//private function definitions

int initGPIO(uint32_t gpio_tb)
{
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
        return -1;
    }
}

int buttonStateRaw(int id)
{
    bool state;
    gpio_read(gpio_tb[id], &state);
    return !state; //inverse logic ! ! !
}

void readButtons(void)
{
    for (int i = 0; i < 6; i++)
    {
        button_state[i] = buttonStateRaw(i);
        button_posedge[i] = button_state[i] && !button_prev_state[i];
        button_negedge[i] = !button_state[i] && button_prev_state[i];
        button_prev_state[i] = button_state[i];
    }
}

int buttonState(int num)
{
    return !button_prev_state[num]; //inverse logic ! ! !
}