/*
                              *******************
******************************* C SOURCE FILE *******************************
**                            *******************                          **
**                                                                         **
** project  : blinky_freertos                                              **
** filename : main.c                                                       **
** version  : 1                                                            **
** date     : 20/12/2022                                                   **
**                                                                         **
*****************************************************************************
**                                                                         **
** Copyright (C) 2019 Amazon.com, Inc. or its affiliates.                  **
** Copyright (C) 2020 ETH Zurich										   **
** Copyright (C) 2022 EPFL                                                 **
**                                                                         **
*****************************************************************************
*/
/*
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

/***************************************************************************/
/***************************************************************************/

/**
* @file   main.c
* @date   20/12/2022
* @brief  This is the application example of FreeRTOS for x-heep
*
* main() creates one queue, and two tasks.  It then starts the
* scheduler.
*
* The Queue Send Task:
* The queue send task is implemented by the prvQueueSendTask() function in
* this file.  prvQueueSendTask() sits in a loop that causes it to repeatedly
* block for 1000 milliseconds, before sending the value 100 to the queue that
* was created within main_blinky().  Once the value is sent, the task loops
* back around to block for another 1000 milliseconds...and so on.
*
* The Queue Receive Task:
* The queue receive task is implemented by the prvQueueReceiveTask() function
* in this file.  prvQueueReceiveTask() sits in a loop where it repeatedly
* blocks on attempts to read data from the queue that was created within
* main_blinky().  When data is received, the task checks the value of the
* data, and if the value equals the expected 100, writes 'Blink' to the UART
* (the UART is used in place of the LED to allow easy execution in QEMU).  The
* 'block time' parameter passed to the queue receive function specifies that
* the task should be held in the Blocked state indefinitely to wait for data to
* be available on the queue.  The queue receive task will only leave the
* Blocked state when the queue send task writes to the queue.  As the queue
* send task writes to the queue every 1000 milliseconds, the queue receive
* task leaves the Blocked state every 1000 milliseconds, and therefore toggles
* the LED every 200 milliseconds.
* Please, see FreeRTOSConfig.h to check the configuration
*/

#define _MAIN_C_SRC

/****************************************************************************/
/**                                                                        **/
/*                             MODULES USED                                 */
/**                                                                        **/
/****************************************************************************/

/* FreeRTOS kernel includes */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

/* c stdlib */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

/* X-HEEP related includes */
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "rv_timer.h"
#include "soc_ctrl.h"
#include "gpio.h"
#include "x-heep.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

/* Priorities used by the tasks. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the pdMS_TO_TICKS() macro. */
#ifdef TARGET_IS_FPGA
#define mainQUEUE_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 200 )
#else
#define mainQUEUE_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 3 )
#endif

/* The maximum number items the queue can hold.  The priority of the receiving
task is above the priority of the sending task, so the receiving task will
preempt the sending task and remove the queue items each time the sending task
writes to the queue.  Therefore the queue will never have more than one item in
it at any time, and even with a queue length of 1, the sending task will never
find the queue full. */
#define mainQUEUE_LENGTH					( 1 )

/* Const value to play with TICK counts within the APP */
#define TICK_COUNT                          ( 50 )

/* Set mainCREATE_SIMPLE_BLINKY_DEMO_ONLY to one to run the simple blinky demo,
or 0 to run the more comprehensive test and demo application. */
#define mainCREATE_SIMPLE_BLINKY_DEMO_ONLY	1

/*
 * main_blinky() is used when mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 1.
 * main_full() is used when mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 0.
 */
#if mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 1
	//extern void main_blinky( void );
#else
#error "Full demo is not available in this project. Check demos/ directory."
#endif /* #if mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 1 */


#ifdef TARGET_IS_FPGA
    #define GPIO_LD5_R  15
    #define GPIO_LD5_B  16
	#define GPIO_LD5_G  17
    #pragma message ( "Executing FreeRTOS using X-HEEP and Pynq-z2" )
#else
    #define GPIO_LD5_R  29
    #define GPIO_LD5_B  30
	#define GPIO_LD5_G  31
#endif

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
/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );

/* Prototypes for the standard FreeRTOS callback/hook functions implemented
within this file.  See https://www.freertos.org/a00016.html */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vApplicationTickHook( void );

/* Prepare haredware to run the demo. */
static void SetupHardware( void );

/* Send a messaage to the UART initialised in prvSetupHardware. */
void vSendString( const char * const pcString );

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

/* Allocate heap to special section. Note that we have no references in the
 * whole program to this variable (since its just here to allocate space in the
 * section for our heap), so when using LTO it will be removed. We force it to
 * stay with the "used" attribute
 */
__attribute__((section(".heap"), used)) uint8_t ucHeap[configTOTAL_HEAP_SIZE];

/* Timer 0 AO Domain as Tick Counter */
static rv_timer_t timer_0_1;

/* In case of playing with the Tick Frequency, set it to the desired value
 * E.g.: REFERENCE_CLOCK_Hz/configTICK_RATE_HZ --> 100kHz
 */
static const uint64_t kTickFreqHz = (REFERENCE_CLOCK_Hz/configTICK_RATE_HZ); 

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

/* Temporal flag to store ISR status */
int8_t intr_flag = 0;

/* Temporal counter to store blinking status */
int8_t intr_blink = 0;

/****************************************************************************/
/**                                                                        **/
/*                           EXPORTED FUNCTIONS                             */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

/**
 * Board init code. Always call this before anything else.
 */
void system_init(void)
{

    // Get current Frequency
    soc_ctrl_t soc_ctrl;
    soc_ctrl.base_addr = mmio_region_from_addr((uintptr_t)SOC_CTRL_START_ADDRESS);
    uint32_t freq_hz = soc_ctrl_get_frequency(&soc_ctrl);


    gpio_result_t gpio_res;
    gpio_cfg_t pin_cfg = {
        .pin= GPIO_LD5_R, 
        .mode= GpioModeOutPushPull
    };
    gpio_res = gpio_config(pin_cfg);
    pin_cfg.pin = GPIO_LD5_B;
    gpio_res |= gpio_config(pin_cfg);
    pin_cfg.pin = GPIO_LD5_G;
	gpio_res |= gpio_config(pin_cfg);
    if (gpio_res != GpioOk) printf("Failed\n;");
    
    gpio_write(GPIO_LD5_R, false);
    gpio_write(GPIO_LD5_B, false);
    gpio_write(GPIO_LD5_G, false);

    // Setup rv_timer_0_1
    mmio_region_t timer_0_1_reg = mmio_region_from_addr(RV_TIMER_AO_START_ADDRESS);
    rv_timer_init(timer_0_1_reg, (rv_timer_config_t){.hart_count = 2, .comparator_count = 1}, &timer_0_1);
	
	// Just in case you are playing with Tick freq.
    //rv_timer_approximate_tick_params(freq_hz, kTickFreqHz, &tick_params);

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

    // Enable timer interrupt
    uint32_t mask = 1 << 7;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    configASSERT(rv_timer_irq_enable(&timer_0_1, 0, 0, kRvTimerEnabled) == kRvTimerOk);
	configASSERT(rv_timer_counter_set_enabled(&timer_0_1, 0, kRvTimerEnabled) == kRvTimerOk);
}

/*****************************************************************************
*****************************************************************************/

/**
 * Use this function in case you want an ad-hoc MTIME logic.
 */
void handler_irq_timer(void)
{
	configASSERT(rv_timer_reset(&timer_0_1)==kRvTimerOk);
    configASSERT(rv_timer_irq_enable(&timer_0_1, 0, 0, kRvTimerEnabled) == kRvTimerOk);
	configASSERT(rv_timer_arm(&timer_0_1, 0, 0, TICK_COUNT) == kRvTimerOk);
	
    if (xTaskIncrementTick() != 0) {
		vTaskSwitchContext();
		intr_flag = 1;
	}
	
	uint32_t out = 0;
	out = xTaskGetTickCountFromISR();
	printf( "I %d\r\n",out);
	
	configASSERT(rv_timer_counter_set_enabled(&timer_0_1, 0, kRvTimerEnabled) == kRvTimerOk);
}

/*****************************************************************************
*****************************************************************************/

void main_blinky( void )
{
	//printf( "Calling %s\n\r", __func__);
	/* Create the queue. */
	xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( uint32_t ) );

	if( xQueue != NULL )
	{

	    //printf( "Creating two tasks (xTaskCreate)\n\r" );
		/* Start the two tasks as described in the comments at the top of this
		file. */
		xTaskCreate( prvQueueReceiveTask,				/* The function that implements the task. */
					"Rx", 								/* The text name assigned to the task - for debug only as it is not used by the kernel. */
					configMINIMAL_STACK_SIZE * 2U, 		/* The size of the stack to allocate to the task. */
					NULL, 								/* The parameter passed to the task - not used in this case. */
					mainQUEUE_RECEIVE_TASK_PRIORITY, 	/* The priority assigned to the task. */
					NULL );								/* The task handle is not required, so NULL is passed. */
		//printf( "created prvQueueReceveiTask\n\r" );

		xTaskCreate( prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE * 2U, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );
		//printf( "created prvQueueSendTask\n\r" );
		printf( "SS\n\r" );

		/* Start the tasks and timer running. */
		vTaskStartScheduler();
		printf( "SS\n\r" );
	}

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the Idle and/or
	timer tasks to be created.  See the memory management section on the
	FreeRTOS web site for more details on the FreeRTOS heap
	http://www.freertos.org/a00111.html. */
	for( ;; );
}

/*****************************************************************************
*****************************************************************************/

static void prvQueueSendTask( void *pvParameters )
{
  TickType_t xNextWakeTime;
  const unsigned long ulValueToSend = 100UL;
  BaseType_t xReturned;

  //printf( "%s\n\r", __func__ );
  /* Remove compiler warning about unused parameter. */
  ( void ) pvParameters;
  
  /* Initialise xNextWakeTime - this only needs to be done once. */
  xNextWakeTime = xTaskGetTickCount();
  
  for( ;; )
  {
    vSendString( "T1\r\n" );
  	
    /* Place this task in the blocked state until it is time to run again. */
    vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );
    //vTaskDelay(1);
    
    /* Send to the queue - causing the queue receive task to unblock and
    toggle the LED.  0 is used as the block time so the sending operation
    will not block - it shouldn't need to block as the queue should always
    be empty at this point in the code. */
    xReturned = xQueueSend( xQueue, &ulValueToSend, 0U );
    configASSERT( xReturned == pdPASS );
  }
}

/*****************************************************************************
*****************************************************************************/

static void prvQueueReceiveTask( void *pvParameters )
{
  unsigned long ulReceivedValue = 0;
  const unsigned long ulExpectedValue = 100UL;
  const char * const pcPassMessage = "BL\r\n";
  const char * const pcFailMessage = "!Q\r\n";
  extern void vSendString( const char * const pcString );
  extern void vToggleLED( void );

  //printf( "%s\n\r", __func__ );
  /* Remove compiler warning about unused parameter. */
  ( void ) pvParameters;
  
  for( ;; )
  {
    vSendString( "T2\r\n" );
    
    /* Wait until something arrives in the queue - this task will block
    indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
    FreeRTOSConfig.h. */
    xQueueReceive( xQueue, &ulReceivedValue, ( TickType_t ) portMAX_DELAY );
    
    /*  To get here something must have been received from the queue, but
    is it the expected value?  If it is, toggle the LED. */
    if( ulReceivedValue == ulExpectedValue )
    {
      vSendString( pcPassMessage );
      vToggleLED();
      ulReceivedValue = 0U;
    }
    else
    {
      vSendString( pcFailMessage );
    }
  }
}

/*****************************************************************************
*****************************************************************************/

static void SetupHardware( void )
{
	/* Init board hardware. */
	system_init();
}

/*****************************************************************************
*****************************************************************************/

void vToggleLED( void )
{
  if (intr_blink == 0)
  { 
	gpio_write(GPIO_LD5_R, true);
	gpio_write(GPIO_LD5_B, false);
	gpio_write(GPIO_LD5_G, false);
	intr_blink++;
  }
  else if (intr_blink == 1)
  { 
	gpio_write(GPIO_LD5_R, false);
	gpio_write(GPIO_LD5_B, true);
	gpio_write(GPIO_LD5_G, false);
	intr_blink++;
  }
  else if (intr_blink == 2)
  { 
	gpio_write(GPIO_LD5_R, false);
	gpio_write(GPIO_LD5_B, false);
	gpio_write(GPIO_LD5_G, true);
	intr_blink++;
  }
  else
  { 
	gpio_write(GPIO_LD5_R, false);
	gpio_write(GPIO_LD5_B, false);
	gpio_write(GPIO_LD5_G, false);
	intr_blink = 0;
  }

}

/*****************************************************************************
*****************************************************************************/

void vSendString( const char * const pcString )
{
	taskENTER_CRITICAL();
	/* TODO: UART dumping */
	printf( "%s", pcString );
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	printf( "error: application malloc failed\n\r" );
	__asm volatile( "ebreak" );
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
	taskENTER_CRITICAL();
	printf("I\r\n");
	taskEXIT_CRITICAL();
	
}
/*-----------------------------------------------------------*/

void freertos_risc_v_application_exception_handler(uint32_t mcause)
{
	printf("App mcause:%d\r\n", mcause);
}

/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	__asm volatile( "ebreak" );
	for( ;; );
}

/*****************************************************************************
*****************************************************************************/

void vApplicationTickHook( void )
{
	/* The tests in the full demo expect some interaction with interrupts. */
	#if( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY != 1 )
	{
		extern void vFullDemoTickHook( void );
		vFullDemoTickHook();
	}
	#endif
}

/*****************************************************************************
*****************************************************************************/

int main( void )
{
	SetupHardware();
	
	//printf("Going into main_blinky\n\r");

	/* The mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting is described at the top
	of this file. */
	#if( mainCREATE_SIMPLE_BLINKY_DEMO_ONLY == 1 )
	{
		main_blinky();
	}
	#else
	{
	  #error "Full demo is not available in this project. 
	}
	#endif
	
	//should never reach this point
	for(;;);
}

/****************************************************************************/
/**                                                                        **/
/*                                 EOF                                      */
/**                                                                        **/
/****************************************************************************/
