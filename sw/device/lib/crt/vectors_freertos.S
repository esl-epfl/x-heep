/*
* Copyright 2019 ETH Zürich and University of Bologna
* Copyright 2022 EPFL
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

.section .vectors, "ax"
.option norvc
vector_table:
	//  0 : exception Handler and user software interrupt
	j freertos_risc_v_trap_handler
	//  1 : supervisor software interrupt
	j __no_irq_handler
	//  2 : unmapped
	j __no_irq_handler
	//  3 : machine software interrupt handler
	j freertos_risc_v_trap_handler
	//  4 : user timer interrupt
	j __no_irq_handler
	//  5 : supervisor timer interrupt
	j __no_irq_handler
	//  6 : unmapped
	j __no_irq_handler
	//  7 : machine timer interrupt handler
	j freertos_risc_v_mtimer_interrupt_handler
	//  8 : user external interrupt
	j __no_irq_handler
	//  9 : supervisor external interrupt
	j __no_irq_handler
	// 10 : unmapped
	j __no_irq_handler
	// 11 : machine external interrupt handler
	j freertos_risc_v_trap_handler
	// 12 : unmapped
	j __no_irq_handler
	// 13 : unmapped
	j __no_irq_handler
	// 14 : unmapped
	j __no_irq_handler
	// 15 : unmapped
	j __no_irq_handler
	// 16 : fast interrupt - timer_1
	j freertos_risc_v_trap_handler
	// 17 : fast interrupt - timer_2
	j freertos_risc_v_trap_handler
	// 18 : fast interrupt - timer_3
	j freertos_risc_v_trap_handler
	// 19 : fast interrupt - dma
	j freertos_risc_v_trap_handler
	// 20 : fast interrupt - spi
	j freertos_risc_v_trap_handler
	// 21 : fast interrupt - spi_flash
	j freertos_risc_v_trap_handler
	// 22 : fast interrupt - gpio_0
	j freertos_risc_v_trap_handler
	// 23 : fast interrupt - gpio_1
	j freertos_risc_v_trap_handler
	// 24 : fast interrupt - gpio_2
	j freertos_risc_v_trap_handler
	// 25 : fast interrupt - gpio_3
	j freertos_risc_v_trap_handler
	// 26 : fast interrupt - gpio_4
	j freertos_risc_v_trap_handler
	// 27 : fast interrupt - gpio_5
	j freertos_risc_v_trap_handler
	// 28 : fast interrupt - gpio_6
	j freertos_risc_v_trap_handler
	// 29 : fast interrupt - gpio_7
	j freertos_risc_v_trap_handler
	// 30 : fast interrupt - unmapped
	j __no_irq_handler
	// vendor interrupts: on Ibex interrupt id 31 is for non-maskable interrupts
	j __no_irq_handler
	// 64-32 : not connected on Ibex
	j freertos_risc_v_trap_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler
	j __no_irq_handler

/* this is fixed to 0x8000, used for PULP_SECURE=0. We redirect this entry to the
new vector table (which is at mtvec) */
/* .section .legacy_irq, "ax" */
/*	j vector_table */
/*	j __no_irq_handler */
/*	j __no_irq_handler */
/*	j __no_irq_handler */

.section .text.vecs
/* exception handling */
__no_irq_handler:
	la a0, no_exception_handler_msg
	jal ra, puts
	j __no_irq_handler

handle_unknown:
	la a0, unknown_msg
	jal ra, puts
	j end_handler

end_handler:
	csrr a0, mepc
	addi a0, a0, 4
	csrw mepc, a0
	mret
/* this interrupt can be generated for verification purposes, random or when the PC is equal to a given value*/
verification_irq_handler:
	mret

.section .rodata
illegal_insn_msg:
	.string "illegal instruction exception handler entered\n"
ecall_msg:
	.string "ecall exception handler entered\n"
ebreak_msg:
	.string "ebreak exception handler entered\n"
unknown_msg:
	.string "unknown exception handler entered\n"
no_exception_handler_msg:
	.string "no exception handler installed\n"
