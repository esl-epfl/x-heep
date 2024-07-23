#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "csr.h"
#include "x-heep.h"
#include "timer_sdk.h"

#define FS_INITIAL 0x01

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

void __attribute__((aligned(4), interrupt)) handler_irq_timer(void) {
    timer_arm_stop();
    timer_irq_clear();
    return;   
}

extern float add_asm_function(float* a, float* b);
extern float add_asm_function2(float* a, float* b);

int main() {

    //enable FP operations
    CSR_SET_BITS(CSR_REG_MSTATUS, (FS_INITIAL << 13));
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 7;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    timer_init();
    timer_irq_enable();
    timer_arm_start(4);

    float num1 = 10.0;
    float num2 = 20.0;
    float sum = add_asm_function2(&num1, &num2);
//    int mul = mul_by_const_asm_function(num2);

//    PRINTF("%d+%d=%d\n", num1, num2, sum);
//    PRINTF("%d*%d=%d\n", num2, MULTIPLY_CONSTANT, mul );
    
    return (sum == num1+num2) ? EXIT_SUCCESS : EXIT_FAILURE;   
}