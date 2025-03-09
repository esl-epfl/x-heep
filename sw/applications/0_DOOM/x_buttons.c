
#include <stdio.h>
#include <stdbool.h>

#include "x_buttons.h"
#include "doomkeys.h"
#include "d_event.h"
#include "i_system.h"

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
int X_ButtonStateRaw(int id);

//private variables

/* Defined Buttons are:
 * 0: UP
 * 1: DOWN
 * 2: LEFT
 * 3: RIGHT
 * 4: SHOOT
 * 5: ACTIVATE (for example lift doors)
 */
bool button_prev_state[6] = {1,1,1,1,1,1,1}; //inverse logic ! ! !
bool button_state[6];
bool button_posedge[6];
bool button_negedge[6];


char button_map[] = {KEY_UPARROW, KEY_DOWNARROW, KEY_LEFTARROW, KEY_RIGHTARROW, 'h', 'j'};

//public function definitions

void X_ButtonsInit(void)
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
        PRINTF("Failed\n;");
        return -1;
    }
}

int X_ButtonStateRaw(int id)
{
    bool state;
    gpio_read(gpio_tb[id], &state);
    return !state; //inverse logic ! ! !
}

void X_ReadButtons(void)
{
    static event_t event;

    for(uint8_t iter=0; iter<6; iter++)
    {
        gpio_read(gpio_tb[iter], &button_state[iter]);
        button_posedge[iter] = !button_state[iter] && button_prev_state[iter];
        button_negedge[iter] = button_state[iter] && !button_prev_state[iter];

        if(button_posedge[iter]){
            event.type = ev_keydown;
            event.data1 = button_map[iter];
            event.data2 = 0;
            event.data3 = 0;
            D_PostEvent(&event);
        } else if(button_negedge[iter]){
            event.type = ev_keyup;
            event.data1 = button_map[iter];
            event.data2 = 0;
            event.data3 = 0;
            D_PostEvent(&event);
        }

        button_prev_state[iter] = button_state[iter];
    }
        
}

int X_ButtonState(int num)
{
    return !button_prev_state[num]; //inverse logic ! ! !
}