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
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

void __attribute__ ((noinline)) matrixMul8(int8_t *  A, int8_t *  Bt, int32_t *  C, int N);

uint32_t check_results(int32_t * C, int N);

int32_t m_c[SIZE*SIZE];

int main()
{

    uint32_t errors = 0;
    unsigned int instr, cycles;

    //enable mcycle csr
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

    CSR_WRITE(CSR_REG_MCYCLE, 0);

    //execute the kernel
    matrixMul8(m_a, m_b_transposed, m_c, SIZE);

    CSR_READ(CSR_REG_MCYCLE, &cycles);

    errors = check_results(m_c, SIZE);

    PRINTF("program finished with %d errors and %d cycles\n\r", errors, cycles);
    return errors;
}

void __attribute__ ((noinline)) matrixMul8(int8_t *  A, int8_t *  Bt, int32_t *  C, int N)
{
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            int32_t acc = 0;
            for(int k = 0; k < N; k++) {
                acc+= A[i*N+k] * Bt[k*N+j];
            }
            C[i*N+j] = acc;
        }
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
