/*
   Copyright EPFL contributors.
  Licensed under the Apache License, Version 2.0, see LICENSE for details.
  SPDX-License-Identifier: Apache-2.0

  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>

  Info: Header file of im2colGolden, contains activations parameters and the prototypes of both input tensors and golden output.
*/

#ifndef IMAGE_AND_COL_H
#define IMAGE_AND_COL_H

#include <stdint.h>

/* Parameters */
#define IW 4
#define IH 4
#define CH 1
#define FW 2
#define FH 2
#define BATCH 1
#define STRIDE_D1 2
#define STRIDE_D2 2
#define TOP_PAD 1
#define BOTTOM_PAD 1
#define LEFT_PAD 1
#define RIGHT_PAD 1
extern const uint32_t input_image_nchw[16];
extern const uint32_t golden_im2col_nchw[36];
extern const uint32_t input_image_nhwc[16];
extern const uint32_t golden_im2col_nhwc[36];

#endif
