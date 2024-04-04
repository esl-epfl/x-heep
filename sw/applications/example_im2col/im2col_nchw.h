#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_DIM 8


struct tensor {
    void *data;                     /**< Real data pointing to tensors */
    int32_t dim[MAX_DIM];           /**< Describes the size of each dimension in the tensor.  */
};

struct im2col_params {
    int32_t pad_top;               /**< The number of top padding */
    int32_t pad_down;              /**< The number of bottom padding */
    int32_t pad_left;              /**< The number of left padding */
    int32_t pad_right;             /**< The number of right padding */
    int32_t stride_h;              /**< Vertical step */
    int32_t stride_w;              /**< Horizontal step */
    int32_t kernel_h;              /**< Kernel height */
    int32_t kernel_w;              /**< Kernel width */
};

static int im2col_nchw_f32(struct tensor *input, struct tensor *output,
                                   struct im2col_params *params);

int32_t get_index(int32_t *dim, int32_t index0, int32_t index1, int32_t index2,
                          int32_t index3);