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
#define IW 3
#define IH 3
#define CH 1
#define FW 2
#define FH 2
#define STRIDES 1
#define PAD 1
#define BATCH 1

extern const uint32_t input_image_nchw[9];
extern const uint32_t golden_im2col_nchw[64];
extern const uint32_t input_image_nhwc[9];
extern const uint32_t golden_im2col_nhwc[64];

#endif
