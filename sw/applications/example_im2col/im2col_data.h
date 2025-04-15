#ifndef TEST_DATA_H
#define TEST_DATA_H

#include <stdint.h>

#define IH 12
#define IW 8
#define CH 1
#define BATCH 1
#define FW 3
#define FH 3
#define TOP_PAD 1
#define BOTTOM_PAD 2
#define LEFT_PAD 1
#define RIGHT_PAD 1
#define STRIDE_D1 4
#define STRIDE_D2 2
#define INPUT_DATATYPE 0

extern const uint32_t input_image_nchw[96];
extern const uint32_t golden_im2col_nchw[126];

#endif // TEST_DATA_H
