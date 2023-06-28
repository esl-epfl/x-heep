// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "matrixAdd32.h"

#define DEBUG_OUTPUT
#define FS_INITIAL 0x01

void __attribute__ ((noinline)) matrixAdd(float * A, float * B, float * C, int N, int M);
uint32_t check_results(float *  C, int N, int M);

float m_c[HEIGHT*WIDTH];

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

    printf("program finished with %d errors and %d cycles\n", errors, cycles);
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
            #ifdef DEBUG_OUTPUT
                printf("Error at index %d, %d, expected %d, got %d\n", i, j, m_exp[i*WIDTH+j], C[i*N+j]);
            #endif
            }
        }
    }

    return err;
}
