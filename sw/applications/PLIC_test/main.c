#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "gpio.h"
#include "pad_control.h"
#include "pad_control_regs.h"  // Generated.
#include "x-heep.h"

#include "rv_plic_structs.h"


void test_init();
void test_setPrio();
void test_setEnable();
void test_getEnable();
void test_triggerType();
void test_complete();

// typedef struct app_rv_plic {
//     irq_sources_t irq_src;
// } app_rv_plic_t;

static app_rv_plic_t app_plic;

// irq_sources_t irq_src;


void delay(){
  for(int i=0; i<10000000; i++){
    asm("nop");
  }
}

// int8_t external_intr_flag = 0;

/*
Notes:
 - Ports 30 and 31 are connected in questasim testbench, but in the FPGA version they are connected to the EPFL programmer and should not be used
 - Connect a cable between the two pins for the applicatio to work
*/

#ifdef TARGET_PYNQ_Z2
    #define GPIO_TB_OUT 8
    #define GPIO_TB_IN  9
    #define GPIO_INTR  GPIO_INTR_9
    #pragma message ( "Connect a cable between GPIOs IN and OUT" )

#else

    #define GPIO_TB_OUT 30
    #define GPIO_TB_IN  31
    #define GPIO_INTR  GPIO_INTR_31

#endif


void handler_irq_gpio(uint32_t id) {
  printf("Interrupt serviced!");
  printf("Interrupt ID:\t%d\n", id);  
}

int main(){

  printf("\nStarting...\n");

  // TESTING INIT
  test_init();
  //

  // test_bitfield();

  pad_control_t pad_control;
  pad_control.base_addr = mmio_region_from_addr((uintptr_t)PAD_CONTROL_START_ADDRESS);

    // In case GPIOs 30 and 31 are used:
#if GPIO_TB_OUT == 31 || GPIO_TB_IN == 31
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_I2C_SCL_REG_OFFSET), 1);
#endif

#if GPIO_TB_OUT == 30|| GPIO_TB_IN == 30
    pad_control_set_mux(&pad_control, (ptrdiff_t)(PAD_CONTROL_PAD_MUX_I2C_SDA_REG_OFFSET), 1);
#endif
  gpio_params_t gpio_params;
  gpio_t gpio;
  gpio_result_t gpio_res;
  gpio_params.base_addr = mmio_region_from_addr((uintptr_t)GPIO_START_ADDRESS);
  gpio_res = gpio_init(gpio_params, &gpio);
  if (gpio_res != kGpioOk) {
      printf("Failed\n;");
      return -1;
  }

  irq_src = IRQ_GPIO_SRC;
  
  
  test_setPrio();

  test_setEnable();

  // test_getEnable();

  // test_triggerType();

  // plic_target_set_threshold(4);


  // Enable interrupt on processor side
  // Enable global interrupt for machine-level interrupts
  CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
  // Set mie.MEIE bit to one to enable machine-level external interrupts
  const uint32_t mask = 1 << 11;
  CSR_SET_BITS(CSR_REG_MIE, mask);

  gpio_res = gpio_output_set_enabled(&gpio, GPIO_TB_OUT, true);
  if (gpio_res != kGpioOk) {
      printf("Failed\n;");
      return -1;
  }
  gpio_write(&gpio, GPIO_TB_OUT, false);

  gpio_res = gpio_input_enabled(&gpio, GPIO_TB_IN, true);
  if (gpio_res != kGpioOk) {
      printf("Failed\n;");
      return -1;
  }


  gpio_res = gpio_irq_set_trigger(&gpio, GPIO_TB_IN, true, kGpioIrqTriggerEdgeRising);
  if (gpio_res != kGpioOk) {
      printf("Failed\n;");
      return -1;
  }
  

  plic_intr_flag = 0;
  // printf("Write 1 to GPIO 30 and wait for interrupt...\n");
  while(plic_intr_flag==0) {
      // gpio_write(&gpio, GPIO_TB_OUT, true);
      wait_for_interrupt();
  }
  printf("plic_intr_flag = %d\n", plic_intr_flag);
  
  printf("Done...\n");

  return 0;
}


void test_init(){
  // printf("ext:\t%d\n", external_intr_flag);
  plic_result_t r = plic_Init();
  // printf("ext:\t%d\n", external_intr_flag);

  if(r != kPlicOk) {
    printf("PLIC init failed!\n");
    return -1;
  }

  // To see all the priority registers
  // for(int i=0; i<64; i++){
  //   printf("PRIO%d:\t%d\n", i, *((&rv_plic_peri->PRIO0) + i));
  // }
}


void test_setPrio(){

  plic_result_t plic_res = plic_irq_set_priority(GPIO_INTR, 1);
  
  if(plic_res != kPlicOk){
    printf("Prio setting failed!\n");
  }

  plic_res = plic_irq_set_priority(12, 5);
  
  if(plic_res != kPlicOk){
    printf("Prio setting failed!\n");
  }


  // To see all the priority registers
  // for(int i=0; i<64; i++){
  //   printf("PRIO%d:\t%d\n", i, *((&rv_plic_peri->PRIO0) + i));
  // }
}


void test_setEnable(){
  plic_result_t plic_res = plic_irq_set_enabled(GPIO_INTR, kPlicToggleEnabled);
  if (plic_res != kPlicOk) {
      printf("Failed\n");
      return -1;
  }
  plic_toggle_t state;
  plic_irq_get_enabled(GPIO_INTR, &state);
}


void test_getEnable(){

  plic_result_t res;
  plic_toggle_t state;

  printf("--------------------------\n");
  plic_irq_get_enabled(GPIO_INTR, &state);
  printf("Enabled[%d]:\t%d\n\n", GPIO_INTR, state);
  plic_irq_get_enabled(45, &state);
  printf("Enabled[%d]:\t%d\n\n", 45, state);
  plic_irq_get_enabled(22, &state);
  printf("Enabled[%d]:\t%d\n\n", 22, state);
  printf("--------------------------\n");

}


void test_triggerType(){
  plic_result_t res;

  plic_irq_set_trigger(GPIO_INTR, kPlicIrqTriggerEdge);
}