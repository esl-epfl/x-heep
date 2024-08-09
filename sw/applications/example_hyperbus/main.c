#include "hyperbus.h"
#include <stdio.h>
#include <stdlib.h>
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "hyperbus_regs.h"

int main() {

    uint8_t random_val = 5;
    uint16_t t_latency_access = random_val++ & HYPERBUS_T_LATENCY_ACCESS_T_LATENCY_ACCESS_MASK;
    uint8_t en_latency_additional = random_val++ & (0x1 << HYPERBUS_EN_LATENCY_ADDITIONAL_EN_LATENCY_ADDITIONAL_BIT);
    uint16_t t_burst_max = random_val++ & HYPERBUS_T_BURST_MAX_T_BURST_MAX_MASK;
    uint8_t t_read_write_recovery = random_val++ & HYPERBUS_T_READ_WRITE_RECOVERY_T_READ_WRITE_RECOVERY_MASK;
    uint8_t t_rx_clk_delay = random_val++ & HYPERBUS_T_RX_CLK_DELAY_T_RX_CLK_DELAY_MASK;
    uint8_t t_tx_clk_delay = random_val++ & HYPERBUS_T_TX_CLK_DELAY_T_TX_CLK_DELAY_MASK;
    uint8_t address_mask_msb = random_val++ & HYPERBUS_ADDRESS_MASK_MSB_ADDRESS_MASK_MSB_MASK;
    uint8_t address_space = random_val++ & (0x1 << HYPERBUS_ADDRESS_SPACE_ADDRESS_SPACE_BIT);
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

    return 0;
}