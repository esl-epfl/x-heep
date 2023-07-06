// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrixAdd32.h"
#include "csr.h"
#include "x-heep.h"

#define FS_INITIAL 0x01

/* Change this value to 0 to disable prints for FPGA and enable them for simulation. */
#define DEFAULT_PRINTF_BEHAVIOR 1

/* By default, printfs are activated for FPGA and disabled for simulation. */
#ifdef TARGET_PYNQ_Z2 
    #define ENABLE_PRINTF DEFAULT_PRINTF_BEHAVIOR
#else 
    #define ENABLE_PRINTF !DEFAULT_PRINTF_BEHAVIOR
#endif

#if ENABLE_PRINTF
  #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
  #define PRINTF(...)
#endif 

void __attribute__ ((noinline)) matrixAdd(float * A, float * B, float * C, int N, int M);
uint32_t check_results(float *  C, int N, int M);

float m_c[HEIGHT*WIDTH];

void swap(char *a, char *b)
{
    char temp = *a;
    *a = *b;
    *b = temp;
}

// A function to print a floating point number using putchar
void putfloat(float x, int p)
{
    // Check if x is negative
    if (x < 0)
    {
        // Print a minus sign
        putchar('-');
        // Make x positive
        x = -x;
    }

    // Convert the integer part of x into a string of digits
    long i = (long)x; // Get the integer part
    char int_str[20]; // An array to store the digits
    int len = 0; // The length of the string
    do
    {
        // Get the last digit and store it in the array
        int_str[len] = '0' + i % 10;
        len++;
        // Remove the last digit from i
        i /= 10;
    } while (i > 0);

    // Reverse the string of digits
    for (int j = 0; j < len / 2; j++)
    {
        // Swap the elements at both ends
        swap(&int_str[j], &int_str[len - 1 - j]);
    }

    // Print the string of digits
    for (int j = 0; j < len; j++)
    {
        putchar(int_str[j]);
    }

    // Print a decimal point
    putchar('.');

    // Convert the fractional part of x into a string of digits
    float f = x - (long)x; // Get the fractional part
    char frac_str[20]; // An array to store the digits
    len = 0; // The length of the string
    while (p--)
    {
        // Get the first digit after the decimal point and store it in the array
        f = (f - (long)f) * 10;
        frac_str[len] = '0' + (long)f;
        len++;
        // Round up if necessary
        if (fabs(f - (long)f) >= 0.5f)
        {
            frac_str[len - 1]++;
        }
    }

    // Print the string of digits
    for (int j = 0; j < len; j++)
    {
        putchar(frac_str[j]);
    }
}

void __attribute__ ((noinline)) printMatrix(float *  C, int N, int M)
{
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < M; j++) {
            putfloat(C[i*N+j], 2);
            if( j != M -1)
                printf(", ");
        }
        printf("\n");
    }
}

int main()
{
    int N = WIDTH;
    int M = HEIGHT;
    uint32_t errors = 0;
    unsigned int instr, cycles, ldstall, jrstall, imstall;

    //enable FP operations
    CSR_SET_BITS(CSR_REG_MSTATUS, (FS_INITIAL << 13));

    CSR_WRITE(CSR_REG_MCYCLE, 0);

    //execute the kernel
    matrixAdd(m_a, m_b, m_c, N, M);

    CSR_READ(CSR_REG_MCYCLE, &cycles) ;

    //stop the HW counter used for monitoring

    errors = check_results(m_c, N, M);

    PRINTF("program finished with %d errors and %d cycles\n\r", errors, cycles);

#ifdef ENABLE_PRINTF
    printMatrix(m_c,N,M);
#endif


    return errors;
}

void __attribute__ ((noinline)) matrixAdd(float *  A, float *  B, float *  C, int N, int M)
{
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < M; j++) {
            C[i*N+j] = A[i*WIDTH+j] + B[i*WIDTH+j];
        }
    }
}

uint32_t check_results(float * C, int N, int M)
{
    // check
    int i, j;
    uint32_t err = 0;

    for(i = 0; i < N; i++) {
        for(j = 0; j < M; j++) {
            if(C[i*N+j] != m_exp[i*WIDTH+j]) {
                err++;
                PRINTF("Error at index %d, %d, expected %d, got %d\n\r", i, j, m_exp[i*WIDTH+j], C[i*N+j]);
            }
        }
    }

    return err;
}
