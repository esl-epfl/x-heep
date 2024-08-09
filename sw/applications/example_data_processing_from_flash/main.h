// Copyright 2024 EPFL
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
//
// File: sw/applications/example_data_processing_from_flash/main.h
// Author:  Francesco Poluzzi
// Date: 29/07/2024

#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "x-heep.h"
#include "w25q128jw.h"
#include "matrices.h"

w25q_error_codes_t fill_buffer(uint32_t *source, uint32_t *buffer, uint32_t len);
void matmul(int32_t *A, int32_t *B, int32_t *res, int rowsA, int colsA, int colsB);

w25q_error_codes_t fill_buffer(uint32_t *source, uint32_t *buffer, uint32_t len){
    uint32_t *source_flash = heep_get_flash_address_offset(source);
    w25q_error_codes_t status = w25q128jw_read_standard(source_flash, buffer, len*4);
    return status;
}

void matmul(int32_t *A, int32_t *B, int32_t *res, int rowsA, int colsA, int colsB) {
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            int32_t sum = 0;
            for (int k = 0; k < colsA; k++) {
                sum += A[i*colsA + k] * B[k*colsB + j];
            }
            res[i*colsB + j] = sum;
        }
    }
}


#endif // DATA_H_
