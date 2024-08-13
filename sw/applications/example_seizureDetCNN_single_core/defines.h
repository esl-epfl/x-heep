// Porting to X-Heep : Francesco Poluzzi
/*
 *  Copyright (c) [2024] [Embedded Systems Laboratory (ESL), EPFL]
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


//////////////////////////////////////////////////////
// Main Author:     Alireza Amirshahi               //
// Optimizations:   Dimitrios Samakovlis            //
/////////////////////////////////////////////////////



#include <stdlib.h>
#include <stdint.h>

#ifndef EPILEPSYGAN_GAN_H
#define EPILEPSYGAN_GAN_H

#define NUM_FRACTION_DATA 10
#define NUM_FRACTION_CNV_FC 8
#define NUM_FRACTION_BN 5
#define NEG_INF (-(1<<14))
#define INPUT_LEN (23 * 1024)

#define MUL_CONV(x, y, num) (int32_t)((int)(x)*(int)(y))>>(num)
#define MUL(x, y, num) (short)(((int)(x)*(int)(y))>>(num))

#define mem2d(data,data_len,j,i)   data[((j)*(data_len))+(i)]
#define mem3d(filter,filter_len,filter_depth,n,k,i)   filter[((n)*(filter_depth)+(k))*(filter_len)+(i)]


extern int16_t input_array[INPUT_LEN];
    
//========= DEFINE PRINTING OUTPUT ================//
// #define PRINT_INPUT
// #define PRINT_SUM
// #define PRINT_FC0_OUT
// #define PRINT_FC1_OUT
// #define PRINT_OVERFLOW
// #define PRINT_BLOCK
#define PRINT_OUTPUT
#define PRINT_CYCLES

extern int8_t* conv1d_w[3];
extern int8_t* conv1d_b[3];
extern int8_t* dense_w[2];
extern int8_t* dense_b[2];
extern int8_t* bn[12];

#endif //EPILEPSYGAN_GAN_H
