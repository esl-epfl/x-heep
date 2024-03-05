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

typedef enum
{
    DEC1 = 10,
    DEC2 = 100,
    DEC3 = 1000,
    DEC4 = 10000,
    DEC5 = 100000,
    DEC6 = 1000000,

} tPrecision;

void __attribute__ ((noinline)) matrixAdd(float * A, float * B, float * C, int N, int M);
uint32_t check_results(float *  C, int N, int M);

float m_c[HEIGHT*WIDTH];

void swap(char *a, char *b)
{
    char temp = *a;
    *a = *b;
    *b = temp;
}

// A function to print a long number using putchar recursively
void putLong(long x)
{
    if(x < 0)
    {
        putchar('-');
        x = -x;
    }
    if (x >= 10) 
    {
        putLong(x / 10);
    }
    putchar(x % 10+'0');
}

// A function to print a floating point number
void putfloat(float x, tPrecision p)
{
    // print integer part
    long i = (long)x;
    putLong( i ) ;

    // print decimal
    putchar('.') ;

    // print p zeros directly if x * p < 0.5f 
    if ( ( x - i ) * p < 0.5f) {
        long dec_zero = p;
        while(dec_zero > 1) {
            putchar('0');
            dec_zero /= 10;
        }
        return;
    }

    // print zero after the decimal point
    x = x - i;
    long scale = 1;
    while(scale < p) {
        if ((x * scale * 10 > 0.5f))
            break;
        putchar('0');
        scale *= 10;
    }

    // scale up decimal part to print it using putlong
    x = x * p;
    i = fabs((long)x);

    // round up if necessary
    if( fabs(x) - i >= 0.5f )
    {
        i++ ;
    }

    // print decimal part
    putLong( i ) ;
}

void __attribute__ ((noinline)) printMatrix(float *  C, int N, int M)
{
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < M; j++) {
            putfloat(C[i*N+j], DEC3);
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
    unsigned int instr, cycles;

    //enable FP operations
    CSR_SET_BITS(CSR_REG_MSTATUS, (FS_INITIAL << 13));

    //enable mcycle csr
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

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
