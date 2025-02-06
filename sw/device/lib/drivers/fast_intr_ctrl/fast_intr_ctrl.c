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
#ifdef __cplusplus
extern "C" {
#endif

#include "fast_intr_ctrl.h"
#include "core_v_mini_mcu.h"
#include "fast_intr_ctrl_regs.h"  // Generated.
#include "fast_intr_ctrl_structs.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

/**
 * The RISC-V interrupt vector will not include the addresses of the handlers,
 * instead, it includes (uncompressed) instructions. Thus the interrupt vector
 * will include `j <interrupt handler name>` for each handler.
 * 
 * The only requirement on the symbol in the jump is that it must be correctly
 * aligned. If the processor supports the C extension, this can be 2-byte
 * aligned, but 4-byte aligned is compatible with all RISC-V processors.
 * 
 * If the processor is not using interrupt vectoring, then there will be a
 * single address where interrupts jump to, which will either contain a function
 * (which will need to be aligned), or will contain a jump to a function, again
 * which will need to be aligned.
 * 
 * You only need to use this ABI for handlers that are the first function called
 * in an interrupt handler. Subsequent functions can just use the regular RISC-V
 * calling convention.
 */
#define INTERRUPT_HANDLER_ABI __attribute__((aligned(4), interrupt))

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
 * @brief Fast timer 1 irq handler. The first entry point when timer 1 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_timer_1(void);

/**
 * @brief Fast timer 2 irq handler. The first entry point when timer 2 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_timer_2(void);

/**
 * @brief Fast timer 3 irq handler. The first entry point when timer 3 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_timer_3(void);

/**
 * @brief Fast dma irq handler. The first entry point when dma interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_dma(void);

/**
 * @brief Fast spi irq handler. The first entry point when spi interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_spi(void);

/**
 * @brief Fast spi flash irq handler. The first entry point when spi flash 
 * interrupt is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_spi_flash(void);

/**
 * @brief Fast gpio 0 irq handler. The first entry point when gpio 0 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_0(void);

/**
 * @brief Fast gpio 1 irq handler. The first entry point when gpio 1 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_1(void);

/**
 * @brief Fast gpio 2 irq handler. The first entry point when gpio 2 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_2(void);

/**
 * @brief Fast gpio 3 irq handler. The first entry point when gpio 3 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_3(void);

/**
 * @brief Fast gpio 4 irq handler. The first entry point when gpio 4 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_4(void);

/**
 * @brief Fast gpio 5 irq handler. The first entry point when gpio 5 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_5(void);

/**
 * @brief Fast gpio 6 irq handler. The first entry point when gpio 6 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_6(void);

/**
 * @brief Fast gpio 7 irq handler. The first entry point when gpio 7 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_gpio_7(void);

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
fast_intr_ctrl_result_t enable_fast_interrupt(fast_intr_ctrl_fast_interrupt_t\
 fast_interrupt, bool enable) 
{
    /* masking a 32b go write in reg*/
    uint32_t reg = fast_intr_ctrl_peri->FAST_INTR_ENABLE;
    reg = bitfield_bit32_write(reg, fast_interrupt, enable);
    /* write in reg through structure */
    fast_intr_ctrl_peri->FAST_INTR_ENABLE = reg;
    return kFastIntrCtrlOk_e;
}

fast_intr_ctrl_result_t enable_all_fast_interrupts(bool enable) 
{
    fast_intr_ctrl_peri->FAST_INTR_ENABLE = enable ? 0x7fff : 0x0000;
    return kFastIntrCtrlOk_e;
}


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

__attribute__((weak, optimize("O0"))) void fic_irq_timer_1(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_timer_2(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_timer_3(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_dma(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_spi(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_spi_flash(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_gpio_0(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_gpio_1(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_gpio_2(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_gpio_3(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_gpio_4(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_gpio_5(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_gpio_6(void)
{
    /* Users should implement their non-weak version */
}

__attribute__((weak, optimize("O0"))) void fic_irq_gpio_7(void)
{
    /* Users should implement their non-weak version */
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
#ifdef __cplusplus
}
#endif

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
