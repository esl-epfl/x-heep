#include <stdio.h>
#include <stdbool.h>

#include "x_buttons.h"
#include "doomkeys.h"
#include "d_event.h"
#include "i_system.h"
#include "pad_control.h"
#include "pad_control_regs.h"
#include "core_v_mini_mcu.h"

#include "csr.h"
#include "rv_plic.h"
#include "gpio.h"

// --- Private defines ---
#define GPIO_TB_IN_UP     10
#define GPIO_TB_IN_DOWN  9
#define GPIO_TB_IN_LEFT  11
#define GPIO_TB_IN_RIGHT 12
#define GPIO_TB_IN_A     13
#define GPIO_TB_IN_B     14

// --- Private variables ---
static const uint32_t gpio_tb[6] = {
    GPIO_TB_IN_UP, GPIO_TB_IN_DOWN, GPIO_TB_IN_LEFT,
    GPIO_TB_IN_RIGHT, GPIO_TB_IN_A, GPIO_TB_IN_B
};

static const char button_map[6] = {
    KEY_UPARROW, KEY_DOWNARROW, KEY_LEFTARROW, KEY_RIGHTARROW, 'h', 'j'
};

static bool button_prev_state[6] = {1, 1, 1, 1, 1, 1};

// --- Forward declarations ---
static void x_button_common_handler(int button_idx);

// --- Public functions ---

void X_ButtonsInit(void)
{
    printf("Buttons init\n");

    pad_control_t pad_control;
    pad_control.base_addr = mmio_region_from_addr((uintptr_t)PAD_CONTROL_START_ADDRESS);

    if (plic_Init() != kPlicOk) {
        printf("PLIC init failed\n");
        return;
    }

    for (int i = 0; i < 6; i++) {
        gpio_cfg_t cfg = {
            .pin = gpio_tb[i],
            .mode = GpioModeIn,
            .en_input_sampling = true,
            .en_intr = true,
            .intr_type = GpioIntrEdgeRising
        };

        if (gpio_config(cfg) != GpioOk) {
            printf("GPIO config failed for pin %d\n", gpio_tb[i]);
        }

        plic_irq_set_priority(gpio_tb[i], 1);
        plic_irq_set_enabled(gpio_tb[i], kPlicToggleEnabled);
    }

    gpio_assign_irq_handler(GPIO_TB_IN_UP,    button_up_handler);
    gpio_assign_irq_handler(GPIO_TB_IN_DOWN,  button_down_handler);
    gpio_assign_irq_handler(GPIO_TB_IN_LEFT,  button_left_handler);
    gpio_assign_irq_handler(GPIO_TB_IN_RIGHT, button_right_handler);
    gpio_assign_irq_handler(GPIO_TB_IN_A,     button_a_handler);
    gpio_assign_irq_handler(GPIO_TB_IN_B,     button_b_handler);

    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    CSR_SET_BITS(CSR_REG_MIE, (1 << 11));
}

// --- Private helper ---

static void x_button_common_handler(int idx)
{
    static event_t event;

    // Send KeyDown event
    event.type = ev_keydown;
    event.data1 = button_map[idx];
    event.data2 = 0;
    event.data3 = 0;
    D_PostEvent(&event);

    // Immediately send KeyUp event
    event.type = ev_keyup;
    D_PostEvent(&event);

    printf("Button %d pressed (rising edge)\n", idx);
}

// --- Interrupt handlers ---

void button_up_handler(void) {
    x_button_common_handler(0);
    gpio_intr_clear_stat(GPIO_TB_IN_UP);
}

void button_down_handler(void) {
    x_button_common_handler(1);
    gpio_intr_clear_stat(GPIO_TB_IN_DOWN);
}

void button_left_handler(void) {
    x_button_common_handler(2);
    gpio_intr_clear_stat(GPIO_TB_IN_LEFT);
}

void button_right_handler(void) {
    x_button_common_handler(3);
    gpio_intr_clear_stat(GPIO_TB_IN_RIGHT);
}

void button_a_handler(void) {
    x_button_common_handler(4);
    gpio_intr_clear_stat(GPIO_TB_IN_A);
}

void button_b_handler(void) {
    x_button_common_handler(5);
    gpio_intr_clear_stat(GPIO_TB_IN_B);
}

// --- Optional helpers ---

int X_ButtonStateRaw(int id)
{
    bool state;
    gpio_read(gpio_tb[id], &state);
    return !state;
}

int X_ButtonState(int num)
{
    return !button_prev_state[num];
}

void X_ReadButtons(void)
{
    // Not needed anymore, handled by interrupts
}