/*
                              *******************
******************************* H SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : X-HEEP                                                       **
** filename : hyperbus.h                                                   **
**                                                                         **
*****************************************************************************
**
** Copyright (c) EPFL contributors.
** All rights reserved.
**
*****************************************************************************
*/

/***************************************************************************/
/***************************************************************************/
/**
* @file     hyperbus.h
* @date     09/10/2024
* @author   Davide Schiavone
* @version  1
* @brief    HyperBus driver
*/

#ifndef HYPERBUS_H_
#define HYPERBUS_H_

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void hyperbus_set_t_latency_access(uint8_t t_latency_access);
uint8_t hyperbus_get_t_latency_access();
void hyperbus_set_en_latency_additional(uint8_t en_latency_additional);
uint8_t hyperbus_get_en_latency_additional();
void hyperbus_set_t_burst_max(uint16_t t_burst_max);
uint16_t hyperbus_get_t_burst_max();
void hyperbus_set_t_read_write_recovery(uint8_t t_read_write_recovery);
uint8_t hyperbus_get_t_read_write_recovery();
void hyperbus_set_t_rx_clk_delay(uint8_t t_rx_clk_delay);
uint8_t hyperbus_get_t_rx_clk_delay();
void hyperbus_set_t_tx_clk_delay(uint8_t t_tx_clk_delay);
uint8_t hyperbus_get_t_tx_clk_delay();
void hyperbus_set_address_mask_msb(uint16_t address_mask_msb);
uint16_t hyperbus_get_address_mask_msb();
void hyperbus_set_address_space(uint8_t address_space);
uint8_t hyperbus_get_address_space();
void hyperbus_set_phys_in_use(uint8_t phys_in_use);
uint8_t hyperbus_get_phys_in_use();
void hyperbus_set_which_phy(uint8_t which_phy);
uint8_t hyperbus_get_which_phy();
void hyperbus_set_t_csh_cycle(uint8_t t_csh_cycle);
uint8_t hyperbus_get_t_csh_cycle();

#endif  // HYPERBUS_H_
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/