#ifndef IMAGE_AND_COL_H
#define IMAGE_AND_COL_H

#include <stdint.h>

#define IW 4
#define IH 4
#define CH 3
#define FW 2
#define FH 2
#define STRIDES 1
#define PAD 0

#define BATCH 1

extern const uint32_t input_image[48];
extern const uint32_t golden_im2col[108];
#endif
