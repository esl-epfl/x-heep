#include "hyperbus.h"
#include <stdio.h>
#include <stdlib.h>
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "hyperbus_regs.h"

#define WORD0 0x12345678
#define WORD1 0x87654321
#define WORD2 0xDEADC0DE
#define WORD3 0xDEADBEEF


//HyperRam works only on 32b
volatile int32_t __attribute__((section(".xheep_data_hyperram"))) hyperram_buffer_32[301];
volatile int32_t __attribute__((section(".xheep_data_hyperram"))) hyperram_buffer_32_config_reg;

#define TEST_SORT_ARRAY_32

void __attribute__((noinline)) feed_array_32(int32_t* a, int n){
    for(int i=n-1;i>=0;i--)
        a[i] = n - i;
}

void __attribute__((noinline)) bubbleSort_32(int32_t* a, int n) {
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-i-1; j++) {
            if (a[j] > a[j+1]) {
                // Swap arr[j] and arr[j+1]
                int32_t temp = a[j];
                a[j] = a[j+1];
                a[j+1] = temp;
            }
        }
    }
}

uint8_t __attribute__((noinline)) isSorted_32(int32_t* a, int n) {
    for (int i = 0; i < n-1; i++) {
        if (a[i] > a[i+1]) {
            return -1;
        }
    }
    return 0;
}

int main() {

    uint8_t random_val = 6;
    //this should be 2 times 36-40ns (2* tacc for en_latency additional, this is the default value in the Flash model)
    uint16_t t_latency_access = 6 & HYPERBUS_T_LATENCY_ACCESS_T_LATENCY_ACCESS_MASK;
    uint8_t en_latency_additional = 1 & (0x1 << HYPERBUS_EN_LATENCY_ADDITIONAL_EN_LATENCY_ADDITIONAL_BIT);
    uint16_t t_burst_max = random_val++ & HYPERBUS_T_BURST_MAX_T_BURST_MAX_MASK;
    uint8_t t_read_write_recovery = random_val++ & HYPERBUS_T_READ_WRITE_RECOVERY_T_READ_WRITE_RECOVERY_MASK;
    uint8_t t_rx_clk_delay = random_val++ & HYPERBUS_T_RX_CLK_DELAY_T_RX_CLK_DELAY_MASK;
    uint8_t t_tx_clk_delay = random_val++ & HYPERBUS_T_TX_CLK_DELAY_T_TX_CLK_DELAY_MASK;
    uint8_t address_mask_msb = 24 & HYPERBUS_ADDRESS_MASK_MSB_ADDRESS_MASK_MSB_MASK; //up to 16MB
    uint8_t address_space = 0 & (0x1 << HYPERBUS_ADDRESS_SPACE_ADDRESS_SPACE_BIT);
#if HYPERRAMNUMPHY == 1
    uint8_t phys_in_use = 0;
    uint8_t which_phy = 0;
#else
    uint8_t phys_in_use = random_val++ & (0x1 << HYPERBUS_PHYS_IN_USE_PHYS_IN_USE_BIT);
    uint8_t which_phy = random_val++ & (0x1 << HYPERBUS_WHICH_PHY_WHICH_PHY_BIT);
#endif
    uint8_t t_csh_cycle = random_val++ & HYPERBUS_T_CSH_CYCLE_T_CSH_CYCLE_MASK;

    hyperbus_set_t_latency_access(t_latency_access);
    hyperbus_set_en_latency_additional(en_latency_additional);
    hyperbus_set_t_burst_max(t_burst_max);
    hyperbus_set_t_read_write_recovery(t_read_write_recovery);
    hyperbus_set_t_rx_clk_delay(t_rx_clk_delay);
    hyperbus_set_t_tx_clk_delay(t_tx_clk_delay);
    hyperbus_set_address_mask_msb(address_mask_msb);
    hyperbus_set_address_space(address_space);
    hyperbus_set_phys_in_use(phys_in_use);
    hyperbus_set_which_phy(which_phy);
    hyperbus_set_t_csh_cycle(t_csh_cycle);

    if (hyperbus_get_t_latency_access()     != t_latency_access) { return -1; }
    if(hyperbus_get_en_latency_additional() != en_latency_additional) { return -2; }
    if(hyperbus_get_t_burst_max()           != t_burst_max) { return -3; }
    if(hyperbus_get_t_read_write_recovery() != t_read_write_recovery) { return -4; }
    if(hyperbus_get_t_rx_clk_delay()        != t_rx_clk_delay) { return -5; }
    if(hyperbus_get_t_tx_clk_delay()        != t_tx_clk_delay) { return -6; }
    if(hyperbus_get_address_mask_msb()      != address_mask_msb) { return -7; }
    if(hyperbus_get_address_space()         != address_space) { return -8; }
    if(hyperbus_get_phys_in_use()           != phys_in_use) { return -9; }
    if(hyperbus_get_which_phy()             != which_phy) { return -10; }
    if(hyperbus_get_t_csh_cycle()           != t_csh_cycle) { return -11; }

    int32_t error_code = -1;

    // -- TEST 32b --

    hyperram_buffer_32[0] = WORD0;
    hyperram_buffer_32[1] = WORD1;

    volatile int32_t value_32 = hyperram_buffer_32[0];
    if (value_32 != WORD0) return error_code;
    error_code--;

    value_32 = hyperram_buffer_32[1];
    if (value_32 != WORD1) return error_code;
    error_code--;

     asm volatile("one: nop");
    //trying longer latency
    address_space = 1 & (0x1 << HYPERBUS_ADDRESS_SPACE_ADDRESS_SPACE_BIT);
    hyperbus_set_address_space(address_space);

    asm volatile("two: nop");
    uint32_t config_reg = hyperram_buffer_32_config_reg;
    //bits 7-4 should be 6 by default for initial latency value (t_latency_access)
    //(https://www.infineon.com/dgdl/Infineon-S27KL0641_S27KS0641_S70KL1281_S70KS1281_3.0_V_1.8_V_64_Mb_(8_MB)_128_Mb_(16_MB)_HyperRAM_Self-Refresh_DRAM-DataSheet-v15_00-EN.pdf?fileId=8ac78c8c7d0d8da4017d0ed18c684db5)

    uint16_t initial_latency = (config_reg >> 4) & 0x3;
    uint8_t BurstDelay;

    //from Model
    if (initial_latency == 0)
        BurstDelay = 5;
    else if (initial_latency == 1)
        BurstDelay = 6;
    else if (initial_latency == 0xF)
        BurstDelay = 4;
    else if (initial_latency == 0xE)
        BurstDelay = 3;

    error_code--;

    if (BurstDelay != 6) return error_code;

    BurstDelay = 3;
    initial_latency = 0xE;

    config_reg = config_reg & ~(0x7 << 4);
    config_reg = config_reg | (initial_latency << 4);

    //writing to config registers has 0 latency
    hyperbus_set_t_latency_access(0);
    asm volatile("three: nop");
    hyperram_buffer_32_config_reg = config_reg;

    //set it back to 3 now to align periph and flash
    hyperbus_set_t_latency_access(3);
    config_reg = hyperram_buffer_32_config_reg;

    asm volatile("four: nop");
    uint16_t second_latency = (config_reg >> 4) & 0x3;

    if (second_latency == 0)
        BurstDelay = 5;
    else if (second_latency == 1)
        BurstDelay = 6;
    else if (second_latency == 0xF)
        BurstDelay = 4;
    else if (second_latency == 0xE)
        BurstDelay = 3;

    error_code--;

    if (BurstDelay != 3) return error_code;

    //set back address space to 0
    address_space = 0 & (0x1 << HYPERBUS_ADDRESS_SPACE_ADDRESS_SPACE_BIT);
    hyperbus_set_address_space(address_space);

    hyperram_buffer_32[2] = WORD2;
    hyperram_buffer_32[3] = WORD3;

    error_code--;

    value_32 = hyperram_buffer_32[2];
    if (value_32 != WORD2) return error_code;

    error_code--;

    value_32 = hyperram_buffer_32[3];
    if (value_32 != WORD3) return error_code;


#ifdef TEST_SORT_ARRAY_32
    feed_array_32(hyperram_buffer_32, 64);

    bubbleSort_32(hyperram_buffer_32, 64);

    error_code--;

    if(isSorted_32(hyperram_buffer_32, 64) != 0) return error_code;

#endif

    return 0;

}