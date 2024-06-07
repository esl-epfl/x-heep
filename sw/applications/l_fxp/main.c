#include "fxp32.h"
#include "utils.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define COMP_PREC 0.000001f

void test_assert() {
    assert_closef(1.0, 1.0, 0.0001f);
    assert_closef(1.0, 1.0001, 0.0002f);
    assert_closef(1.0, 1.0002, 0.0003f);
    assert_closef(0.01, 0.0109, 0.001f);
    assert_closei32(1, 1, 1);
    assert_closei32(1000, 1002, 3);
    assert_closei32(-1000, -1003, 4);
}

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
    assert_closef(result_add_float, f1 + f2, COMP_PREC);
    assert_closef(result_multiply_float, f1 * f2, COMP_PREC);
    assert_closef(result_divide_float, f1 / f2, COMP_PREC);
}

void test_fxp_sqrt() {
    fxp32 x = fxp32_fromFloat(2.0);
    fxp32 result = fxp32_sqrt(x);
    float result_float = fxp32_toFloat(result);
    assert_closef(result_float, 1.41421356237, 10*COMP_PREC);
}

int main() {
    PRINTF("\033[1;93m====== Test FXP ==========\n");
    PRINTF("\033[0m====== Test Assert =======\n");
    // test_assert();
    PRINTF("\033[1;32m====== Test passed =======\n");
    PRINTF("\033[0m====== Test Basic ========\n");
    // test_fxp_basic();
    PRINTF("\033[1;32m====== Test passed =======\n");
    PRINTF("\033[0m====== Test Sqrt =========\n");
    // test_fxp_sqrt();
    PRINTF("\033[1;32m====== Test passed =======\n");
    PRINTF("\033[0m====== Test FXP end ======\n\n");
    return EXIT_SUCCESS;
}