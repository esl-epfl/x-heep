/*
   Copyright EPFL contributors.
  Licensed under the Apache License, Version 2.0, see LICENSE for details.
  SPDX-License-Identifier: Apache-2.0

  Author: Tommaso Terzano <tommaso.terzano@epfl.ch>

  Info: Contains randomly generated input activations and the golden result of the im2col algorithm, computed with either Pytorch or Tensorflow,
  depending on the format.
*/

#include "im2colGolden.h"

const uint32_t input_image_nchw[9] = {
    41333, 64428, 16530,
    40448, 42251, 53689,
    5895, 5611, 60392
};
const uint32_t golden_im2col_nchw[64] = {
    0, 0, 0, 0, 0, 41333, 64428, 16530, 0, 40448, 42251, 53689, 0, 5895, 5611, 60392,
    0, 0, 0, 0, 41333, 64428, 16530, 0, 40448, 42251, 53689, 0, 5895, 5611, 60392, 0,
    0, 41333, 64428, 16530, 0, 40448, 42251, 53689, 0, 5895, 5611, 60392, 0, 0, 0, 0,
    41333, 64428, 16530, 0, 40448, 42251, 53689, 0, 5895, 5611, 60392, 0, 0, 0, 0, 0
};


const uint32_t input_image_nhwc[9] = {
 55709, 59898, 39852,
 41363, 33436, 40818,
 15986, 15877, 55307
};
const uint32_t golden_im2col_nhwc[64] = {
 0, 0, 0, 55709,
 0, 0, 55709, 59898,
 0, 0, 59898, 39852,
 0, 0, 39852, 0,
 0, 55709, 0, 41363,
 55709, 59898, 41363, 33436,
 59898, 39852, 33436, 40818,
 39852, 0, 40818, 0,
 0, 41363, 0, 15986,
 41363, 33436, 15986, 15877,
 33436, 40818, 15877, 55307,
 40818, 0, 55307, 0,
 0, 15986, 0, 0,
 15986, 15877, 0, 0,
 15877, 55307, 0, 0,
 55307, 0, 0, 0
};
