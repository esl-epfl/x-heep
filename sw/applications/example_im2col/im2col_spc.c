/*
 *  Copyright EPFL contributors.
 *  Licensed under the Apache License, Version 2.0, see LICENSE for details.
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
 *  
 *  Info: This is the driver library of the im2col SPC (Smart Peripheral Controller). 
 *        It defines functions and structures to easly define, verify and launch the im2col SPC.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "im2colGolden.h"
#include "dma.h"
#include "im2col_spc_regs.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "rv_plic.h"
#include "csr.h"

/****************************************************************************/
/**                                                                        **/
/*                        DEFINITIONS AND MACROS                            */
/**                                                                        **/
/****************************************************************************/

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

/* A transaction contains all the data needed to setup and run the im2col SPC */
typedef struct {
  uint32_t* src_ptr; /* Pointer to start address from where data will be processed */
  uint32_t* dst_ptr; /* Pointer to destination of the processed data */
  uint8_t num_ch; /* Number of DMA channels that the SPC has access to */
  dma_data_type_t datatype; /* Datatype of the data to be copied */
  uint8_t filter_height; /* Height of the filter */
  uint8_t filter_width; /* Width of the filter */
  uint32_t input_height; /* Height of the input matrix */
  uint32_t input_width; /* Width of the input matrix */
  uint32_t ch_col; /* Number of channels in the output matrix */
  uint32_t n_patches_width; /* Number of patches in the width direction */
  uint32_t n_patches_height; /* Number of patches in the height direction */
  uint32_t top_pad; /* Top padding */
  uint32_t bottom_pad; /* Bottom padding */
  uint32_t left_pad; /* Left padding */
  uint32_t right_pad; /* Right padding */
  uint32_t stride_d1; /* Stride in the first dimension */
  uint32_t stride_d2; /* Stride in the second dimension */
  uint32_t batch; /* Number of batches */
  uint32_t channels; /* Number of channels */
  uint32_t adapted_pad_right; /* Right padding after the adaptation */
  uint32_t adapted_pad_bottom; /* Bottom padding after the adaptation */
  char interrupt_enable; /* Enable interrupt */

} im2col_trans_t;

