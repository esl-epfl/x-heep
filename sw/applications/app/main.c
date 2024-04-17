#include "fxp32.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define COMP_PREC 0.000001f

#define my_assert(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed: %s\n"); \
            fprintf(stderr, "File: %s\n", __FILE__); \
            fprintf(stderr, "Line: %d\n", __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

void test_fxp_basic() {
    float f1 = 3.14159286537;
    float f2 = 2.71828723519;

    // Convert floats to fxp32
    fxp32 x = fxp32_fromFloat(f1);
    fxp32 y = fxp32_fromFloat(f2);

    // Perform addition and multiplication
    fxp32 result_add = x + y;
    fxp32 result_multiply = fxp32_mul(x, y);
    fxp32 result_divide = fxp32_div(x, y);

    // Convert fxp32 back to floats
    float result_add_float = fxp32_toFloat(result_add);
    float result_multiply_float = fxp32_toFloat(result_multiply);
    float result_divide_float = fxp32_toFloat(result_divide);

    // Print results
    my_assert(fxp32_close(result_add_float, f1 + f2, COMP_PREC));
    my_assert(fxp32_close(result_multiply_float, f1 * f2, COMP_PREC));
    my_assert(fxp32_close(result_divide_float, f1 / f2, COMP_PREC));
}

void test_fxp_sqrt() {
    fxp32 x = fxp32_fromFloat(2.0);
    fxp32 result = fxp32_sqrt(x);
    float result_float = fxp32_toFloat(result);
    my_assert(fxp32_close(result_float, 1.4142135f, COMP_PREC));
}

int main() {
    printf("\033[1;93m====== Test FXP ==========\n");
    printf("\033[0m====== Test Basic ========\n");
    test_fxp_basic();
    test_fxp_sqrt();
    printf("\033[1;32m====== Test passed =======\n");
    printf("\033[0m====== Test Basic end ====\n\n");
    return EXIT_SUCCESS;
}
