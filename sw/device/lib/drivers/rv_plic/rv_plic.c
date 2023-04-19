/*
                              *******************
******************************* C SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : x-heep                                                       **
** filename : rv_plic.c                                                    **
** date     : 18/04/2023                                                   **
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
* @date   18/04/2023
* @brief  This is the main file for the HAL of the RV_PLIC peripheral
*
* In this file there are the defintions of the HAL functions for the RV_PLIC
* peripheral. They provide many low level functionalities to interact
* with the registers content, reading and writing them according to the specific
* function of each one.
*
*/

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


/**
 * Array of handler functions. Each entry is a callable handler function.
 * When an interrupt is serviced, its ID is detected and basing on it an
 * index is generated. This index will be used to address the proper
 * handler function inside this array.
*/
handler_funct_t handlers[PLIC_INTR_SRCS_NUM] = {&handler_irq_uart, 
                                                &handler_irq_gpio, 
                                                &handler_irq_i2c, 
                                                &handler_irq_spi};

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

/**
 * Returns the irq_sources_t source type of a given irq source ID
*/
static irq_sources_t plic_get_irq_src_type(plic_irq_id_t irq_id);

/**
 * Get an IE, IP or LE register offset (IE0_0, IE01, ...) from an IRQ source ID.
 *
 * With more than 32 IRQ sources, there is a multiple of these registers to
 * accommodate all the bits (1 bit per IRQ source). This function calculates
 * the offset for a specific IRQ source ID (ID 32 would be IE01, ...).
 */
static ptrdiff_t plic_offset_from_reg0(plic_irq_id_t irq);

/**
 * 
 * Get an IE, IP, LE register bit index from an IRQ source ID.
 *
 * With more than 32 IRQ sources, there is a multiple of these registers to
 * accommodate all the bits (1 bit per IRQ source). This function calculates
 * the bit position within a register for a specific IRQ source ID (ID 32 would
 * be bit 0).
 */
static uint8_t plic_irq_bit_index(plic_irq_id_t irq);

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

/* 
  Flag to handle the wait for interrupt.
  Set to 1 after an interrupt occours and the core is in wait for interrupt
  so the execution of the program can continue.
*/
uint8_t plic_intr_flag = 0;


/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

__attribute__((weak, optimize("O0"))) void handler_irq_uart(void) {

} 

__attribute__((weak, optimize("O0"))) void handler_irq_gpio(void) {
  
}

__attribute__((weak, optimize("O0"))) void handler_irq_i2c(void) {
  
}

__attribute__((weak, optimize("O0"))) void handler_irq_spi(void) {
  
}

void handler_irq_external(void)
{
  plic_irq_id_t int_id = 0;
  plic_result_t res = plic_irq_claim(&int_id);
  irq_sources_t type = plic_get_irq_src_type(int_id);

  if(type != IRQ_BAD)
  {
    // Calls the proper handler
    handlers[type]();
    plic_intr_flag = 1;

    plic_irq_complete(&int_id);
  }
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
  for(uint8_t i=0; i<RV_PLIC_LE_MULTIREG_COUNT; i++)
  {
    uint32_t *dest = (&rv_plic_peri->IP0 + i);
    *dest = 0;
  }


  /* Clears all the priority registers */ 
  for(uint8_t i=0; i<RV_PLIC_PARAM_NUM_SRC; i++)
  {
    uint32_t *dest = &rv_plic_peri->IP0 + i;
    *dest = 0;
  }

  /* Address of the first IE register */
  uint32_t *base = &rv_plic_peri->IE00;

  /* Clears the Interrupt Enable registers */
  for(uint8_t i=0; i<RV_PLIC_IE0_MULTIREG_COUNT; i++)
  {
    uint32_t *dest = base + i;
    *dest = 0;
  }
  
  /* Clears all the threshold registers */
  rv_plic_peri->THRESHOLD0 = 0;
  if(rv_plic_peri->THRESHOLD0 != 0)
  {
    return kPlicError;
  }


  /* clears software interrupts registers */
  rv_plic_peri->MSIP0 = 0;
  if(rv_plic_peri->MSIP0 != 0)
  {
    return kPlicError;
  }

  return kPlicOk;

}


plic_result_t plic_irq_set_enabled(plic_irq_id_t irq,
                                       plic_toggle_t state)
{
  if(irq >= RV_PLIC_PARAM_NUM_SRC)
  {
    return kPlicBadArg;
  }

  // Check that `state` is an accettable value
  if (state != kPlicToggleEnabled && state!=kPlicToggleDisabled)
  {
    return kPlicBadArg;
  }

  ptrdiff_t offset = plic_offset_from_reg0(irq);
  uint32_t *dest = &rv_plic_peri->IE00 + offset;

  // Get the destination bit in which to write
  uint16_t bit_index = plic_irq_bit_index(irq);

  *dest = bitfield_bit32_write(*dest, bit_index, state);

  return kPlicOk;
}


plic_result_t plic_irq_get_enabled(plic_irq_id_t irq,
                                       plic_toggle_t *state)
{
  if(irq >= RV_PLIC_PARAM_NUM_SRC)
  {
    return kPlicBadArg;
  }
  
  // Get the destination register 
  ptrdiff_t offset = plic_offset_from_reg0(irq);
  uint32_t *reg = &rv_plic_peri->IE00 + offset;

  // Get the destination bit in which to write
  uint16_t bit_index = plic_irq_bit_index(irq);

  // Reads the enabled/disabled bit 
  *state = bitfield_bit32_read(*reg, bit_index);

  return kPlicOk;

}


plic_result_t plic_irq_set_trigger(plic_irq_id_t irq,
                                           plic_irq_trigger_t trigger)
{
  if(irq >= RV_PLIC_PARAM_NUM_SRC)
  {
    return kPlicBadArg;
  }

  // Get the destination register 
  ptrdiff_t offset = plic_offset_from_reg0(irq);
  uint32_t *reg = &rv_plic_peri->LE0 + offset;

  // Get the destination bit in which to write
  uint16_t bit_index = plic_irq_bit_index(irq);

  // Updated the register with the new bit
  *reg = bitfield_bit32_write(*reg, bit_index, trigger);

  return kPlicOk;

}


plic_result_t plic_irq_set_priority(plic_irq_id_t irq, uint32_t priority)
{
  if(irq >= RV_PLIC_PARAM_NUM_SRC || priority > plicMaxPriority)
  {
    return kPlicBadArg;
  }

  // Writes the new priority into the proper register
  uint32_t *dest = &rv_plic_peri->PRIO0 + irq;
  *dest = priority;

  if(*dest != priority){
    return kPlicError;
  }

  return kPlicOk;
}


plic_result_t plic_target_set_threshold(uint32_t threshold)
{
  if(threshold > plicMaxPriority)
  {
    return kPlicBadArg;
  }

  rv_plic_peri->THRESHOLD0 = threshold;

  return kPlicOk;

}


plic_result_t plic_irq_is_pending(plic_irq_id_t irq,
                                          bool *is_pending)
{
  if(irq >= RV_PLIC_PARAM_NUM_SRC || is_pending == NULL)
  {
    return kPlicBadArg;
  }

  // Get the destination register 
  ptrdiff_t offset = plic_offset_from_reg0(irq);
  uint32_t *reg = &rv_plic_peri->IP0 + offset;

  // Get the destination bit in which to write
  uint16_t bit_index = plic_irq_bit_index(irq);

  *is_pending = bitfield_bit32_read(*reg, bit_index);

  return kPlicOk;
}


plic_result_t plic_irq_claim(plic_irq_id_t *claim_data) 
{
  if (claim_data == NULL) 
  {
    return kPlicBadArg;
  }
  
  *claim_data = rv_plic_peri->CC0;

  return kPlicOk;
}


plic_result_t plic_irq_complete(const plic_irq_id_t *complete_data) 
{
  if (complete_data == NULL) 
  {
    return kPlicBadArg;
  }

  // Write back the claimed IRQ ID to the target specific CC register,
  // to notify the PLIC of the IRQ completion.
  rv_plic_peri->CC0 = *complete_data;

  return kPlicOk;
}


void plic_software_irq_force(void)
{
  rv_plic_peri->MSIP0 = 1;
}


void plic_software_irq_acknowledge(void)
{
  rv_plic_peri->MSIP0 = 0;
}


plic_result_t plic_software_irq_is_pending(void)
{
  return rv_plic_peri->MSIP0;
}


/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

static irq_sources_t plic_get_irq_src_type(plic_irq_id_t irq_id)
{
  if (irq_id < UART_ID_START || irq_id > SPI_ID)
  {
    return IRQ_BAD;
  }
  else if (irq_id <= UART_ID_END)
  {
    return IRQ_UART_SRC;
  }
  else if (irq_id <= GPIO_ID_END)
  {
    return IRQ_GPIO_SRC;
  } 
  else if (irq_id <= I2C_ID_END)
  {
    return IRQ_I2C_SRC;
  }
  else 
  {
    return IRQ_SPI_SRC;
  }

}

static ptrdiff_t plic_offset_from_reg0(plic_irq_id_t irq) 
{
  uint8_t register_index = irq / RV_PLIC_PARAM_REG_WIDTH;
  return register_index * sizeof(uint32_t);
}

static uint8_t plic_irq_bit_index(plic_irq_id_t irq) 
{
  return irq % RV_PLIC_PARAM_REG_WIDTH;
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
