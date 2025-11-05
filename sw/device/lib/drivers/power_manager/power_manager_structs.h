/*
                              *******************
******************************* H SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : power_manager_structs.h                                 **
** date     : 28/08/2025                                                      **
**                                                                         **
*****************************************************************************
**                                                                         **
**                                                                         **
*****************************************************************************

*/

/**
* @file   power_manager_structs.h
* @date   28/08/2025
* @brief  Contains structs for every register
*
* This file contains the structs of the registes of the peripheral.
* Each structure has the various bit fields that can be accessed
* independently.
* 
*/

#ifndef _POWER_MANAGER_STRUCTS_H
#define POWER_MANAGER_STRUCTS

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/

#include <inttypes.h>
#include "core_v_mini_mcu.h"

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/

#define power_manager_peri ((volatile power_manager *) POWER_MANAGER_START_ADDRESS)

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/



typedef struct {

  uint32_t WAKEUP_STATE;                          /*!< Wake-up state of the system*/

  uint32_t RESTORE_ADDRESS;                       /*!< Restore address value*/

  uint32_t GLOBAL_POINTER;                        /*!< Global Pointer value*/

  uint32_t EN_WAIT_FOR_INTR;                      /*!< Enable wait for interrupt*/

  uint32_t INTR_STATE;                            /*!< Interrupt state*/

  uint32_t POWER_GATE_CORE;                       /*!< Used to power gate core*/

  uint32_t POWER_GATE_CORE_ACK;                   /*!< Used by the core switch to ack the power manager*/

  uint32_t CPU_RESET_ASSERT_COUNTER;              /*!< Counter before resetting the CPU domain*/

  uint32_t CPU_RESET_DEASSERT_COUNTER;            /*!< Counter before unreset the CPU domain*/

  uint32_t CPU_SWITCH_OFF_COUNTER;                /*!< Counter before switching off the CPU domain*/

  uint32_t CPU_SWITCH_ON_COUNTER;                 /*!< Counter before switching on the CPU domain*/

  uint32_t CPU_WAIT_ACK_SWITCH_ON_COUNTER;        /*!< Bit to set whether to further wait for the ACK from the core switch after the counter expired*/

  uint32_t CPU_ISO_OFF_COUNTER;                   /*!< Counter before setting off the isolation of the CPU domain*/

  uint32_t CPU_ISO_ON_COUNTER;                    /*!< Counter before setting on the isolation of the CPU domain*/

  uint32_t CPU_COUNTERS_STOP;                     /*!< Bits to stop the counters keeping the done_o signal high*/

  uint32_t POWER_GATE_PERIPH_ACK;                 /*!< Used by the periph switch to ack the power manager*/

  uint32_t PERIPH_RESET;                          /*!< Reset the PERIPH domain*/

  uint32_t PERIPH_SWITCH;                         /*!< Switch off the PERIPH domain*/

  uint32_t PERIPH_WAIT_ACK_SWITCH_ON;             /*!< Wait for the PERIPH domain switch ack*/

  uint32_t PERIPH_ISO;                            /*!< Set on the isolation of the PERIPH domain*/

  uint32_t PERIPH_CLK_GATE;                       /*!< Clock-gates the PERIPH domain*/

  uint32_t DMA_CH0_CLK_GATE;                      /*!< Clock-gates the DMA CH0*/

  uint32_t DMA_CH1_CLK_GATE;                      /*!< Clock-gates the DMA CH1*/

  uint32_t DMA_CH2_CLK_GATE;                      /*!< Clock-gates the DMA CH2*/

  uint32_t DMA_CH3_CLK_GATE;                      /*!< Clock-gates the DMA CH3*/

  uint32_t RAM_0_CLK_GATE;                        /*!< Clock-gates the RAM_0 domain*/

  uint32_t POWER_GATE_RAM_BLOCK_0_ACK;            /*!< Used by the ram 0 switch to ack the power manager*/

  uint32_t RAM_0_SWITCH;                          /*!< Switch off the RAM_0 domain*/

  uint32_t RAM_0_WAIT_ACK_SWITCH_ON;              /*!< Wait for the RAM_0 domain switch ack*/

  uint32_t RAM_0_ISO;                             /*!< Set on the isolation of the RAM_0 domain*/

  uint32_t RAM_0_RETENTIVE;                       /*!< Set on retentive mode for the RAM_0 domain*/

  uint32_t RAM_1_CLK_GATE;                        /*!< Clock-gates the RAM_1 domain*/

  uint32_t POWER_GATE_RAM_BLOCK_1_ACK;            /*!< Used by the ram 1 switch to ack the power manager*/

  uint32_t RAM_1_SWITCH;                          /*!< Switch off the RAM_1 domain*/

  uint32_t RAM_1_WAIT_ACK_SWITCH_ON;              /*!< Wait for the RAM_1 domain switch ack*/

  uint32_t RAM_1_ISO;                             /*!< Set on the isolation of the RAM_1 domain*/

  uint32_t RAM_1_RETENTIVE;                       /*!< Set on retentive mode for the RAM_1 domain*/

  uint32_t MONITOR_POWER_GATE_CORE;               /*!< Used to monitor the signals to power gate core*/

  uint32_t MONITOR_POWER_GATE_PERIPH;             /*!< Used to monitor the signals to power gate periph*/

  uint32_t MONITOR_POWER_GATE_RAM_BLOCK_0;        /*!< Used to monitor the signals to power gate ram block 0*/

  uint32_t MONITOR_POWER_GATE_RAM_BLOCK_1;        /*!< Used to monitor the signals to power gate ram block 1*/

  uint32_t MASTER_CPU_FORCE_SWITCH_OFF;           /*!< Used to force core switch off*/

  uint32_t MASTER_CPU_FORCE_SWITCH_ON;            /*!< Used to force core switch on*/

  uint32_t MASTER_CPU_FORCE_RESET_ASSERT;         /*!< Used to force core reset assert*/

  uint32_t MASTER_CPU_FORCE_RESET_DEASSERT;       /*!< Used to force core reset deassert*/

  uint32_t MASTER_CPU_FORCE_ISO_OFF;              /*!< Used to force core iso off*/

  uint32_t MASTER_CPU_FORCE_ISO_ON;               /*!< Used to force core iso on*/

} power_manager;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

#ifndef _POWER_MANAGER_STRUCTS_C_SRC



#endif  /* _POWER_MANAGER_STRUCTS_C_SRC */

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/


/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/



#endif /* _POWER_MANAGER_STRUCTS_H */
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
