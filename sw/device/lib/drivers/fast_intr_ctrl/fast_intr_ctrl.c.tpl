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

% for i in range(15):
/**
 * @brief Fast irq ${i} handler. The first entry point when timer 1 interrupt
 * is recieved through fic.
 * This function clear the responsible bit in FAST_INTR_PENDING then call a 
 * function that can be overriden inside peripherals.
 */
INTERRUPT_HANDLER_ABI void handler_irq_fast_${i}(void);

%endfor


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


<%
    intr_sources = xheep.get_rh().use_target_as_sv_multi("fast_irq_target", xheep.get_mcu_node())
    intrs = []
    for i, intr_s in enumerate(intr_sources):
        ep = xheep.get_rh().get_source_ep_copy(intr_s.split(".")[-1])
        intrs.append(ep.handler)
    for _ in range(i+1, 15):
        intrs.append("")
%>
% for i, intr in enumerate(intrs):
% if intr != "":

__attribute__((weak, optimize("O0"))) void ${intr}(void)
{
    /* Users should implement their non-weak version */
}
% endif

void handler_irq_fast_${i}(void)
{
    // The interrupt is cleared.
    clear_fast_interrupt(${i});
% if intr != "":
    // call the weak fic handler
    ${intr}();
% endif
}

% endfor

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
