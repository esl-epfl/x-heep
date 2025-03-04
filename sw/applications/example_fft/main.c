// Copyright 2024 EPFL 
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: main.c
// Author: Francesco Poluzzi
// Date: 04/09/2024
// Description: FFT example for X-Heep
// Functions for the FFT computation are in fft.h
// Parameters as FFT_LEN and DECIMAL_BITS can be changed in data.h. To also automate the input and 
// golden model generation with different parameters, the script datagen.py can be used. 

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "data.h"
#include "x-heep.h"
#include "timer_sdk.h"
#include "fft.h"

/* By default, PRINTs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   1

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

// Tolerance for the comparison of the results in fixed point (needs to be adjusted based on the number of decimal bits).
// The error is due to shifts and roundings in the fixed-point computation.
#define TOLERANCE 0x000000f 

int16_t  __attribute__((aligned(4))) R_radix_2[2 * FFT_LEN] ;
int16_t  __attribute__((aligned(4))) R_radix_4[2 * FFT_LEN] ; 
int16_t  __attribute__((aligned(4))) twiddle_factors_radix2[FFT_LEN]; 
int16_t  __attribute__((aligned(4))) twiddle_factors_radix4[FFT_LEN]; 
int16_t  __attribute__((aligned(4))) w_real_fixed[FFT_LEN / 2];
int16_t  __attribute__((aligned(4))) w_imag_fixed[FFT_LEN / 2];
int32_t __attribute__((aligned(4))) xrev_32[ FFT_LEN];
int16_t  __attribute__((aligned(4))) xrev[2* FFT_LEN];
int16_t  __attribute__((aligned(4))) bit_reversed_seq_radix2[FFT_LEN];
int16_t  __attribute__((aligned(4))) bit_reversed_seq_radix4[FFT_LEN];

int main(void)
{
    uint32_t radix2_cycles, radix4_cycles;

    if(!is_power_of(FFT_LEN, 2)){
        PRINTF("FFT_LEN must be a power of 2, FFT radix 2 cannot be performed.\n");
        return EXIT_FAILURE;
    }

    PRINTF("Starting radix 2 FFT\n");

        // precompute twiddle factors for radix-2 FFT
    compute_twiddle_factors_radix2(twiddle_factors_radix2, FFT_LEN, DECIMAL_BITS);

    // precompute bit reversed sequence
    get_bit_reversed_seq(bit_reversed_seq_radix2, FFT_LEN, log_floor(FFT_LEN, 2), 2);

    timer_cycles_init();
    timer_start();

    iterative_FFT_radix2(A, R_radix_2, FFT_LEN, twiddle_factors_radix2, DECIMAL_BITS, w_real_fixed, w_imag_fixed, xrev, bit_reversed_seq_radix2);

    radix2_cycles = timer_stop();

    for(int i = 0; i < 2 * FFT_LEN; i++){
        if(abs(R_radix_2[i] - R[i] > TOLERANCE)){
            PRINTF("Error: R_gold[%d] = %x, R_radix_2[%d] = %x\n", i, R[i], i, R_radix_2[i]);
            return 1;
        }
    }

    PRINTF("Radix-2 FFT took %d cycles\n", radix2_cycles);

    if(!is_power_of(FFT_LEN, 4)){
        PRINTF("FFT_LEN must be a power of 4, FFT radix 4 cannot be performed.\n");
        return EXIT_FAILURE;
    }

    PRINTF("Starting radix 4 FFT\n");

    // precompute twiddle factors for radix-4 FFT
    compute_twiddle_factors_radix4(twiddle_factors_radix4, FFT_LEN, DECIMAL_BITS);

    // precompute bit reversed sequence
    get_bit_reversed_seq(bit_reversed_seq_radix4, FFT_LEN, log_floor(FFT_LEN, 4), 4);

    timer_cycles_init();
    timer_start();

    iterative_FFT_radix4(A, R_radix_4, FFT_LEN, twiddle_factors_radix4, w_real_fixed, w_imag_fixed, xrev, DECIMAL_BITS, bit_reversed_seq_radix4);
    
    radix4_cycles = timer_stop();

    for(int i = 0; i < 2 * FFT_LEN; i++){
        if(abs(R_radix_2[i] - R[i] > TOLERANCE)){
            PRINTF("Error: R_gold[%d] = %x, R_radix_4[%d] = %x\n", i, R[i], i, R_radix_4[i]);
            return 1;
        }
    }   

    PRINTF("Radix-4 FFT took %d cycles\n", radix4_cycles);

    return EXIT_SUCCESS;
}
