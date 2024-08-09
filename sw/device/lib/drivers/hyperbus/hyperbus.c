/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************
**
** project  : X-HEEP
** filename : hyperbus.c
**
***************************************************************************
**
** Copyright (c) EPFL contributors.
** All rights reserved.
**
***************************************************************************
*/

/***************************************************************************/
/***************************************************************************/
/**
* @file     hyperbus.c
* @date     09/10/2024
* @author   Davide Schiavone
* @version  1
* @brief    HyperBus driver
*/
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "hyperbus.h"
#include "hyperbus_regs.h"  // Generated.
#include "hyperbus_structs.h"
#include "bitfield.h"

void hyperbus_set_t_latency_access(uint8_t t_latency_access)
{
  hyperbus_peri->T_LATENCY_ACCESS = bitfield_field32_write(hyperbus_peri->T_LATENCY_ACCESS, HYPERBUS_T_LATENCY_ACCESS_T_LATENCY_ACCESS_FIELD, t_latency_access);
  return;
}

uint8_t hyperbus_get_t_latency_access()
{
    uint32_t val = bitfield_field32_read(hyperbus_peri->T_LATENCY_ACCESS, HYPERBUS_T_LATENCY_ACCESS_T_LATENCY_ACCESS_FIELD);
    return (uint8_t)val;
}

void hyperbus_set_en_latency_additional(uint8_t en_latency_additional)
{
  hyperbus_peri->EN_LATENCY_ADDITIONAL = bitfield_bit32_write(hyperbus_peri->EN_LATENCY_ADDITIONAL, HYPERBUS_EN_LATENCY_ADDITIONAL_EN_LATENCY_ADDITIONAL_BIT, en_latency_additional);
  return;
}

uint8_t hyperbus_get_en_latency_additional()
{
    uint32_t val = bitfield_bit32_read(hyperbus_peri->EN_LATENCY_ADDITIONAL, HYPERBUS_EN_LATENCY_ADDITIONAL_EN_LATENCY_ADDITIONAL_BIT);
    return (uint8_t)val;
}

void hyperbus_set_t_burst_max(uint16_t t_burst_max)
{
  hyperbus_peri->T_BURST_MAX = bitfield_field32_write(hyperbus_peri->T_BURST_MAX, HYPERBUS_T_BURST_MAX_T_BURST_MAX_FIELD, t_burst_max);
  return;
}

uint16_t hyperbus_get_t_burst_max()
{
    uint32_t val = bitfield_field32_read(hyperbus_peri->T_BURST_MAX, HYPERBUS_T_BURST_MAX_T_BURST_MAX_FIELD);
    return (uint16_t)val;
}

void hyperbus_set_t_read_write_recovery(uint8_t t_read_write_recovery)
{
  hyperbus_peri->T_READ_WRITE_RECOVERY = bitfield_field32_write(hyperbus_peri->T_READ_WRITE_RECOVERY, HYPERBUS_T_READ_WRITE_RECOVERY_T_READ_WRITE_RECOVERY_FIELD, t_read_write_recovery);
  return;
}

uint8_t hyperbus_get_t_read_write_recovery()
{
    uint32_t val = bitfield_field32_read(hyperbus_peri->T_READ_WRITE_RECOVERY, HYPERBUS_T_READ_WRITE_RECOVERY_T_READ_WRITE_RECOVERY_FIELD);
    return (uint8_t)val;
}

void hyperbus_set_t_rx_clk_delay(uint8_t t_rx_clk_delay)
{
  hyperbus_peri->T_RX_CLK_DELAY = bitfield_field32_write(hyperbus_peri->T_RX_CLK_DELAY, HYPERBUS_T_RX_CLK_DELAY_T_RX_CLK_DELAY_FIELD, t_rx_clk_delay);
  return;
}

uint8_t hyperbus_get_t_rx_clk_delay()
{
    uint32_t val = bitfield_field32_read(hyperbus_peri->T_RX_CLK_DELAY, HYPERBUS_T_RX_CLK_DELAY_T_RX_CLK_DELAY_FIELD);
    return (uint8_t)val;
}

void hyperbus_set_t_tx_clk_delay(uint8_t t_tx_clk_delay)
{
  hyperbus_peri->T_TX_CLK_DELAY = bitfield_field32_write(hyperbus_peri->T_TX_CLK_DELAY, HYPERBUS_T_TX_CLK_DELAY_T_TX_CLK_DELAY_FIELD, t_tx_clk_delay);
  return;
}

uint8_t hyperbus_get_t_tx_clk_delay()
{
    uint32_t val = bitfield_field32_read(hyperbus_peri->T_TX_CLK_DELAY, HYPERBUS_T_TX_CLK_DELAY_T_TX_CLK_DELAY_FIELD);
    return (uint8_t)val;
}

void hyperbus_set_address_mask_msb(uint8_t address_mask_msb)
{
  hyperbus_peri->ADDRESS_MASK_MSB = bitfield_field32_write(hyperbus_peri->ADDRESS_MASK_MSB, HYPERBUS_ADDRESS_MASK_MSB_ADDRESS_MASK_MSB_FIELD, address_mask_msb);
  return;
}

uint8_t hyperbus_get_address_mask_msb()
{
    uint32_t val = bitfield_field32_read(hyperbus_peri->ADDRESS_MASK_MSB, HYPERBUS_ADDRESS_MASK_MSB_ADDRESS_MASK_MSB_FIELD);
    return (uint8_t)val;
}

void hyperbus_set_address_space(uint8_t address_space)
{
  hyperbus_peri->ADDRESS_SPACE = bitfield_bit32_write(hyperbus_peri->ADDRESS_SPACE, HYPERBUS_ADDRESS_SPACE_ADDRESS_SPACE_BIT, address_space);
  return;
}

uint8_t hyperbus_get_address_space()
{
    uint32_t val = bitfield_bit32_read(hyperbus_peri->ADDRESS_SPACE, HYPERBUS_ADDRESS_SPACE_ADDRESS_SPACE_BIT);
    return (uint8_t)val;
}  

void hyperbus_set_phys_in_use(uint8_t phys_in_use)
{
  hyperbus_peri->PHYS_IN_USE = bitfield_bit32_write(hyperbus_peri->PHYS_IN_USE, HYPERBUS_PHYS_IN_USE_PHYS_IN_USE_BIT, phys_in_use);
  return;
}

uint8_t hyperbus_get_phys_in_use()
{
    uint32_t val = bitfield_bit32_read(hyperbus_peri->PHYS_IN_USE, HYPERBUS_PHYS_IN_USE_PHYS_IN_USE_BIT);
    return (uint8_t)val;
}


void hyperbus_set_which_phy(uint8_t which_phy)
{
  hyperbus_peri->WHICH_PHY = bitfield_bit32_write(hyperbus_peri->WHICH_PHY, HYPERBUS_WHICH_PHY_WHICH_PHY_BIT, which_phy);
  return;
}

uint8_t hyperbus_get_which_phy()
{
    uint32_t val = bitfield_bit32_read(hyperbus_peri->WHICH_PHY, HYPERBUS_WHICH_PHY_WHICH_PHY_BIT);
    return (uint8_t)val;
}

void hyperbus_set_t_csh_cycle(uint8_t t_csh_cycle)
{
  hyperbus_peri->T_CSH_CYCLE = bitfield_field32_write(hyperbus_peri->T_CSH_CYCLE, HYPERBUS_T_CSH_CYCLE_T_CSH_CYCLE_FIELD, t_csh_cycle);
  return;
}

uint8_t hyperbus_get_t_csh_cycle()
{
    uint32_t val = bitfield_field32_read(hyperbus_peri->T_CSH_CYCLE, HYPERBUS_T_CSH_CYCLE_T_CSH_CYCLE_FIELD);
    return (uint8_t)val;
}

#ifdef __cplusplus
}
#endif // __cplusplus

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/