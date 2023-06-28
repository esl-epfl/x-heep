// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "matrixAdd32.h"

#define FS_INITIAL 0x01

/* Enable printf by default only for FPGA. */
#ifdef TARGET_PYNQ_Z2
#define DEBUG
#endif // TARGET_PYNQ_Z2
 
// Use PRINTF instead of printf to remove print by default
#ifdef DEBUG
  #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
  #define PRINTF(...)
#endif // DEBUG

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

    PRINTF("program finished with %d errors and %d cycles\n\r", errors, cycles);
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
