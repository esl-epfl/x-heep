#include "utils.h"
#include "fxp32.h"
#include "sylt-fft/fft.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "testdata_fft.h"

#define COMP_PREC 0.000001f


fft_complex_t arr[LEN];
int len = LEN;

void fill_arr_real(fft_complex_t* arr, int n, float* data_r) {
    for (int i = 0; i < n; i++) {
        arr[i].r = fxp32_fromFloat(data_r[i]);
        arr[i].i = 0;
    }
}

void print_arr(fft_complex_t* arr, int n) {
    for (int i = 0; i < n; i++) {
        PRINTF("i = %d, r = %f, i = %f\n", i, fxp32_toFloat(arr[i].r), fxp32_toFloat(arr[i].i));
    }
}

void analyze_arr(fft_complex_t* arr, int n) {
    int nonzeroCount = 0;
    PRINTF("Nonzero indexes: ");
    for (int i = 0; i < n; i++) {
        if (fxp32_toFloat(arr[i].r) > COMP_PREC || fxp32_toFloat(arr[i].r) < -COMP_PREC || 
            fxp32_toFloat(arr[i].i) > COMP_PREC || fxp32_toFloat(arr[i].i) < -COMP_PREC) {
            nonzeroCount++;
            PRINTF("\n%d (r = %d, i = %d fxp32 values)", i, arr[i].r, arr[i].i);
        }
    }
    PRINTF("\n");
    PRINTF("Nonzero count: %d\n", nonzeroCount);
}

void test_fft_a1() {
    fill_arr_real(arr, len, a1);
    fft_fft(arr, bits);
    analyze_arr(arr, len);
}

void test_fft_a2() {
    fill_arr_real(arr, len, a2);
    fft_fft(arr, bits);
    analyze_arr(arr, len);
}

void test_fft_a3() {
    fill_arr_real(arr, len, a3);
    fft_fft(arr, bits);
    analyze_arr(arr, len);
}

// TODO: to have exactly the python result, we must multiply the result by len
void test_fft_a4() {
    fill_arr_real(arr, len, a4);
    fft_fft(arr, bits);
    for (int i = 0; i < len; i++) {
        assert_closef_si(fxp32_toFloat(arr[i].r), A4_real[i], COMP_PREC, i);
        assert_closef_si(fxp32_toFloat(arr[i].i), A4_imag[i], COMP_PREC, i);
    }
}

int main() {
    PRINTF("====== Test FFT ==========\n");
    PRINTF("====== Test A1 ===========\n");
    test_fft_a1();
    PRINTF("====== Test passed =======\n");
    PRINTF("====== Test A2 ===========\n");
    test_fft_a2();
    PRINTF("====== Test passed =======\n");
    PRINTF("====== Test A3 ===========\n");
    test_fft_a3();
    PRINTF("====== Test passed =======\n");
    PRINTF("====== Test A3 ===========\n");
    test_fft_a4();
    PRINTF("====== Test passed =======\n");
    PRINTF("====== Test FFT end ======\n\n");
    return EXIT_SUCCESS;
}