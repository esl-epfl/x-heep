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
#define IW 22
#define IH 22
#define CH 3
#define FW 4
#define FH 4
#define STRIDES 2
#define PAD 2
#define BATCH 1

extern const uint32_t input_image_nchw[1452];
extern const uint32_t golden_im2col_nchw[6912];
// extern const uint32_t input_image_nhwc[1452];
// extern const uint32_t golden_im2col_nhwc[6912];

#endif
