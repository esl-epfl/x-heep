/*
 * FreeRTOS Kernel V10.3.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (C) 2020 ETH Zurich
 * Copyright (C) 2022 EPFL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "x-heep.h" /* figure out our FLL/clock setup */
#include "core_v_mini_mcu.h"
#include "rv_timer_regs.h"


#define DEFAULT_SYSTEM_CLOCK           REFERENCE_CLOCK_Hz /* Default System clock value */

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#include <stddef.h>
#ifdef __PULP_USE_LIBC
	#include <assert.h>
#endif

/* Ensure stdint is only used by the compiler, and not the assembler. */
#if defined( __GNUC__ )
    #include <stdint.h>
#endif

/* See https://www.freertos.org/Using-FreeRTOS-on-RISC-V.html */
//#define configCLINT_BASE_ADDRESS		 0 /* There is no CLINT so the base address must be set to 0. */
#define configMTIME_BASE_ADDRESS                    (RV_TIMER_AO_START_ADDRESS + RV_TIMER_TIMER_V_LOWER0_REG_OFFSET)
#define configMTIMECMP_BASE_ADDRESS                 (RV_TIMER_AO_START_ADDRESS + RV_TIMER_COMPARE_LOWER0_0_REG_OFFSET)


#define configUSE_PREEMPTION	 1
#define configUSE_IDLE_HOOK	     1
#define configUSE_TICK_HOOK	     1
#define configCPU_CLOCK_HZ	 DEFAULT_SYSTEM_CLOCK
#define configTICK_RATE_HZ	 ((TickType_t)1000)
#define configMAX_PRIORITIES	 (5)
/* Can be as low as 60 but some of the demo tasks that use this constant require it to be higher. */
#define configMINIMAL_STACK_SIZE ((unsigned short)80)
/* we want to put the heap into special section */
#define configAPPLICATION_ALLOCATED_HEAP 1
#define configTOTAL_HEAP_SIZE		 ((size_t)(6 * 1024))
#define configMAX_TASK_NAME_LEN		 (12)
#define configUSE_TRACE_FACILITY	 0 /* TODO: 0 */
#define configUSE_16_BIT_TICKS		 0
#define configIDLE_SHOULD_YIELD		 0
#define configUSE_MUTEXES		 1
#define configQUEUE_REGISTRY_SIZE	 8
#define configCHECK_FOR_STACK_OVERFLOW	 2
#define configUSE_RECURSIVE_MUTEXES	 1
#define configUSE_MALLOC_FAILED_HOOK	 1
#define configUSE_APPLICATION_TASK_TAG	 0
#define configUSE_COUNTING_SEMAPHORES	 1
#define configGENERATE_RUN_TIME_STATS	 0
#define configUSE_QUEUE_SETS                        1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES       3

/* newlib reentrancy */
//#define configUSE_NEWLIB_REENTRANT 1
/* Co-routine definitions. */
//#define configUSE_CO_ROUTINES		0
//#define configMAX_CO_ROUTINE_PRIORITIES (2)				   	   

/* Software timer definitions. */
#define configUSE_TIMERS                            1
#define configTIMER_TASK_PRIORITY                   ( configMAX_PRIORITIES - 3 )
#define configTIMER_QUEUE_LENGTH                    20
#define configTIMER_TASK_STACK_DEPTH                ( configMINIMAL_STACK_SIZE *2)

/* Task priorities.  Allow these to be overridden. */
#ifndef uartPRIMARY_PRIORITY
    #define uartPRIMARY_PRIORITY                    ( configMAX_PRIORITIES - 3 )
#endif

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet                    1
#define INCLUDE_uxTaskPriorityGet                   1
#define INCLUDE_vTaskDelete                         1
#define INCLUDE_vTaskCleanUpResources               1
#define INCLUDE_vTaskSuspend                        1
#define INCLUDE_vTaskDelayUntil                     1
#define INCLUDE_vTaskDelay                          1
#define INCLUDE_eTaskGetState                       1
#define INCLUDE_xTimerPendFunctionCall              1
#define INCLUDE_xTaskAbortDelay                     1
#define INCLUDE_xTaskGetHandle                      1
#define INCLUDE_xSemaphoreGetMutexHolder            1

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#ifdef __PULP_USE_LIBC
	#define configASSERT( x ) assert ( x )
#else
	#define configASSERT( x ) do { if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); } } while ( 0 )
#endif


#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configKERNEL_INTERRUPT_PRIORITY 7


#endif /* FREERTOS_CONFIG_H */
