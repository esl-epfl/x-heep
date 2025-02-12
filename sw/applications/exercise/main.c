

#include <stdio.h>
#include <stdlib.h>

static inline int add_two_numbers(int a, int b) {
    int result;
    __asm__ volatile (
        ".insn r, 0x5B, 0x6, %[rd], %[rs1], %[rs2]\n\t"
        : [rd] "=r" (result)
        : [rs1] "r" (a), [rs2] "r" (b)
    );
    return result;
}



int main(int argc, char *argv[])
{
    printf("hello world!\n");

    int result = add_two_numbers(2, 3);

    printf("result: %d\n", result);

    return EXIT_SUCCESS;
}
