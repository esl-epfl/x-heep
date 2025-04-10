#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "x-heep.h"

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA 1
#define PRINTF_IN_SIM 0

#if TARGET_SIM && PRINTF_IN_SIM
#ifndef TEST_MODE
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define PRINTF_TEST(...)
#else
#define PRINTF(...)
#define PRINTF_TEST(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif
#elif PRINTF_IN_FPGA && !TARGET_SIM
#ifndef TEST_MODE
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define PRINTF_TEST(...)
#else
#define PRINTF(...)
#define PRINTF_TEST(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif
#else
#define PRINTF(...)
#define PRINTF_TEST(...)
#endif

extern int add_asm_function(int a, int b);
extern int mul_by_const_asm_function(int a);

int main()
{
    int num1 = 10;
    int num2 = 20;
    int sum = add_asm_function(num1, num2);
    int mul = mul_by_const_asm_function(num2);

    PRINTF("%d+%d=%d\n", num1, num2, sum);
    PRINTF("%d*%d=%d\n", num2, MULTIPLY_CONSTANT, mul);

    int return_value = !((sum == num1 + num2) && (mul == num2 * MULTIPLY_CONSTANT));

    PRINTF_TEST("0:%d&\n", return_value);

    return return_value ? EXIT_FAILURE : EXIT_SUCCESS;
}