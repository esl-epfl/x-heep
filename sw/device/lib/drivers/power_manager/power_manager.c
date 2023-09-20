/*
                              *******************
******************************* C SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : X-HEEP                                                       **
** filename : power_manager.c                                              **
** version  : 1                                                            **
** date     : 22/08/23                                                     **
**                                                                         **
*****************************************************************************
**                                                                         **
** Copyright (c) EPFL contributors.                                        **
** All rights reserved.                                                    **
**                                                                         **
*****************************************************************************

VERSION HISTORY:
----------------
Version     : 1
Date        : -/-/-
Revised by  : -
Description : Original version

*/

/***************************************************************************/
/***************************************************************************/

/**
* @file   power_manager.c
* @date   22/08/23
* @brief  The Power Manager driver to set up and use the power manager
* peripheral
*
* X-HEEP power domains (take from paper)
*/

/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

#include "power_manager.h"      // Generated

#include "bitfield.h"

#include "power_manager_regs.h" // Generated

#include "x-heep.h"

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

/**
 *
 *
 *
 */
typedef struct power_manager_counters {
  /**
   * The counter to set and unset the reset and switch of the CPU.
   */
  uint32_t reset_off;
  /**
  *
  *
  *
  */
  uint32_t reset_on;
  /**
  *
  *
  *
  */
  uint32_t switch_off;
  /**
  *
  *
  *
  */
  uint32_t switch_on;
  /**
  *
  *
  *
  */
  uint32_t iso_off;
  /**
  *
  *
  *
  */
  uint32_t iso_on;
  /**
  *
  *
  *
  */
  uint32_t retentive_off;
  /**
  *
  *
  *
  */
  uint32_t retentive_on;
} power_manager_counters_t;

/**
 * @todo completely wrong, copy paste from DMA
 * Interrupts must be enabled in the INTERRUPT register of the DMA.
 * Only one at a time. In case more than one is interrupt is to be triggered,
 * at the same time (last byte of a transaction of size multiple of the window
 * size) only the lowest ID is triggered.
 */
typedef enum
{
    INTR_EN_NONE        = 0x0, /*!< No interrupts should be triggered. */
    INTR_EN_TRANS_DONE  = 0x1, /*!< The TRANS_DONE interrupt is a fast
    interrupt that is triggered once the whole transaction has finished. */
    INTR_EN_WINDOW_DONE = 0x2, /*!< The WINDOW_DONE interrupt is a PLIC
    interrupt that is triggered every given number of bytes (set in the
    transaction configuration as win_du). */
    INTR_EN__size,
} inter_en_t;

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

/**
 * Control Block (CB) of the power manager peripheral.
 * Has variables and constant necessary/useful for its control.
 */
static struct
{
    /**
    * Flag to lower as soon as a transaction is launched, and raised by the
    * interrupt handler once it has finished. Only used when the end event is
    * set to INTR_WAIT.
    */
    uint8_t intrFlag;

    /**
     * memory mapped structure of a power manager.
     */
    power_manager *peri;

    /**
     * memory mapped structure of a power manager.
     */
    power_manager_counters_t counters;

}power_manager_cb;

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

void power_manager_init( power_manager *peri )
{
    /*
     * If a power_manager peripheral was provided, use that one, otherwise use the
     * integrated one.
     */
    power_manager_cb.peri = peri ? peri : power_manager_peri;

    /* Clear all values in the power_manager registers. */
    /**
     * @todo check register initializaton
     * */
    // power_manager_cb.peri->WAKEUP_STATE                    = 0;
    // power_manager_cb.peri->RESTORE_ADDRESS                 = 0;
    // power_manager_cb.peri->EN_WAIT_FOR_INTR                = 0;
    // power_manager_cb.peri->INTR_STATE                      = 0;
    // power_manager_cb.peri->POWER_GATE_CORE                 = 0;
    // power_manager_cb.peri->POWER_GATE_CORE_ACK             = 0;
    // power_manager_cb.peri->CPU_RESET_ASSERT_COUNTER        = 0;
    // power_manager_cb.peri->CPU_RESET_DEASSERT_COUNTER      = 0;
    // power_manager_cb.peri->CPU_SWITCH_OFF_COUNTER          = 0;
    // power_manager_cb.peri->CPU_SWITCH_ON_COUNTER           = 0;
    // power_manager_cb.peri->CPU_WAIT_ACK_SWITCH_ON_COUNTER  = 0;
    // power_manager_cb.peri->CPU_ISO_OFF_COUNTER             = 0;
    // power_manager_cb.peri->CPU_ISO_ON_COUNTER              = 0;
    // power_manager_cb.peri->CPU_COUNTERS_STOP               = 0;
    // power_manager_cb.peri->POWER_GATE_PERIPH_ACK           = 0;
    // power_manager_cb.peri->PERIPH_RESET                    = 0;
    // power_manager_cb.peri->PERIPH_SWITCH                   = 0;
    // power_manager_cb.peri->PERIPH_WAIT_ACK_SWITCH_ON       = 0;
    // power_manager_cb.peri->PERIPH_ISO                      = 0;
    // power_manager_cb.peri->PERIPH_CLK_GATE                 = 0;
    // power_manager_cb.peri->RAM_0_CLK_GATE                  = 0;
    // power_manager_cb.peri->POWER_GATE_RAM_BLOCK_0_ACK      = 0;
    // power_manager_cb.peri->RAM_0_SWITCH                    = 0;
    // power_manager_cb.peri->RAM_0_WAIT_ACK_SWITCH_ON        = 0;
    // power_manager_cb.peri->RAM_0_ISO                       = 0;
    // power_manager_cb.peri->RAM_0_RETENTIVE                 = 0;
    // power_manager_cb.peri->RAM_1_CLK_GATE                  = 0;
    // power_manager_cb.peri->POWER_GATE_RAM_BLOCK_1_ACK      = 0;
    // power_manager_cb.peri->RAM_1_SWITCH                    = 0;
    // power_manager_cb.peri->RAM_1_WAIT_ACK_SWITCH_ON        = 0;
    // power_manager_cb.peri->RAM_1_ISO                       = 0;
    // power_manager_cb.peri->RAM_1_RETENTIVE                 = 0;
    // power_manager_cb.peri->MONITOR_POWER_GATE_CORE         = 0;
    // power_manager_cb.peri->MONITOR_POWER_GATE_PERIPH       = 0;
    // power_manager_cb.peri->MONITOR_POWER_GATE_RAM_BLOCK_0  = 0;
    // power_manager_cb.peri->MONITOR_POWER_GATE_RAM_BLOCK_1  = 0;
    // power_manager_cb.peri->MASTER_CPU_FORCE_SWITCH_OFF     = 0;
    // power_manager_cb.peri->MASTER_CPU_FORCE_SWITCH_ON      = 0;
    // power_manager_cb.peri->MASTER_CPU_FORCE_RESET_ASSERT   = 0;
    // power_manager_cb.peri->MASTER_CPU_FORCE_RESET_DEASSERT = 0;
    // power_manager_cb.peri->MASTER_CPU_FORCE_ISO_OFF        = 0;
    // power_manager_cb.peri->MASTER_CPU_FORCE_ISO_ON         = 0;
}

void __attribute__ ((noinline)) power_gate_core_asm()
{
    asm volatile (

        // write POWER_GATE_CORE[0] = 1
        "lui a0, %[base_address_20bit]\n"
        "li  a1, 1\n"
        "sw  a1, %[power_manager_power_gate_core_reg_offset](a0)\n"

        // write WAKEUP_STATE[0] = 1
        "sw  a1, %[power_manager_wakeup_state_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_power_gate_core_reg_offset] "i" (POWER_MANAGER_POWER_GATE_CORE_REG_OFFSET), \
        [power_manager_wakeup_state_reg_offset] "i" (POWER_MANAGER_WAKEUP_STATE_REG_OFFSET) : "a0", "a1" \
    );

    asm volatile (

        // write registers
        "la a0, __power_manager_start\n"
        "sw x1,  0(a0)\n"
        "sw x2,  4(a0)\n"
        "sw x3,  8(a0)\n"
        "sw x4,  12(a0)\n"
        "sw x5,  16(a0)\n"
        "sw x6,  20(a0)\n"
        "sw x7,  24(a0)\n"
        "sw x8,  28(a0)\n"
        "sw x9,  32(a0)\n"
        "sw x10, 36(a0)\n"
        "sw x11, 40(a0)\n"
        "sw x12, 44(a0)\n"
        "sw x13, 48(a0)\n"
        "sw x14, 52(a0)\n"
        "sw x15, 56(a0)\n"
        "sw x16, 60(a0)\n"
        "sw x17, 64(a0)\n"
        "sw x18, 68(a0)\n"
        "sw x19, 72(a0)\n"
        "sw x20, 76(a0)\n"
        "sw x21, 80(a0)\n"
        "sw x22, 88(a0)\n"
        "sw x23, 92(a0)\n"
        "sw x24, 96(a0)\n"
        "sw x25, 100(a0)\n"
        "sw x26, 104(a0)\n"
        "sw x27, 108(a0)\n"
        "sw x28, 112(a0)\n"
        "sw x29, 116(a0)\n"
        "sw x30, 120(a0)\n"
        "sw x31, 124(a0)\n"
        //csr
        "csrr a1, mstatus\n"
        "sw a1, 128(a0)\n"
        "csrr a1, mie\n"
        "sw a1, 132(a0)\n"
        "csrr a1, mtvec\n"
        "sw a1, 136(a0)\n"
        "csrr a1, mscratch\n"
        "sw a1, 140(a0)\n"
        "csrr a1, mepc\n"
        "sw a1, 144(a0)\n"
        "csrr a1, mcause\n"
        "sw a1, 148(a0)\n"
        "csrr a1, mtval\n"
        "sw a1, 152(a0)\n"
        "csrr a1, mcycle\n"
        "sw a1, 156(a0)\n"
        "csrr a1, minstret\n"
        "sw a1, 160(a0)\n"  : : : "a0", "a1" \
    );

    asm volatile (

        // write RESTORE_ADDRESS[31:0] = PC
        "lui a0, %[base_address_20bit]\n"
        "la  a1, wakeup\n"
        "sw  a1, %[power_manager_restore_address_reg_offset](a0)\n"

        // wait for interrupt
        "wfi\n"

        // ----------------------------
        // power-gate
        // ----------------------------

        // ----------------------------
        // wake-up
        // ----------------------------

        // write POWER_GATE_CORE[0] = 0
        "wakeup:"
        "lui a0, %[base_address_20bit]\n"
        "sw  x0, %[power_manager_power_gate_core_reg_offset](a0)\n"

        // write WAKEUP_STATE[0] = 0
        "sw x0, %[power_manager_wakeup_state_reg_offset](a0)\n"

        // write RESTORE_ADDRESS[31:0] = 0
        "sw x0, %[power_manager_restore_address_reg_offset](a0)\n" : : \
        \
        [base_address_20bit] "i" (POWER_MANAGER_START_ADDRESS >> 12), \
        [power_manager_power_gate_core_reg_offset] "i" (POWER_MANAGER_POWER_GATE_CORE_REG_OFFSET), \
        [power_manager_wakeup_state_reg_offset] "i" (POWER_MANAGER_WAKEUP_STATE_REG_OFFSET), \
        [power_manager_restore_address_reg_offset] "i" (POWER_MANAGER_RESTORE_ADDRESS_REG_OFFSET) : "a0", "a1" \
    );

    asm volatile (

        // write CORE_REG_Xn[31:0] = Xn
        "la a0, __power_manager_start\n"
        //one of the following load is gonna overwrite a0, but a0 was already stored before to the right value
        "lw x1,  0(a0)\n"
        "lw x2,  4(a0)\n"
        "lw x3,  8(a0)\n"
        "lw x4,  12(a0)\n"
        "lw x5,  16(a0)\n"
        "lw x6,  20(a0)\n"
        "lw x7,  24(a0)\n"
        "lw x8,  28(a0)\n"
        "lw x9,  32(a0)\n"
        "lw x10, 36(a0)\n"
        "lw x11, 40(a0)\n"
        "lw x12, 44(a0)\n"
        "lw x13, 48(a0)\n"
        "lw x14, 52(a0)\n"
        "lw x15, 56(a0)\n"
        "lw x16, 60(a0)\n"
        "lw x17, 64(a0)\n"
        "lw x18, 68(a0)\n"
        "lw x19, 72(a0)\n"
        "lw x20, 76(a0)\n"
        "lw x21, 80(a0)\n"
        "lw x22, 88(a0)\n"
        "lw x23, 92(a0)\n"
        "lw x24, 96(a0)\n"
        "lw x25, 100(a0)\n"
        "lw x26, 104(a0)\n"
        "lw x27, 108(a0)\n"
        "lw x28, 112(a0)\n"
        "lw x29, 116(a0)\n"
        "lw x30, 120(a0)\n"
        "lw x31, 124(a0)\n"
        //csr
        "lw a1, 128(a0)\n"
        "csrw mstatus, a1\n"
        "lw a1, 132(a0)\n"
        "csrw mie, a1\n"
        "lw a1, 136(a0)\n"
        "csrw mtvec, a1\n"
        "lw a1, 140(a0)\n"
        "csrw mscratch, a1\n"
        "lw a1, 144(a0)\n"
        "csrw mepc, a1\n"
        "lw a1, 148(a0)\n"
        "csrw mcause, a1\n"
        "lw a1, 152(a0)\n"
        "csrw mtval, a1\n"
        "lw a1, 156(a0)\n"
        "csrw mcycle, a1\n"
        "lw a1, 160(a0)\n"
        "csrw minstret, a1\n": : : "a0", "a1" \
    );

    return;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_core(power_manager_sel_intr_t sel_intr)
{
    // set counters
    power_manager_cb.peri->CPU_RESET_ASSERT_COUNTER = power_manager_cb.counters.reset_off;
    power_manager_cb.peri->CPU_RESET_DEASSERT_COUNTER = power_manager_cb.counters.reset_on;
    power_manager_cb.peri->CPU_SWITCH_OFF_COUNTER = power_manager_cb.counters.switch_off;
    power_manager_cb.peri->CPU_SWITCH_ON_COUNTER = power_manager_cb.counters.switch_on;
    power_manager_cb.peri->CPU_ISO_OFF_COUNTER = power_manager_cb.counters.iso_off;
    power_manager_cb.peri->CPU_ISO_ON_COUNTER = power_manager_cb.counters.iso_on;

    // enable wakeup timers
    power_manager_cb.peri->EN_WAIT_FOR_INTR = 1 << sel_intr;
    power_manager_cb.peri->INTR_STATE = 0x0;

    // enable wait for SWITCH ACK
    #ifdef TARGET_PYNQ_Z2
        power_manager_cb.peri->CPU_WAIT_ACK_SWITCH_ON_COUNTER = bitfield_bit32_write(power_manager_cb.peri->CPU_WAIT_ACK_SWITCH_ON_COUNTER, POWER_MANAGER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_BIT, false);
    #else
        power_manager_cb.peri->CPU_WAIT_ACK_SWITCH_ON_COUNTER = bitfield_bit32_write(power_manager_cb.peri->CPU_WAIT_ACK_SWITCH_ON_COUNTER, POWER_MANAGER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_CPU_WAIT_ACK_SWITCH_ON_COUNTER_BIT, true);
    #endif

    asm volatile ("nop\n;"); // Necessary in SIM to swith on/off the ack signal
    power_gate_core_asm();

    // clean up states
    power_manager_cb.peri->EN_WAIT_FOR_INTR = 0x0;
    power_manager_cb.peri->INTR_STATE = 0x0;

    // stop counters
    power_manager_cb.peri->CPU_COUNTERS_STOP = bitfield_bit32_write(power_manager_cb.peri->CPU_COUNTERS_STOP, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_RESET_ASSERT_STOP_BIT_COUNTER_BIT, true);
    power_manager_cb.peri->CPU_COUNTERS_STOP = bitfield_bit32_write(power_manager_cb.peri->CPU_COUNTERS_STOP, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_RESET_DEASSERT_STOP_BIT_COUNTER_BIT, true);
    power_manager_cb.peri->CPU_COUNTERS_STOP = bitfield_bit32_write(power_manager_cb.peri->CPU_COUNTERS_STOP, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_SWITCH_OFF_STOP_BIT_COUNTER_BIT, true);
    power_manager_cb.peri->CPU_COUNTERS_STOP = bitfield_bit32_write(power_manager_cb.peri->CPU_COUNTERS_STOP, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_SWITCH_ON_STOP_BIT_COUNTER_BIT, true);
    power_manager_cb.peri->CPU_COUNTERS_STOP = bitfield_bit32_write(power_manager_cb.peri->CPU_COUNTERS_STOP, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_ISO_OFF_STOP_BIT_COUNTER_BIT, true);
    power_manager_cb.peri->CPU_COUNTERS_STOP = bitfield_bit32_write(power_manager_cb.peri->CPU_COUNTERS_STOP, POWER_MANAGER_CPU_COUNTERS_STOP_CPU_ISO_ON_STOP_BIT_COUNTER_BIT, true);

    return kPowerManagerOk_e;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_periph(power_manager_sel_state_t sel_state)
{
    uint32_t reg = 0;

    #ifdef TARGET_PYNQ_Z2
        power_manager_cb.peri->PERIPH_WAIT_ACK_SWITCH_ON = 0x0;
    #else
        power_manager_cb.peri->PERIPH_WAIT_ACK_SWITCH_ON = 0x1;
    #endif

    if (sel_state == kOn_e)
    {
        for (int i=0; i<power_manager_cb.counters.switch_on; i++) asm volatile ("nop\n;");
        power_manager_cb.peri->PERIPH_SWITCH = 0x0;
        for (int i=0; i<power_manager_cb.counters.iso_off; i++) asm volatile ("nop\n;");
        power_manager_cb.peri->PERIPH_ISO = 0x0;
        for (int i=0; i<power_manager_cb.counters.reset_off; i++) asm volatile ("nop\n;");
        power_manager_cb.peri->PERIPH_RESET = 0x0;
    }
    else
    {
        for (int i=0; i<power_manager_cb.counters.iso_on; i++) asm volatile ("nop\n;");
        power_manager_cb.peri->PERIPH_ISO = 0x1;
        for (int i=0; i<power_manager_cb.counters.switch_off; i++) asm volatile ("nop\n;");
        power_manager_cb.peri->PERIPH_SWITCH = 0x1;
        for (int i=0; i<power_manager_cb.counters.reset_on; i++) asm volatile ("nop\n;");
        power_manager_cb.peri->PERIPH_RESET = 0x1;
    }

    return kPowerManagerOk_e;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_ram_block(uint32_t sel_block, power_manager_sel_state_t sel_state)
{
    uint32_t reg = 0;

    /**
    * @todo Add ram sel sanity check (< num of banks)
    */

    if (sel_state == kOn_e)
    {
        #ifdef TARGET_PYNQ_Z2
            *(&power_manager_cb.peri->RAM_0_WAIT_ACK_SWITCH_ON + sel_block*6) = 0x0;
        #else
            *(&power_manager_cb.peri->RAM_0_WAIT_ACK_SWITCH_ON + sel_block*6) = 0x1;
        #endif
        for (int i=0; i<power_manager_cb.counters.switch_on; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->RAM_0_SWITCH + sel_block*6) = 0x0;
        for (int i=0; i<power_manager_cb.counters.iso_off; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->RAM_0_ISO + sel_block*6) = 0x0;
    }
    else if (sel_state == kOff_e)
    {
        #ifdef TARGET_PYNQ_Z2
            *(&power_manager_cb.peri->RAM_0_WAIT_ACK_SWITCH_ON + sel_block*6) = 0x0;
        #else
            *(&power_manager_cb.peri->RAM_0_WAIT_ACK_SWITCH_ON + sel_block*6) = 0x1;
        #endif
        for (int i=0; i<power_manager_cb.counters.iso_on; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->RAM_0_ISO + sel_block*6) = 0x1;
        for (int i=0; i<power_manager_cb.counters.switch_off; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->RAM_0_SWITCH + sel_block*6) = 0x1;
    }
    else if (sel_state == kRetOn_e)
    {
        *(&power_manager_cb.peri->RAM_0_WAIT_ACK_SWITCH_ON + sel_block*6) = 0x0;
        for (int i=0; i<power_manager_cb.counters.retentive_on; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->RAM_0_RETENTIVE + sel_block*6) = 0x1;
    }
    else
    {
        *(&power_manager_cb.peri->RAM_0_WAIT_ACK_SWITCH_ON + sel_block*6) = 0x0;
        for (int i=0; i<power_manager_cb.counters.retentive_off; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->RAM_0_RETENTIVE + sel_block*6) = 0x0;
    }

    return kPowerManagerOk_e;
}

power_manager_result_t __attribute__ ((noinline)) power_gate_external(uint32_t sel_external, power_manager_sel_state_t sel_state)
{

    /**
    * @todo Add ram sel sanity check (< num of banks)
    */
   #ifdef EXTERNAL_0_CLK_GATE

    uint32_t reg = 0;

    if (sel_state == kOn_e)
    {
        #ifdef TARGET_PYNQ_Z2
            *(&power_manager_cb.peri->EXTERNAL_0_WAIT_ACK_SWITCH_ON + sel_external*7) = 0x0;
        #else
            *(&power_manager_cb.peri->EXTERNAL_0_WAIT_ACK_SWITCH_ON + sel_external*7) = 0x1;
        #endif
        for (int i=0; i<external_counters->switch_on; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->EXTERNAL_0_SWITCH + sel_external*7) = 0x0;
        for (int i=0; i<external_counters->iso_off; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->EXTERNAL_0_ISO + sel_external*7) = 0x0;
        for (int i=0; i<external_counters->reset_off; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->EXTERNAL_0_RESET + sel_external*7) = 0x0;
    }
    else if (sel_state == kOff_e)
    {
        #ifdef TARGET_PYNQ_Z2
            *(&power_manager_cb.peri->EXTERNAL_0_WAIT_ACK_SWITCH_ON + sel_external*7) = 0x0;
        #else
            *(&power_manager_cb.peri->EXTERNAL_0_WAIT_ACK_SWITCH_ON + sel_external*7) = 0x1;
        #endif
        for (int i=0; i<external_counters->iso_on; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->EXTERNAL_0_ISO + sel_external*7) = 0x1;
        for (int i=0; i<external_counters->switch_off; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->EXTERNAL_0_SWITCH + sel_external*7) = 0x1;
        for (int i=0; i<external_counters->reset_on; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->EXTERNAL_0_RESET + sel_external*7) = 0x1;
    }
    else if (sel_state == kRetOn_e)
    {
        *(&power_manager_cb.peri->EXTERNAL_0_WAIT_ACK_SWITCH_ON + sel_external*7) = 0x0;
        for (int i=0; i<external_counters->retentive_on; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->EXTERNAL_RAM_0_RETENTIVE + sel_external*7) = 0x1;
    }
    else
    {
        *(&power_manager_cb.peri->EXTERNAL_0_WAIT_ACK_SWITCH_ON + sel_external*7) = 0x0;
        for (int i=0; i<external_counters->retentive_off; i++) asm volatile ("nop\n;");
        *(&power_manager_cb.peri->EXTERNAL_RAM_0_RETENTIVE + sel_external*7) = 0x0;
    }

    return kPowerManagerOk_e;
    #endif
}

uint32_t periph_power_domain_is_off()
{
    uint32_t switch_state;

    switch_state = power_manager_cb.peri->POWER_GATE_PERIPH_ACK;

    return switch_state == 0;
}

uint32_t ram_block_power_domain_is_off(uint32_t sel_block)
{
    uint32_t switch_state;

    switch_state = *(&power_manager_cb.peri->POWER_GATE_RAM_BLOCK_0_ACK + sel_block*6);

    return switch_state == 0;
}

uint32_t external_power_domain_is_off(uint32_t sel_external)
{
    #ifdef POWER_GATE_EXTERNAL_0_ACK
    uint32_t switch_state;

    switch_state = power_manager_cb.peri->POWER_GATE_EXTERNAL_X_ACK(sel_external);

    return switch_state == 0;
    #endif
}

power_manager_result_t power_gate_counters_init(uint32_t reset_off, uint32_t reset_on, uint32_t switch_off, uint32_t switch_on, uint32_t iso_off, uint32_t iso_on, uint32_t retentive_off, uint32_t retentive_on)
{
    power_manager_cb.counters.reset_off     = reset_off;
    power_manager_cb.counters.reset_on      = reset_on;
    power_manager_cb.counters.switch_off    = switch_off;
    power_manager_cb.counters.switch_on     = switch_on;
    power_manager_cb.counters.iso_off       = iso_off;
    power_manager_cb.counters.iso_on        = iso_on;
    power_manager_cb.counters.retentive_off = retentive_off;
    power_manager_cb.counters.retentive_on  = retentive_on;

    return kPowerManagerOk_e;
}

monitor_signals_t monitor_power_gate_core()
{
    uint32_t reg = 0;
    monitor_signals_t monitor_signals;

    reg = power_manager_cb.peri->MONITOR_POWER_GATE_CORE;

    monitor_signals.kSwitch_e = reg & 0x1;
    monitor_signals.kIso_e    = (reg & 0x2) >> 1;
    monitor_signals.kReset_e  = (reg & 0x4) >> 2;

    return monitor_signals;
}

monitor_signals_t monitor_power_gate_periph()
{
    uint32_t reg = 0;
    monitor_signals_t monitor_signals;

    reg = power_manager_cb.peri->MONITOR_POWER_GATE_PERIPH;

    monitor_signals.kSwitch_e = reg & 0x1;
    monitor_signals.kIso_e    = (reg & 0x2) >> 1;
    monitor_signals.kReset_e  = (reg & 0x4) >> 2;

    return monitor_signals;
}

monitor_signals_t monitor_power_gate_ram_block(uint32_t sel_block)
{
    uint32_t reg = 0;
    monitor_signals_t monitor_signals;

    reg = *(&power_manager_cb.peri->MONITOR_POWER_GATE_RAM_BLOCK_0 + sel_block);

    monitor_signals.kSwitch_e = reg & 0x1;
    monitor_signals.kIso_e    = (reg & 0x2) >> 1;
    monitor_signals.kReset_e  = 0x1;

    return monitor_signals;
}

monitor_signals_t monitor_power_gate_external(uint32_t sel_external)
{
    #ifdef MONITOR_POWER_GATE_EXTERNAL_0
    uint32_t reg = 0;
    monitor_signals_t monitor_signals;

    reg = *(&power_manager_cb.peri->MONITOR_POWER_GATE_EXTERNAL_0 + sel_external);

    monitor_signals.kSwitch_e = reg & 0x1;
    monitor_signals.kIso_e    = (reg & 0x2) >> 1;
    monitor_signals.kReset_e  = (reg & 0x4) >> 2;

    return monitor_signals;
    #endif
}

/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
