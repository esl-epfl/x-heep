/*
                              *******************
******************************* H SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : X_HEEP                                                       **
** filename : fast_intr_ctrl.h  
** version  : 1                                             **
** date     : 27/03/23                                                   **
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
* @file   fast_intr_ctrl.h
* @date   27/03/23
* @brief  The fast interrupt controller peripheral driver 
*/

#ifndef _FAST_INTR_CTRL_H_
#define _FAST_INTR_CTRL_H_

/****************************************************************************/
/**                                                                        **/
/**                            MODULES USED                                **/
/**                                                                        **/
/****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>
#include "mmio.h"

/****************************************************************************/
/**                                                                        **/
/**                       DEFINITIONS AND MACROS                           **/
/**                                                                        **/
/****************************************************************************/



/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/**
 * possible returns of the clear_fast_interrupt function.
 */
typedef enum fast_intr_ctrl_result {
  kFastIntrCtrlOk_e    = 0, /*!< successfully done. */
  kFastIntrCtrlError_e = 1, /*!< an error occured. */
} fast_intr_ctrl_result_t;

/**
 * Specify the modules connected to FIC.
 * This enum is used as an input for clearing the specifc bit inside 
 * pending register while recieving an interrupt through FIC.
 */
typedef enum fast_intr_ctrl_fast_interrupt {
  kTimer_1_fic_e  = 0, /*!< Timer 1. */
  kTimer_2_fic_e  = 1, /*!< Timer 2. */
  kTimer_3_fic_e  = 2, /*!< Timer 3. */
  kDma_fic_e      = 3, /*!< DMA. */
  kSpi_fic_e      = 4, /*!< SPI. */
  kSpiFlash_fic_e = 5, /*!< SPI Flash. */
  kGpio_0_fic_e   = 6, /*!< GPIO 0. */
  kGpio_1_fic_e   = 7, /*!< GPIO 1. */
  kGpio_2_fic_e   = 8, /*!< GPIO 2. */
  kGpio_3_fic_e   = 9, /*!< GPIO 3. */
  kGpio_4_fic_e   = 10,/*!< GPIO 4. */
  kGpio_5_fic_e   = 11,/*!< GPIO 5. */
  kGpio_6_fic_e   = 12,/*!< GPIO 6. */
  kGpio_7_fic_e   = 13,/*!< GPIO 7. */
} fast_intr_ctrl_fast_interrupt_t;

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED VARIABLES                            **/
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

/**
 * @brief Enable the generation of the fast interrupt
 * peripheral by writing inside FAST_INTR_ENABLE
 * @param fast_interrupt specify the peripheral that will be enabled
 * @param enable enable value
 * @retval kFastIntrCtrlOk_e (= 0) if successfully set the bit
 * @retval kFastIntrCtrlError_e (= 1) if an error occured during operation
 */
fast_intr_ctrl_result_t enable_fast_interrupt(fast_intr_ctrl_fast_interrupt_t\
 fast_interrupt, bool enable);

 /**
 * @brief Enable the generation of the fast interrupt
 * peripheral by writing inside FAST_INTR_ENABLE
 * @param enable enable value
 * @retval kFastIntrCtrlOk_e (= 0) if successfully set the bit
 * @retval kFastIntrCtrlError_e (= 1) if an error occured during operation
 */
fast_intr_ctrl_result_t enable_all_fast_interrupts(bool enable);


/**
 * @brief clean the bit inside FAST_INTR_PENDING register for the requested 
 * peripheral by writing inside FAST_INTR_CLEAR
 * @param fast_interrupt specify the peripheral that will be cleared
 * @retval kFastIntrCtrlOk_e (= 0) if successfully clear the bit
 * @retval kFastIntrCtrlError_e (= 1) if an error occured during operation
 */
fast_intr_ctrl_result_t clear_fast_interrupt(fast_intr_ctrl_fast_interrupt_t\
 fast_interrupt);

/**
 * @brief fast interrupt controller irq for timer 1 
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_timer_1(void);

/**
 * @brief fast interrupt controller irq for timer 2 
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_timer_2(void);

/**
 * @brief fast interrupt controller irq for timer 3 
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_timer_3(void);

/**
 * @brief fast interrupt controller irq for dma 
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_dma(void);

/**
 * @brief fast interrupt controller irq for spi
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_spi(void);

/**
 * @brief fast interrupt controller irq for spi flash
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_spi_flash(void);

/**
 * @brief fast interrupt controller irq for gpio 0 
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_gpio_0(void);

/**
 * @brief fast interrupt controller irq for gpio 1
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_gpio_1(void);

/**
 * @brief fast interrupt controller irq for gpio 2
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_gpio_2(void);

/**
 * @brief fast interrupt controller irq for gpio 3
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_gpio_3(void);

/**
 * @brief fast interrupt controller irq for gpio 4
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_gpio_4(void);

/**
 * @brief fast interrupt controller irq for gpio 5
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_gpio_5(void);

/**
 * @brief fast interrupt controller irq for gpio 6 
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_gpio_6(void);

/**
 * @brief fast interrupt controller irq for gpio 7
 * `fast_intr_ctrl.c` provides a weak definition of this symbol, which can 
 * be overridden at link-time by providing an additional non-weak definition 
 * inside peripherals connected through FIC
 */
void fic_irq_gpio_7(void);


/****************************************************************************/
/**                                                                        **/
/**                          INLINE FUNCTIONS                              **/
/**                                                                        **/
/****************************************************************************/
#ifdef __cplusplus
}
#endif
/****************************************************************************/
/**                                                                        **/
/**                                EOF                                     **/
/**                                                                        **/
/****************************************************************************/
#endif  // _FAST_INTR_CTRL_H_
