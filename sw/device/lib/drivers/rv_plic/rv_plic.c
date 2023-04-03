/*
                              *******************
******************************* C SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : rv_plic.c                                                    **
** date     : 28/03/2023                                                   **
**                                                                         **
*****************************************************************************
**                                                                         **
**                                                                         **
*****************************************************************************

*/

/***************************************************************************/
/***************************************************************************/

/**
* @file   rv_plic.c
* @date   28/03/2023
* @brief  This is the main file for the HAL of the RV_PLIC peripheral
*
* In this file there are the defintions of the HAL functions for the RV_PLIC
* peripheral. They provide many low level functionalities to interact
* with the registers content, reading and writing them according to the specific
* function of each one.
*
*/

#define _RV_PLIC_C_SRC

/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "rv_plic.h"
#include "rv_plic_structs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bitfield.h"
#include "mmio.h"

#include "rv_plic_regs.h"  // Generated.
#include "core_v_mini_mcu.h"

#include "handler.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

/**
 * Minimum and Maximum priority that an interrupt source can have
*/
const uint32_t plicMinPriority = 0;
const uint32_t plicMaxPriority = RV_PLIC_PRIO0_PRIO0_MASK;

/****************************************************************************/
/**                                                                        **/
/*                        TYPEDEFS AND STRUCTURES                           */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED VARIABLES                             */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                            GLOBAL VARIABLES                              */
/**                                                                        **/
/****************************************************************************/

int8_t external_intr_flag = 0;

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

__attribute__((weak)) void handler_irq_uart(void) {
  printf("WEAK UART IRQ triggered!\n");
  volatile int i=0;
  i++;
}

__attribute__((weak)) void handler_irq_gpio(void) {
  printf("WEAK GPIO IRQ triggered!\n");
  volatile int i=0;
  i++;
}

__attribute__((weak)) void handler_irq_i2c(void) {
  printf("WEAK I2C IRQ triggered!\n");
  volatile int i=0;
  i++;
}

__attribute__((weak)) void handler_irq_spi(void) {
  printf("WEAK SPI IRQ triggered!\n");
  volatile int i=0;
  i++;
}


/**
 * Returns the irq_sources_t source type of a given irq source ID
*/
static irq_sources_t plic_get_irq_src_type(plic_irq_id_t irq_id)
{
  if(irq_id >= 1 && irq_id <= 8)
  {
    return IRQ_UART_SRC;
  }
  else if (irq_id >= 9 && irq_id <= 32)
  {
    return IRQ_GPIO_SRC;
  }
  else if (irq_id >= 33 && irq_id <= 48)
  {
    return IRQ_I2C_SRC;
  }
  else if(irq_id == 49)
  {
    return IRQ_SPI_SRC;
  }
  else
    return IRQ_BAD;
}


void handler_irq_external(void)
{
  plic_irq_id_t int_id;
  plic_result_t res = plic_irq_claim(0, &int_id);
  irq_sources_t type = plic_get_irq_src_type(int_id);

  //TODO: adjust each case by calling the proper interrupt handler 
  switch (type)
  {
  case IRQ_UART_SRC: 
    handler_irq_uart();
    external_intr_flag = 1;
    break;
  
  case IRQ_GPIO_SRC:
    handler_irq_gpio();
    external_intr_flag = 1;
    break;

  case IRQ_I2C_SRC:
    handler_irq_i2c();
    external_intr_flag = 1;
    break;

  case IRQ_SPI_SRC:
    handler_irq_spi();
    external_intr_flag = 1;
    break;

  case IRQ_BAD:
    printf("Bad INT source!\n");
    break;

  default:
    break;
  }

  plic_irq_complete(&int_id);
}


/*!
  Resets relevant registers of the PLIC (Level/Edge, 
  priority, target, threshold, interrupts).
  It writes the default value 0 in them and read back
  the value into the proper register struct
*/
plic_result_t plic_Init(void)
{
  
  /* Clears all the Level/Edge registers */ 
  for(size_t i=0; i<RV_PLIC_LE_MULTIREG_COUNT; i++)
  {
    uint32_t *dest = (&rv_plic_peri->IP0 + i);
    *dest = 0;

    if (*dest != 0) 
    {
      return kDifPlicError;
    }
  }


  /* Clears all the priority registers */ 
  for(size_t i=0; i<RV_PLIC_PARAM_NUM_SRC; i++)
  {
    // ptrdiff_t offset = plic_priority_reg_offset(i);
    // mmio_region_write32(rv_plic.rv_plic_base_addr, offset, 0);

    uint32_t *dest = &rv_plic_peri->IP0 + i;
    *dest = 0;

    if(*dest != 0)
    {
      return kDifPlicError;
    }
  }

  /* Address of the first IE register */
  uint32_t *base = &rv_plic_peri->IE00;

  /* Clears the Interrupt Enable registers */
  for(size_t i=0; i<RV_PLIC_IE0_MULTIREG_COUNT; i++)
  {
    uint32_t *dest = base + i;
    *dest = 0;

    if(*dest != 0)
    {
      return kDifPlicError;
    }
  }
  
  /* Clears all the threshold registers */
  rv_plic_peri->THRESHOLD0 = 0;
  if(rv_plic_peri->THRESHOLD0 != 0)
  {
    return kDifPlicError;
  }


  /* clears software interrupts registers */
  rv_plic_peri->MSIP0 = 0;
  if(rv_plic_peri->MSIP0 != 0)
  {
    return kDifPlicError;
  }

  return kDifPlicOk;

}


plic_result_t plic_irq_set_enabled(plic_irq_id_t irq,
                                       plic_toggle_t state)
{
  if(irq >= RV_PLIC_PARAM_NUM_SRC)
  {
    return kDifPlicBadArg;
  }

  bool flag;
  switch (state)
  {
  case kDifPlicToggleEnabled:
    flag = true;
    break;
  
  case kDifPlicToggleDisabled:
    flag = false;
    break;

  default:
    return kDifPlicBadArg;
  }

  ptrdiff_t offset = plic_offset_from_reg0(irq);
  uint32_t *dest = &rv_plic_peri->IE00 + offset;
  uint16_t bit_index = plic_irq_bit_index(irq);
  
  *dest = bitfield_bit32_write(*dest, bit_index, flag);

  return kDifPlicOk;
}


plic_result_t plic_irq_get_enabled(plic_irq_id_t irq,
                                       plic_toggle_t *state)
{
  if(irq >= RV_PLIC_PARAM_NUM_SRC)
  {
    return kDifPlicBadArg;
  }
  
  ptrdiff_t offset = plic_offset_from_reg0(irq);
  uint32_t *reg = &rv_plic_peri->IE00 + offset;
  uint16_t bit_index = plic_irq_bit_index(irq);

  bool is_enabled = bitfield_bit32_read(*reg, bit_index);

  *state = is_enabled ? kDifPlicToggleEnabled : kDifPlicToggleDisabled;

  return kDifPlicOk;

}


plic_result_t plic_irq_set_trigger(plic_irq_id_t irq,
                                           plic_irq_trigger_t trigger)
{
  if(irq >= RV_PLIC_PARAM_NUM_SRC)
  {
    return kDifPlicBadArg;
  }

  bool flag;
  switch(trigger) {
    case kDifPlicIrqTriggerEdge:
      flag = true;  
      break;

    case kDifPlicIrqTriggerLevel:
      flag = false;
      break;

    default:
      return kDifPlicBadArg;
  }

  ptrdiff_t offset = plic_offset_from_reg0(irq);
  uint32_t *reg = &rv_plic_peri->LE0 + offset;
  uint16_t bit_index = plic_irq_bit_index(irq);

  *reg = bitfield_bit32_write(*reg, bit_index, flag);

  return kDifPlicOk;

}


plic_result_t plic_irq_set_priority(plic_irq_id_t irq, uint32_t priority)
{
  if(irq >= RV_PLIC_PARAM_NUM_SRC || priority > plicMaxPriority)
  {
    return kDifPlicBadArg;
  }

  uint32_t *dest = &rv_plic_peri->PRIO0 + irq;
  *dest = priority;

  if(*dest != priority){
    return kDifPlicError;
  }

  return kDifPlicOk;
}


plic_result_t plic_target_set_threshold(uint32_t threshold)
{
  if(threshold > plicMaxPriority)
  {
    return kDifPlicBadArg;
  }

  rv_plic_peri->THRESHOLD0 = threshold;

  return kDifPlicOk;

}


plic_result_t plic_irq_is_pending(plic_irq_id_t irq,
                                          bool *is_pending)
{
  if(irq >= RV_PLIC_PARAM_NUM_SRC || is_pending == NULL)
  {
    return kDifPlicBadArg;
  }

  ptrdiff_t offset = plic_offset_from_reg0(irq);
  uint32_t *reg = &rv_plic_peri->IP0 + offset;
  uint16_t bit_index = plic_irq_bit_index(irq);

  *is_pending = bitfield_bit32_read(*reg, bit_index);

  return kDifPlicOk;
}


plic_result_t plic_irq_claim(plic_target_t target, plic_irq_id_t *claim_data) {
  if (target >= RV_PLIC_PARAM_NUM_TARGET || claim_data == NULL) 
  {
    return kDifPlicBadArg;
  }

  // printf("CC0:\t%d\n", rv_plic_peri->CC0);
  // printf("CC0:\t%d\n", rv_plic_peri->CC0);
  *claim_data = rv_plic_peri->CC0;

  return kDifPlicOk;
}


plic_result_t plic_irq_complete(const plic_irq_id_t *complete_data) {
  if (complete_data == NULL) 
  {
    return kDifPlicBadArg;
  }

  // Write back the claimed IRQ ID to the target specific CC register,
  // to notify the PLIC of the IRQ completion.
  rv_plic_peri->CC0 = *complete_data;

  return kDifPlicOk;
}


plic_result_t plic_software_irq_force()
{
  rv_plic_peri->MSIP0 = 1;

  return kDifPlicOk;
}


plic_result_t plic_software_irq_acknowledge()
{
  rv_plic_peri->MSIP0 = 0;

  return kDifPlicOk;
}


plic_result_t plic_software_irq_is_pending(bool *is_pending)
{
  uint32_t reg = rv_plic_peri->MSIP0;
  *is_pending = (reg == 1) ? true : false;

  return kDifPlicOk;
}


/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

// void plic_get_reg_and_bit_index(uint32_t *base_reg, plic_irq_id_t irq,
//                                 uint32_t *reg, uint16_t *bit_index)
// {
//   ptrdiff_t offset = plic_offset_from_reg0(irq);
//   *reg = *(base_reg + offset);
//   *bit_index = plic_irq_bit_index(irq);
// }

/**
 * Get an IE, IP or LE register offset (IE0_0, IE01, ...) from an IRQ source ID.
 *
 * With more than 32 IRQ sources, there is a multiple of these registers to
 * accommodate all the bits (1 bit per IRQ source). This function calculates
 * the offset for a specific IRQ source ID (ID 32 would be IE01, ...).
 */
static ptrdiff_t plic_offset_from_reg0(plic_irq_id_t irq) {
  uint8_t register_index = irq / RV_PLIC_PARAM_REG_WIDTH;
  return register_index * sizeof(uint32_t);
}

/**
 * 
 * Get an IE, IP, LE register bit index from an IRQ source ID.
 *
 * With more than 32 IRQ sources, there is a multiple of these registers to
 * accommodate all the bits (1 bit per IRQ source). This function calculates
 * the bit position within a register for a specific IRQ source ID (ID 32 would
 * be bit 0).
 */
static uint8_t plic_irq_bit_index(plic_irq_id_t irq) {
  return irq % RV_PLIC_PARAM_REG_WIDTH;
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
