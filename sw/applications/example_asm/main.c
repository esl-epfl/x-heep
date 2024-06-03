#include <stdio.h>
#include "constants.h"

// Declaration of the assembly function for the little Juan
extern int add_asm_function(int a, int b);
extern int multiply_by_const( int a);

int main() {
    int num1 = 10;
    int num2 = 20;
    int sum = add_asm_function(num1, num2);
    int mul = multiply_by_const(num2);

    printf("%d+%d=%d\n", num1, num2, sum);
    printf("%d*%d=%d\n", num2, MULTIPLY_CONSTANT, mul );
    return 0;   
}