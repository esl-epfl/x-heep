#!/usr/bin/env python

## Copyright 2024 EPFL
## Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
## SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

# Author: Francesco Poluzzi
# This script generates the data.h file for the example_fft application, that contains the FFT input and the golden output.
# type " python datagen.py " in the terminal from the example_fft application folder to generate the data.h file.

import sys
import random
import numpy as np

# CONFIGURABLE PARAMETERS
SIZE  = 16
decimal_bits = 10
seed = 9

def format_matrix(matrix: np.ndarray, name: str) -> str:
    # Ensure the matrix is int16 (signed 16-bit integer)
    if matrix.dtype != np.int16:
        raise ValueError("Input matrix must be of dtype int16")
    
    num_bits = matrix.dtype.itemsize * 8
    array_ctype = "int16_t"
    
    # Convert each element to its 2's complement hexadecimal representation
    rows = []
    for row in matrix:
        row = np.atleast_1d(row)  # Ensure row is array-like for iteration
        # Format each element as a 16-bit signed integer in hex format
        hex_values = [f"{element if element >= 0 else (1 << num_bits) + element:#06x}" for element in row]
        rows.append(hex_values)

    # Format the matrix into a C-style array
    matrix_contents = f"{array_ctype} {name}[{matrix.size}] = {{\n"
    if len(rows) > 1:
        matrix_contents += ',\n'.join([f"    {', '.join(row)}" for row in rows])
    else:
        matrix_contents += f"    {', '.join(rows[0])}"
    matrix_contents += '\n};\n\n'

    return matrix_contents


def generate_fft_twiddle_factors_radix2(N):
    # Number of twiddle factors is N
    num_twiddle_factors = N

    # Generate the angles for the twiddle factors
    angles = np.linspace(0, -2 * np.pi, num_twiddle_factors, endpoint=False)

    # Compute the real and imaginary parts
    real_parts = np.cos(angles)
    imaginary_parts = np.sin(angles)

    # Concatenate real and imaginary parts
    twiddle_factors = np.empty(2 * num_twiddle_factors)
    twiddle_factors[0::2] = real_parts
    twiddle_factors[1::2] = imaginary_parts

    return twiddle_factors

import numpy as np

def generate_fft_twiddle_factors_radix4(N):
    # Ensure N is divisible by 4 for Radix-4 FFT
    if N % 4 != 0:
        raise ValueError("N must be a multiple of 4 for Radix-4 FFT")

    num_twiddle_groups = N // 4
    angles_k1 = np.linspace(0, -2 * np.pi / N, num_twiddle_groups, endpoint=False)
    angles_k2 = 2 * angles_k1  # Corresponds to W_N^{2k}
    angles_k3 = 3 * angles_k1  # Corresponds to W_N^{3k}
    real_parts_k1 = np.cos(angles_k1)
    imag_parts_k1 = np.sin(angles_k1)
    real_parts_k2 = np.cos(angles_k2)
    imag_parts_k2 = np.sin(angles_k2)
    real_parts_k3 = np.cos(angles_k3)
    imag_parts_k3 = np.sin(angles_k3)
    twiddle_factors = np.empty(6 * num_twiddle_groups)
    twiddle_factors[0::6] = real_parts_k1
    twiddle_factors[1::6] = imag_parts_k1
    twiddle_factors[2::6] = real_parts_k2
    twiddle_factors[3::6] = imag_parts_k2
    twiddle_factors[4::6] = real_parts_k3
    twiddle_factors[5::6] = imag_parts_k3

    return twiddle_factors


def write_arr(f, name, arr, ctype, size):
    f.write("const " + ctype + " " + name + "[2*FFT_LEN] = {\n")

    for row in arr:
        for elem in row[:-1]:
            f.write('%d,' % (elem))
        f.write('%d,\n' % (row[-1]))

    f.write('};\n\n')
    return

def generate_random_matrix(num_channels, length, decimal_bits):
    """
    Generate a random matrix with num_channels rows and length columns.
    """
    
    real_part = np.random.uniform(-0.01, 0.01, (num_channels, length)) #* (2**(-decimal_bits))
    imag_part = np.random.uniform(-0.01, 0.01, (num_channels, length)) #* (2**(-decimal_bits))
    
    matrix = real_part + 1j * imag_part
    
    return matrix

def perform_fft(matrix):
    """
    Perform FFT on each row of the matrix and return the result matrix.
    """
    return np.fft.fft(matrix, axis=1)

def convert_to_fixed_point(matrix, decimal_places=8):
    """
    Convert the matrix to fixed-point format with a specified number of decimal bits.
    Each element in the output matrix is of type 'int16'.
    """
    # Scaling factor for conversion
    scaling_factor = 1 << decimal_places  # 2^decimal_bits (256 for Q1.8 format)

    # Convert to fixed-point representation
    real_part = np.real(matrix) * scaling_factor
    imag_part = np.imag(matrix) * scaling_factor
    
    # Clip values to ensure they fit within the range of int16
    real_part = np.clip(real_part, -32768, 32767)
    imag_part = np.clip(imag_part, -32768, 32767)
    
    # Convert to int16
    real_part = real_part.astype(np.int16)
    imag_part = imag_part.astype(np.int16)
    
    # Combine real and imaginary parts into a single matrix
    fixed_point_matrix = np.empty((matrix.shape[0], matrix.shape[1] * 2), dtype=np.int16)
    fixed_point_matrix[:, 0::2] = real_part
    fixed_point_matrix[:, 1::2] = imag_part
    
    return fixed_point_matrix

def convert_to_fixed_point_twiddles(array, decimal_bits=8):
    # The input 'array' is a 1D array with interleaved real and imaginary parts
    fixed_point_array = np.empty_like(array, dtype=np.int16)

    # Scale and convert to fixed-point for real and imaginary parts
    scale_factor = 1 << decimal_bits
    fixed_point_array = np.round(array * scale_factor).astype(np.int16)

    return fixed_point_array

################################################################################
f = open('data.h', 'w')
f.write('// This file is automatically generated\n// Type " python datagen.py " in the terminal to generate the data.h file. Configuration parameters can be changed in the datagen.py file.\n')
f.write('\n#ifndef DATA_H_\n')
f.write('#define DATA_H_\n\n')

np.random.seed(seed)

# Generate random input
input = generate_random_matrix(1, SIZE, decimal_bits)

# Perform FFT
fft_output = perform_fft(input)

# Comput twiddles
twiddles_radix2 = generate_fft_twiddle_factors_radix2(SIZE)
twiddles_radix4 = generate_fft_twiddle_factors_radix4(SIZE)

# Convert FFT result to fixed-point format
R = convert_to_fixed_point(fft_output, decimal_bits)
A = convert_to_fixed_point(input, decimal_bits)
W_radix2 = convert_to_fixed_point_twiddles(twiddles_radix2, decimal_bits)
W_radix4 = convert_to_fixed_point_twiddles(twiddles_radix4, decimal_bits)

print("Input:")
print(input)
print("A (fixed point) :")
print(A*2**-decimal_bits)
print("FFT output:")
print(fft_output)
print("R (fixed point):")
print(R*2**-decimal_bits)
print("Twiddles Radix-2 (fixed point):")
print([hex(x) for x in W_radix2.flatten()])  # Print in hexadecimal format
print("Twiddles Radix-4 (fixed point):")
print([hex(x) for x in W_radix4.flatten()])  # Print in hexadecimal format

f.write('#define FFT_LEN %d\n' % SIZE)
f.write('#define DECIMAL_BITS %d\n\n' % decimal_bits)

f.write(format_matrix( A, 'A'))
f.write(format_matrix( R, 'R'))
f.write(format_matrix( W_radix2, 'W_radix2'))
f.write(format_matrix( W_radix4, 'W_radix4'))

f.write('#endif')