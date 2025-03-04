/*
    Copyright EPFL contributors.
    Licensed under the Apache License, Version 2.0, see LICENSE for details.
    SPDX-License-Identifier: Apache-2.0

    Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
                            <tommaso.terzano@gmail.com>

    Info: This simple HAL is used to load the im2col SPC and to run it. Remember, only one DMA channel
          at a time can be used!
*/

#ifndef _IM2COL_SPC_
#define _IM2COL_SPC_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "dma.h"
#include "im2col_spc_regs.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "rv_plic.h"
#include "csr.h"
#include <math.h>
#include "mmio.h"
#include "hart.h"
#include "fast_intr_ctrl.h"

/* Transaction structure */
typedef struct
{
    uint32_t* src;                /*!< Target from where the data will be copied. */
    uint32_t* dst;                /*!< Target to where the data will be copied. */
    uint32_t ch_mask;             /*!< Mask of the channels to be used. */
    uint32_t im_width;            /*!< Width of the input image. */
    uint32_t im_height;           /*!< Height of the input image. */
    uint32_t filter_width;        /*!< Width of the filter. */
    uint32_t filter_height;       /*!< Height of the filter. */
    uint32_t num_channels;        /*!< Number of channels. */
    uint32_t num_channels_col;    /*!< Number of channels to be processed. */
    uint32_t stride_d1;           /*!< Stride in the first dimension. */
    uint32_t stride_d2;           /*!< Stride in the second dimension. */
    uint32_t batch;               /*!< Number of batches. */
    uint32_t n_patches_w;         /*!< Number of patches in the width. */
    uint32_t n_patches_h;         /*!< Number of patches in the height. */
    uint32_t left_pad;            /*!< Padding on the left. */
    uint32_t right_pad;           /*!< Padding on the right. */
    uint32_t top_pad;             /*!< Padding on the top. */
    uint32_t bottom_pad;          /*!< Padding on the bottom. */
    uint32_t adpt_pad_right;      /*!< Adaptive padding on the right. */
    uint32_t adpt_pad_bottom;     /*!< Adaptive padding on the bottom. */
    uint32_t datatype;            /*!< Data type of the input. */
} im2col_trans_t;

int im2col_spc_run(im2col_trans_t trans);
void im2col_spc_init(uint32_t im2col_spc_base_addr_i);
__attribute__((weak, optimize("00"))) void handler_irq_im2col_spc(void);

#endif