/*
   Copyright EPFL contributors.
  Licensed under the Apache License, Version 2.0, see LICENSE for details.
  SPDX-License-Identifier: Apache-2.0

  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>

  Info: Contains randomly generated input activations and the golden result of the im2col algorithm, computed with either Pytorch or Tensorflow,
  depending on the format.
*/

#include "im2colGolden.h"

const uint32_t input_image_nchw[16] = {
    22861, 56321, 43036, 39097,
    25203, 52591, 63250, 28117,
    5347, 61899, 284, 48053,
    52288, 22222, 30793, 7841
};
const uint32_t golden_im2col_nchw[36] = {
    0, 0, 0, 0, 52591, 28117, 0, 22222, 7841,
    0, 0, 0, 25203, 63250, 0, 52288, 30793, 0,
    0, 56321, 39097, 0, 61899, 48053, 0, 0, 0,
    22861, 43036, 0, 5347, 284, 0, 0, 0, 0
};


const uint32_t input_image_nhwc[16] = {
 4374, 32069, 422, 22188,
 52745, 21102, 23056, 56444,
 38503, 59402, 54928, 56427,
 6398, 40434, 27898, 204
};
const uint32_t golden_im2col_nhwc[36] = {
 0, 0, 0, 4374,
 0, 0, 32069, 422,
 0, 0, 22188, 0,
 0, 52745, 0, 38503,
 21102, 23056, 59402, 54928,
 56444, 0, 56427, 0,
 0, 6398, 0, 0,
 40434, 27898, 0, 0,
 204, 0, 0, 0
};
