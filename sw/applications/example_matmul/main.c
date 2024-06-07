// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "matrixMul8.h"
#include "x-heep.h"

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

void __attribute__ ((noinline)) matrixMul8_blocksize(int8_t *  A, int8_t *  B, int32_t *  C, int N);

void __attribute__ ((noinline)) matrixMul8_tiled(int8_t *  A, int8_t *  B, int32_t *  C, int N);

uint32_t check_results(int32_t * C, int N);

int32_t m_c[SIZE*SIZE];

#define BLOCK_SIZE 4

// Define a macro for accessing matrix elements
#define A(i,j) &A[i*SIZE+j]
#define B(i,j) &B[i*SIZE+j]
#define C(i,j) &C[i*SIZE+j]

#define HIGHEST_PERF

int main()
{

    uint32_t errors = 0;
    unsigned int instr, cycles;

    for(int i =0;i<SIZE;i++) {
        for(int j =0;j<SIZE;j++) {
            m_c[i*SIZE+j] = 0;
        }
    }

    //enable mcycle csr
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

    CSR_WRITE(CSR_REG_MCYCLE, 0);

#ifdef HIGHEST_PERF
    matrixMul8_blocksize(m_a, m_b, m_c, SIZE);
#else
    //execute the kernel
    matrixMul8_tiled(m_a, m_b, m_c, SIZE);
#endif

    CSR_READ(CSR_REG_MCYCLE, &cycles);

    errors = check_results(m_c, SIZE);

    PRINTF("program finished with %d errors and %d cycles\n\r", errors, cycles);
    return errors;
}

void __attribute__ ((noinline)) matrixMul8_blocksize(int8_t *  A, int8_t *  B, int32_t *  C, int N)
{

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            int32_t acc = 0;
            for(int k = 0; k < N; k++) {
                acc+= A[i*SIZE+k] * B[k*SIZE+j];
            }
            C[i*SIZE+j] += acc;
        }
    }

}


// Define a recursive function that multiplies two matrices using the tiled algorithm
void __attribute__ ((noinline)) matrixMul8_tiled(int8_t* A, int8_t* B, int32_t* C, int N) {
    // use the elementary function
    if (N == BLOCK_SIZE) {
        matrixMul8_blocksize(A, B, C, N);
    }
    //split the matrices into four blocks each
    else {
        N = N >> 1; // Half the size
        // Multiply the blocks and add them to the corresponding blocks of C
        matrixMul8_tiled(A(0, 0), B(0, 0), C(0, 0), N); // C_00 += A_00 * B_00
        matrixMul8_tiled(A(0, N), B(N, 0), C(0, 0), N); // C_00 += A_01 * B_10
        matrixMul8_tiled(A(0, 0), B(0, N), C(0, N), N); // C_01 += A_00 * B_01
        matrixMul8_tiled(A(0, N), B(N, N), C(0, N), N); // C_01 += A_01 * B_11
        matrixMul8_tiled(A(N, 0), B(0, 0), C(N, 0), N); // C_10 += A_10 * B_00
        matrixMul8_tiled(A(N, N), B(N, 0), C(N, 0), N); // C_10 += A_11 * B_10
        matrixMul8_tiled(A(N, 0), B(0, N), C(N, N), N); // C_11 += A_10 * B_01
        matrixMul8_tiled(A(N, N), B(N, N), C(N, N), N); // C_11 += A_11 * B_11
    }
}


uint32_t check_results(int32_t * C, int N)
{
    // check
    int i, j;
    uint32_t err = 0;

    for(i = 0; i < N; i++) {
        for(j = 0; j < N; j++) {
            if(C[i*N+j] != m_exp[i*N+j]) {
                err++;
                PRINTF("Error at index %d, %d, expected %d, got %d\n\r", i, j, m_exp[i*N+j], C[i*N+j]);
            }
        }
    }

    return err;
}
