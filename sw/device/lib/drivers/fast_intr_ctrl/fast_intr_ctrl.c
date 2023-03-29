/*
                              *******************
******************************* C SOURCE FILE *****************************
**                            *******************                          
**                                                                         
** project  : X-HEEP                                                       
** filename : fast_intr_ctrl.c                                                      
** version  : 1                                                            
** date     : 27/03/23                                                     
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
* @file   fast_intr_ctrl.c
* @date   27/03/23
* @brief  The fast interrupt controller peripheral driver 
*/

/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "fast_intr_ctrl.h"
#include <stddef.h>
#include <stdint.h>
#include "mmio.h"
#include "core_v_mini_mcu.h"
#include "fast_intr_ctrl_regs.h"  // Generated.
#include "fast_intr_ctrl_structs.h"

/* interrupts */
// #include "handler.h"
#include "csr.h"
// #include "stdasm.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

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

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

fast_intr_ctrl_result_t clear_fast_interrupt(fast_intr_ctrl_fast_interrupt_t\
 fast_interrupt)
{
    /* masking a 32b go write in reg*/
    uint32_t reg = 0;
    reg = bitfield_bit32_write(reg, fast_interrupt, true);
    /* write in reg through structure */
    fast_intr_ctrl_peri->FAST_INTR_CLEAR = reg;
    return kFastIntrCtrlOk_e;
}

__attribute__((weak)) void fic_irq_timer_1(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_timer_2(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_timer_3(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_dma(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_spi(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_spi_flash(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_gpio_0(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_gpio_1(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_gpio_2(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_gpio_3(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_gpio_4(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_gpio_5(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_gpio_6(void)
{
    volatile uint8_t i;
    i++;
}

__attribute__((weak)) void fic_irq_gpio_7(void)
{
    volatile uint8_t i;
    i++;
}

/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

void handler_irq_fast_timer_1(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kTimer_1_fic_e);
    // call the weak fic handler
    fic_irq_timer_1();
}

void handler_irq_fast_timer_2(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kTimer_2_fic_e);
    // call the weak fic handler
    fic_irq_timer_2();
}

void handler_irq_fast_timer_3(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kTimer_3_fic_e);
    // call the weak fic handler
    fic_irq_timer_3();
}

void handler_irq_fast_dma(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kDma_fic_e);
    // call the weak fic handler
    fic_irq_dma();
}

void handler_irq_fast_spi(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kSpi_fic_e);
    // call the weak fic handler
    fic_irq_spi();
}

void handler_irq_fast_spi_flash(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kSpiFlash_fic_e);
    // call the weak fic handler
    fic_irq_spi_flash();
}

void handler_irq_fast_gpio_0(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kGpio_0_fic_e);
    // call the weak fic handler
    fic_irq_gpio_0();
}

void handler_irq_fast_gpio_1(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kGpio_1_fic_e);
    // call the weak fic handler
    fic_irq_gpio_1();
}

void handler_irq_fast_gpio_2(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kGpio_2_fic_e);
    // call the weak fic handler
    fic_irq_gpio_2();
}

void handler_irq_fast_gpio_3(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kGpio_3_fic_e);
    // call the weak fic handler
    fic_irq_gpio_3();
}

void handler_irq_fast_gpio_4(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kGpio_4_fic_e);
    // call the weak fic handler
    fic_irq_gpio_4();
}

void handler_irq_fast_gpio_5(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kGpio_5_fic_e);
    // call the weak fic handler
    fic_irq_gpio_5();
}

void handler_irq_fast_gpio_6(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kGpio_6_fic_e);
    // call the weak fic handler
    fic_irq_gpio_6();
}

void handler_irq_fast_gpio_7(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(kGpio_7_fic_e);
    // call the weak fic handler
    fic_irq_gpio_7();
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
