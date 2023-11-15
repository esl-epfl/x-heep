#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "soc_ctrl.h"
#include "spi_host.h"
#include "fast_intr_ctrl.h"
#include "fast_intr_ctrl_regs.h"
#include "x-heep.h"
#include "rv_timer.h"
#include "rv_timer_regs.h"

#include "power_manager.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"

// Timer handler
static rv_timer_t timer_0_1;
// Hardware thread id
uint32_t hart_id = 0;

int main() {
    printf("Timer example\n");
    
    // Get current Frequency
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    uint32_t freq_hz = soc_ctrl_get_frequency(&soc_ctrl);
    printf("Freq: %u\n", freq_hz);

    // Initialize timer
    mmio_region_t timer_0_1_reg = mmio_region_from_addr(RV_TIMER_AO_START_ADDRESS);
    rv_timer_init(timer_0_1_reg, (rv_timer_config_t){.hart_count = 2, .comparator_count = 1}, &timer_0_1);
    /* defining timer prescale and step based on its desired freq */
    uint64_t kTickFreqHz = 1000 * 1000; // 1 MHz
    rv_timer_tick_params_t tick_params;
    rv_timer_approximate_tick_params(freq_hz, kTickFreqHz, &tick_params);
    if (tick_params.prescale==0){
        printf("Timer approximate function was not able to set a correct value prescale\n");
    }

    uint32_t hart_id = 0;

    // Set tick params
    rv_timer_set_tick_params(&timer_0_1, hart_id, tick_params);

    // Start a counter for a particular hart
    rv_timer_counter_set_enabled(&timer_0_1, hart_id, kRvTimerEnabled);

    printf("Doing some work...\n");
    printf("Doing some extra work...\n");

    // Stop the counter
    rv_timer_counter_set_enabled(&timer_0_1, hart_id, kRvTimerDisabled);

    // Read the counter value
    uint64_t counter_value = 0;
    rv_timer_counter_read(&timer_0_1, hart_id, &counter_value);

    // Print the counter value
    printf("Counter value: %u\n", counter_value);

    // Reset the counter
    mmio_region_write32(
        timer_0_1_reg,
        reg_for_hart(hart_id, RV_TIMER_TIMER_V_LOWER0_REG_OFFSET), 0x0
    );
    mmio_region_write32(
        timer_0_1_reg,
        reg_for_hart(hart_id, RV_TIMER_TIMER_V_UPPER0_REG_OFFSET), 0x0
    );

    // Read the counter value
    rv_timer_counter_read(&timer_0_1, hart_id, &counter_value);

    // Print the counter value. It must be 0.
    printf("Counter value: %u (expected: 0)\n", counter_value);

    // Exit code
    if (counter_value != 0) return EXIT_FAILURE;
    else return EXIT_SUCCESS;
}