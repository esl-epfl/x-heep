// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "matrixAdd32.h"
#include "x-heep.h"

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA 1
#define PRINTF_IN_SIM 0

#if TARGET_SIM && PRINTF_IN_SIM
#ifndef TEST_MODE
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define PRINTF_TEST(...)
#else
#define PRINTF(...)
#define PRINTF_TEST(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif
#elif PRINTF_IN_FPGA && !TARGET_SIM
#ifndef TEST_MODE
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define PRINTF_TEST(...)
#else
#define PRINTF(...)
#define PRINTF_TEST(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif
#else
#define PRINTF(...)
#define PRINTF_TEST(...)
#endif

void __attribute__((noinline)) matrixAdd(int32_t *A, int32_t *B, int32_t *C, int N, int M);
uint32_t check_results(int32_t *C, int N, int M);

int32_t m_c[16 * 16];

int main()
{
    int N = WIDTH;
    int M = HEIGHT;
    uint32_t errors = 0;
    unsigned int instr, cycles;

    // enable mcycle csr
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

    CSR_WRITE(CSR_REG_MCYCLE, 0);

    // execute the kernel
    matrixAdd(m_a, m_b, m_c, N, M);

    CSR_READ(CSR_REG_MCYCLE, &cycles);

    errors = check_results(m_c, N, M);

    PRINTF("program finished with %d errors and %d cycles\n\r", errors, cycles);

    PRINTF_TEST("%d&\n", errors);

    return errors;
}

void __attribute__((noinline)) matrixAdd(int32_t *A, int32_t *B, int32_t *C, int N, int M)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            C[i * N + j] = A[i * WIDTH + j] + B[i * WIDTH + j];
        }
    }
}

uint32_t check_results(int32_t *C, int N, int M)
{
    // check
    int i, j;
    uint32_t err = 0;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            if (C[i * N + j] != m_exp[i * WIDTH + j])
            {
                err++;
                PRINTF("Error at index %d, %d, expected %d, got %d\n\r", i, j, m_exp[i * WIDTH + j], C[i * N + j]);
            }
        }
    }

    return err;
}
