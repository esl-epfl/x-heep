#include <stdio.h>

// Declaration of the assembly function for the little Juan
extern int add(int a, int b);

int main() {
    int num1 = 10;
    int num2 = 20;
    int result = add(num1, num2);

    printf("%d+%d=%d\n", num1, num2, result);
    return 0;
}