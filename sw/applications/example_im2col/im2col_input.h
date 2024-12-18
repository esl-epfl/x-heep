#ifndef INPUT_IMAGE_NCHW_H
#define INPUT_IMAGE_NCHW_H

/*
	Copyright EPFL contributors.
	Licensed under the Apache License, Version 2.0, see LICENSE for details.
	SPDX-License-Identifier: Apache-2.0
*/

#include <stdint.h>

#define IH 5
#define IW 5
#define CH 1
#define BATCH 1
#define FH 3
#define FW 3
#define TOP_PAD 1
#define BOTTOM_PAD 1
#define LEFT_PAD 1
#define RIGHT_PAD 1
#define STRIDE_D1 1
#define STRIDE_D2 1

extern const uint32_t input_image_nchw[25];

#endif // INPUT_IMAGE_NCHW_H
