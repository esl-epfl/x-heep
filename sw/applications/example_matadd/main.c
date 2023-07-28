// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "matrixAdd32.h"
#include "x-heep.h"

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

void __attribute__ ((noinline)) matrixAdd(int32_t * A, int32_t * B, int32_t * C, int N, int M);
uint32_t check_results(int32_t *  C, int N, int M);

int32_t m_c[16*16];

int main()
{
    int N = WIDTH;
    int M = HEIGHT;
    uint32_t errors = 0;
    unsigned int instr, cycles, ldstall, jrstall, imstall;

    CSR_WRITE(CSR_REG_MCYCLE, 0);

    //execute the kernel
    matrixAdd(m_a, m_b, m_c, N, M);

    CSR_READ(CSR_REG_MCYCLE, &cycles) ;

    //stop the HW counter used for monitoring

    errors = check_results(m_c, N, M);

    PRINTF("program finished with %d errors and %d cycles\n\r", errors, cycles);
    return errors;
}

void __attribute__ ((noinline)) matrixAdd(int32_t *  A, int32_t *  B, int32_t *  C, int N, int M)
{
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < M; j++) {
            C[i*N+j] = A[i*WIDTH+j] + B[i*WIDTH+j];
        }
    }
}

uint32_t check_results(int32_t * C, int N, int M)
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
