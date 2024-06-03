#include <stdio.h>
#include "constants.h"
#include "x-heep.h"

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

// Declaration of the assembly function for the little Juan
extern int add_asm_function(int a, int b);
extern int mul_by_const_asm_function( int a);

int main() {
    int num1 = 10;
    int num2 = 20;
    int sum = add_asm_function(num1, num2);
    int mul = mul_by_const_asm_function(num2);

    PRINTF("%d+%d=%d\n", num1, num2, sum);
    PRINTF("%d*%d=%d\n", num2, MULTIPLY_CONSTANT, mul );
    
    return EXIT_SUCCESS;   
}