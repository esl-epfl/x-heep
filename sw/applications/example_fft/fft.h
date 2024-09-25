// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: fft.h
// Author: Francesco Poluzzi
// Date: 04/09/2024
// Description: FFT-related functions for X-Heep 

#ifndef FFT_H
#define FFT_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "data.h"

#define PI 3.14159265358979323846

uint8_t log_floor(uint16_t N, uint8_t base) {
    if (N == 0) {
        return 0; // Log of 0 is undefined, return 0 for safety
    }
    uint8_t log_val = 0;

    if (base == 2) {
        while (N >>= 1) { // Right shift by 1 bit (equivalent to dividing by 2)
            log_val++;
        }
    } else if (base == 4) {
        while (N >= 4) {  // Right shift by 2 bits (equivalent to dividing by 4)
            N >>= 2;
            log_val++;
        }
    }
    return log_val;
}

bool is_power_of(uint16_t N, uint8_t base) {
    if (N == 0) {
        return false; 
    }
    if (base == 2) {
        return (N & (N - 1)) == 0;
    } else if (base == 4) {
        return (N & (N - 1)) == 0 && (N & 0x5555) != 0;
    }
    return false;  
}

// Function to perform bit reversal based on radix (base)
uint16_t bit_reversal(uint16_t n, uint8_t num_bits, uint8_t base) {
    uint16_t result = 0;

    // Calculate the number of bits to shift based on the base
    uint8_t shiftamt = (base == 2) ? 1 : 2;  // Shift by 1 for Radix-2, by 2 for Radix-4
    uint16_t mask = base - 1;  // Mask is 1 for Radix-2, 3 for Radix-4

    // Perform the bit reversal by repeatedly shifting and applying the mask
    for (uint8_t i = 0; i < num_bits; i++) {
        result <<= shiftamt;    // Shift result to the left by `shiftamt` bits
        result |= n & mask;     // Add the masked bits from `n` to `result`
        n >>= shiftamt;         // Shift `n` to the right by `shiftamt` bits
    }

    return result;
}

// Function to generate bit-reversed sequence
void get_bit_reversed_seq(uint16_t *seq, uint16_t N, uint8_t num_bits, uint8_t base) {
    for (uint16_t i = 0; i < N; i++) {
        seq[i] = bit_reversal(i, num_bits, base);
    }
}

void compute_twiddle_factors_radix2(int16_t* twiddle_factors, uint32_t N, uint8_t decimal_bits) {
    for (uint16_t j = 0; j < N / 2; j++) {
        float angle = -2.0 * PI * j / N;
        twiddle_factors[2 * j] = (int16_t)(cos(angle) * (1 << decimal_bits));   // Real part in Q1.8 format
        twiddle_factors[2 * j + 1] = (int16_t)(sin(angle) * (1 << decimal_bits)); // Imaginary part in Q1.8 format
    } 
}

void compute_twiddle_factors_radix4(int16_t *twiddle_factors, uint16_t N, int16_t decimal_bits) {
    // Twiddle factor for radix-4 FFT
    // N is the FFT size, which must be a power of 4
    // twiddle_factors is the output array where real and imaginary parts are interleaved
    uint16_t twiddle_count = N / 4; // Radix-4 FFT needs twiddles for N/4 size

    for (uint16_t k = 0; k < twiddle_count; k++) {
        // Compute the real and imaginary parts of W_N^k = e^(-2*pi*i*k/N)
        double angle = -2.0 * PI * k / N;
        double real_part = cos(angle);
        double imag_part = sin(angle);

        // Convert to fixed-point with the specified number of decimal bits
        int16_t real_fixed = (int16_t)(real_part * (1 << decimal_bits));
        int16_t imag_fixed = (int16_t)(imag_part * (1 << decimal_bits));

        // Store the values in the interleaved array
        twiddle_factors[2 * k] = real_fixed;      // Real part
        twiddle_factors[2 * k + 1] = imag_fixed;  // Imaginary part
    }
}

void __attribute__((noinline, aligned(4))) iterative_FFT_radix2(int16_t *x, int16_t *X, uint16_t N, int16_t *twiddle_factors, int16_t decimal_bits,
                           int16_t *w_real_fixed, int16_t *w_imag_fixed, int16_t *xrev, int16_t *bit_reversed_seq) {  

    // Perform bit reversal on the input array
    uint8_t num_bits = log_floor(N, 2);

    for (uint16_t i = 0; i < N; i++) {
        // Access real and imaginary parts independently
        xrev[2 * i] = x[2 * bit_reversed_seq[i]];         // Real part
        xrev[2 * i + 1] = x[2 * bit_reversed_seq[i] + 1]; // Imaginary part
    }

    // Stage 0 unrolled: simple additions and subtractions
    for (uint16_t i = 0; i < N; i += 2) {
        // Access real and imaginary parts for two points independently
        int16_t a_real = xrev[2 * i];
        int16_t a_imag = xrev[2 * i + 1];
        int16_t b_real = xrev[2 * (i + 1)];
        int16_t b_imag = xrev[2 * (i + 1) + 1];

        // No twiddle factor for the first stage (equivalent to multiplying by 1 + 0j)
        xrev[2 * i] = a_real + b_real;
        xrev[2 * i + 1] = a_imag + b_imag;
        xrev[2 * (i + 1)] = a_real - b_real;
        xrev[2 * (i + 1) + 1] = a_imag - b_imag;
    }

    // FFT processing for remaining stages
    uint16_t stage_count = num_bits;
    uint16_t twiddle_step = N/2;

    for (uint16_t stage = 1, step = 4; stage < stage_count; stage++, step *= 2) {
        uint16_t halfstep = step / 2;
        twiddle_step /= 2;

        for (uint16_t j = 0; j < halfstep; j++) {
            // Access precomputed twiddle factors from the interleaved array
            w_real_fixed[j] = twiddle_factors[2 * (j * twiddle_step)];
            w_imag_fixed[j] = twiddle_factors[2 * (j * twiddle_step) + 1];
        }

        // Modify the outer loop to increment from 0 to N, increasing by step
        for (uint16_t i = 0; i < N; i += step) {
            // Calculate the starting index for this iteration
            uint16_t idx = i;
            for (uint16_t j = 0; j < halfstep; j++) {
                // Butterfly operations - Access real and imag parts independently
                int16_t a_real = xrev[2 * (idx + j)];
                int16_t a_imag = xrev[2 * (idx + j) + 1];
                int16_t b_real = xrev[2 * (idx + j + halfstep)];
                int16_t b_imag = xrev[2 * (idx + j + halfstep) + 1];

                // Complex multiplication (b * w) using 32-bit intermediate values
                int32_t temp_real = ((int32_t)b_real * w_real_fixed[j] - (int32_t)b_imag * w_imag_fixed[j]) >> decimal_bits;
                int32_t temp_imag = ((int32_t)b_real * w_imag_fixed[j] + (int32_t)b_imag * w_real_fixed[j]) >> decimal_bits;

                // Store the results directly in the array, independently for real and imag parts
                xrev[2 * (idx + j)] = a_real + temp_real;
                xrev[2 * (idx + j) + 1] = a_imag + temp_imag;
                xrev[2 * (idx + j + halfstep)] = a_real - temp_real;
                xrev[2 * (idx + j + halfstep) + 1] = a_imag - temp_imag;
            }
        }
    }

    // Copy the result to output array X
    for (uint16_t i = 0; i < 2 * N; i++) {
        X[i] = xrev[i];
    }
}

// Radix-4 FFT
void __attribute__((noinline, aligned(4))) iterative_FFT_radix4(int16_t *x, int16_t *X, uint16_t N, int16_t *twiddle_factors, int16_t *w_real_fixed, int16_t * w_imag_fixed, int16_t *xrev, int8_t decimal_bits, int16_t * bit_reversed_seq) {

    uint16_t stage_count = log_floor(N, 4);  // Radix-4 halves the number of stages
    uint16_t twiddle_step = N;

    for (uint16_t i = 0; i < N; i++) {
        xrev[2 * i] = x[2 * bit_reversed_seq[i]];        // Real part
        xrev[2 * i + 1] = x[2 * bit_reversed_seq[i] + 1]; // Imaginary part
    }

    // Unroll the first stage: radix-4 butterfly (4 points per butterfly)
    uint16_t quarterstep = 1;
    for (uint16_t i = 0; i < N; i += 4) {
        uint16_t idx = i;
        // Load the real and imaginary parts for the 4 input points
        int16_t a_real = xrev[2 * idx];
        int16_t a_imag = xrev[2 * idx + 1];
        int16_t b_real = xrev[2 * (idx + quarterstep)];
        int16_t b_imag = xrev[2 * (idx + quarterstep) + 1];
        int16_t c_real = xrev[2 * (idx + 2 * quarterstep)];
        int16_t c_imag = xrev[2 * (idx + 2 * quarterstep) + 1];
        int16_t d_real = xrev[2 * (idx + 3 * quarterstep)];
        int16_t d_imag = xrev[2 * (idx + 3 * quarterstep) + 1];

        // Radix-4 butterfly calculations (without twiddle factors)
        int16_t t0_real = a_real + c_real;
        int16_t t0_imag = a_imag + c_imag;
        int16_t t1_real = b_real + d_real;
        int16_t t1_imag = b_imag + d_imag;

        int16_t t2_real = a_real - c_real;
        int16_t t2_imag = a_imag - c_imag;
        int16_t t3_real = b_real - d_real;
        int16_t t3_imag = b_imag - d_imag;

        // Output without twiddle factors
        xrev[2 * idx] = t0_real + t1_real;  // Result 1 real part
        xrev[2 * idx + 1] = t0_imag + t1_imag;  // Result 1 imag part

        xrev[2 * (idx + quarterstep)] = t2_real - t3_imag;  // Result 2 real part
        xrev[2 * (idx + quarterstep) + 1] = t2_imag + t3_real;  // Result 2 imag part

        xrev[2 * (idx + 2 * quarterstep)] = t0_real - t1_real;  // Result 3 real part
        xrev[2 * (idx + 2 * quarterstep) + 1] = t0_imag - t1_imag;  // Result 3 imag part

        xrev[2 * (idx + 3 * quarterstep)] = t2_real + t3_imag;  // Result 4 real part
        xrev[2 * (idx + 3 * quarterstep) + 1] = t2_imag - t3_real;  // Result 4 imag part
    }

    // Perform the remaining stages
    for (uint16_t stage = 1, step = 16; stage < stage_count; stage++, step *= 4) {
        uint16_t quarterstep = step / 4;
        twiddle_step /= 4;

        // Precompute the twiddle factors for this stage
        for (uint16_t j = 0; j < quarterstep; j++) {
            w_real_fixed[j] = twiddle_factors[2 * (j * twiddle_step)];
            w_imag_fixed[j] = twiddle_factors[2 * (j * twiddle_step) + 1];
        }

        // Loop over groups of 4 points per butterfly
        for (uint16_t i = 0; i < N; i += step) {
            uint16_t idx = i;

            // Perform load, butterfly, and store operations in a single loop
            for (uint16_t j = 0; j < quarterstep; j++) {
                // Load the data for the 4 points
                int16_t a_real = xrev[2 * (idx + j)];
                int16_t a_imag = xrev[2 * (idx + j) + 1];
                int16_t b_real = xrev[2 * (idx + j + quarterstep)];
                int16_t b_imag = xrev[2 * (idx + j + quarterstep) + 1];
                int16_t c_real = xrev[2 * (idx + j + 2 * quarterstep)];
                int16_t c_imag = xrev[2 * (idx + j + 2 * quarterstep) + 1];
                int16_t d_real = xrev[2 * (idx + j + 3 * quarterstep)];
                int16_t d_imag = xrev[2 * (idx + j + 3 * quarterstep) + 1];

                // Apply twiddle factors to b
                int16_t tw_b_real = ((int32_t)b_real * w_real_fixed[j] - (int32_t)b_imag * w_imag_fixed[j]) >> decimal_bits;
                int16_t tw_b_imag = ((int32_t)b_real * w_imag_fixed[j] + (int32_t)b_imag * w_real_fixed[j]) >> decimal_bits;

                int16_t res1_real = a_real + tw_b_real;  // a + Tw(b)
                int16_t res1_imag = a_imag + tw_b_imag;

                int16_t res2_real = a_real - tw_b_real;  // a - Tw(b)
                int16_t res2_imag = a_imag - tw_b_imag;

                // Apply twiddle factors to c
                int16_t tw_c_real = ((int32_t)c_real * w_real_fixed[2 * j] - (int32_t)c_imag * w_imag_fixed[2 * j]) >> decimal_bits;
                int16_t tw_c_imag = ((int32_t)c_real * w_imag_fixed[2 * j] + (int32_t)c_imag * w_real_fixed[2 * j]) >> decimal_bits;

                int16_t res3_real = a_real + tw_c_real;  // a + Tw(c)
                int16_t res3_imag = a_imag + tw_c_imag;

                int16_t res4_real = a_real - tw_c_real;  // a - Tw(c)
                int16_t res4_imag = a_imag - tw_c_imag;

                // Apply twiddle factors to d
                int16_t tw_d_real = ((int32_t)d_real * w_real_fixed[3 * j] - (int32_t)d_imag * w_imag_fixed[3 * j]) >> decimal_bits;
                int16_t tw_d_imag = ((int32_t)d_real * w_imag_fixed[3 * j] + (int32_t)d_imag * w_real_fixed[3 * j]) >> decimal_bits;

                // Store the results
                xrev[2 * (idx + j)] = res1_real;
                xrev[2 * (idx + j) + 1] = res1_imag;
                xrev[2 * (idx + j + quarterstep)] = res2_real;
                xrev[2 * (idx + j + quarterstep) + 1] = res2_imag;
                xrev[2 * (idx + j + 2 * quarterstep)] = res3_real;
                xrev[2 * (idx + j + 2 * quarterstep) + 1] = res3_imag;
                xrev[2 * (idx + j + 3 * quarterstep)] = res4_real;
                xrev[2 * (idx + j + 3 * quarterstep) + 1] = res4_imag;
            }
        }
    }

    // Copy the result to output array X
    for (uint16_t i = 0; i < 2 * N; i++) {
        X[i] = xrev[i];
    }
}

#define FIXED_POINT_SCALE (1 << DECIMAL_BITS)  

// Helper function to print the FFT result
void print_complex_array(int16_t *array, uint16_t N) {
    for (uint16_t i = 0; i < N; i++) {
        // Convert fixed-point to decimal by dividing by FIXED_POINT_SCALE
        int16_t real_part = array[2 * i];
        int16_t imag_part = array[2 * i + 1];

        // Calculate integer and fractional parts
        int16_t real_int = real_part / FIXED_POINT_SCALE;
        int16_t imag_int = imag_part / FIXED_POINT_SCALE;

        // Fractional part needs to be scaled and displayed with leading zeros if necessary
        int16_t real_frac = (real_part % FIXED_POINT_SCALE) * 1000 / FIXED_POINT_SCALE;
        int16_t imag_frac = (imag_part % FIXED_POINT_SCALE) * 1000 / FIXED_POINT_SCALE;

        // Adjust the fractional part for negative values, and ensure fractional part is positive
        if (real_part < 0 && real_frac > 0) {
            real_frac = 1000 - real_frac;
        }
        if (imag_part < 0 && imag_frac > 0) {
            imag_frac = 1000 - imag_frac;
        }

        // Print the complex number in the correct format
        // Use separate handling for real and imaginary signs
        printf("X[%d] = %c%d.%03d %c %d.%03dj\n", 
               i, 
               (real_part < 0) ? '-' : '+', abs(real_int), abs(real_frac),  // Real part
               (imag_part < 0) ? '-' : '+', abs(imag_int), abs(imag_frac));  // Imaginary part
    }
}

#endif // FFT_ITERATIVE_RADIX_2_H