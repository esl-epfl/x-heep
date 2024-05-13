#include "conv2dlayer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Conv2DLayerHandle Conv2DLayer_create(Dim2D dim, Conv2DPadding padding) {
//     Conv2DLayerHandle self = (Conv2DLayerHandle)malloc(sizeof(Conv2DLayer));
//     self->dim = dim;
//     self->padding = padding;
//     self->weightsFxp = (fxp32*)calloc(dim.x * dim.y, sizeof(fxp32));
//     self->weightsFloat = (float*)calloc(dim.x * dim.y, sizeof(float));
//     return self;
// }

// void Conv2DLayer_destroy(Conv2DLayerHandle self) {
//     free(self->weightsFxp);
//     free(self->weightsFloat);
//     free(self);
// }

bool Conv2DLayer_setWeightsFxp(Conv2DLayerHandle self, fxp32* weights) {
    memcpy(self->weightsFxp, weights, self->dim.x * self->dim.y * sizeof(fxp32));
    // TODO: make sure we get the right size of weights
    return true;
}

bool Conv2DLayer_setWeightsFloat(Conv2DLayerHandle self, float* weights) {
    memcpy(self->weightsFloat, weights, self->dim.x * self->dim.y * sizeof(float));
    // TODO: make sure we get the right size of weights
    return true;
}

void Conv2DLayer_transformWeightsToFxp(Conv2DLayerHandle self) {
    for (int i = 0; i < self->dim.x * self->dim.y; ++i) {
        self->weightsFxp[i] = fxp32_fromFloat(self->weightsFloat[i]);
    }
}

// Could be optimized
void convolve2DFxp(fxp32* input, fxp32* output, fxp32* kernel, int inx, int iny, int kerx, int kery, bool valid) {

    // float max = 0;
    // float min = 0;

    // get kernel center
    if (kerx % 2 != 1 || kery % 2 != 1) {
        printf("Kernel size must be odd\n");
        exit(EXIT_FAILURE);
    }
    int cx = kerx / 2;
    int cy = kery / 2;

    int mMax, mMin;
    int nMax, nMin;
    fxp32 sum, w, in;

    int iMin = 0;
    int jMin = 0;
    int iMax = inx;
    int jMax = iny;

    if (valid) {
        iMin = cx;
        jMin = cy;
        iMax = inx - cx;
        jMax = iny - cy;
    }

    for (int i = iMin; i < iMax; ++i) {
        for (int j = jMin; j < jMax; ++j) {
            mMin = i - cx >= 0 ? i - cx : 0;
            mMax = i + cx + 1 < inx ? i + cx + 1 : inx;
            nMin = j - cy >= 0 ? j - cy : 0;
            nMax = j + cy + 1 < iny ? j + cy + 1 : iny;
            sum = 0;
            for (int m = mMin; m < mMax; ++m) {
                for (int n = nMin; n < nMax; ++n) {
                    // printf("input access: %d, %d\n", m, n);
                    in = input[m * iny + n];
                    // printf("kernel access: %d, %d\n", m - i + cx, n - j +
                    // cy);
                    w = kernel[(m - i + cx) * kery + (n - j + cy)];
                    sum += fxp32_mul(w, in);
                    // if (sum > max) {
                    //     max = sum;
                    // }
                    // if (sum < min) {
                    //     min = sum;
                    // }
                    // printf("factor: %.2f\n", w * in);
                }
            }
            if (valid)
                output[(i - cx) * (iny - kery + 1) + (j - cy)] = sum;
            else
                output[i * iny + j] = sum;
            // printf("access output: %d\n", (i-cx)*(iny - kery + 1) + (j-cy));
            // printf("sum = %.2f\n", sum);
        }
    }
    // printf("max: %.6f, min: %.6f\n", max, min);
}

// Could be optimized
void convolve2DFloat(float* input, float* output, float* kernel, int inx, int iny, int kerx, int kery, bool valid) {
    // get kernel center
    if (kerx % 2 != 1 || kery % 2 != 1) {
        printf("Kernel size must be odd\n");
        exit(EXIT_FAILURE);
    }
    int cx = kerx / 2;
    int cy = kery / 2;

    int mMax, mMin;
    int nMax, nMin;
    float sum, w, in;

    int iMin = 0;
    int jMin = 0;
    int iMax = inx;
    int jMax = iny;

    if (valid) {
        iMin = cx;
        jMin = cy;
        iMax = inx - cx;
        jMax = iny - cy;
    }

    for (int i = iMin; i < iMax; ++i) {
        for (int j = jMin; j < jMax; ++j) {
            mMin = i - cx >= 0 ? i - cx : 0;
            mMax = i + cx + 1 < inx ? i + cx + 1 : inx;
            nMin = j - cy >= 0 ? j - cy : 0;
            nMax = j + cy + 1 < iny ? j + cy + 1 : iny;
            sum = .0f;
            for (int m = mMin; m < mMax; ++m) {
                for (int n = nMin; n < nMax; ++n) {
                    in = input[m * iny + n];
                    w = kernel[(m - i + cx) * kery + (n - j + cy)];
                    sum += w * in;
                }
            }
            if (valid)
                output[(i - cx) * (iny - kery + 1) + (j - cy)] = sum;
            else
                output[i * iny + j] = sum;
        }
    }
}

void Conv2DLayer_forwardFxp(Conv2DLayerHandle self, Dim2D inputDim, fxp32* input, fxp32* output) {
    convolve2DFxp(input, output, self->weightsFxp, inputDim.x, inputDim.y, self->dim.x, self->dim.y,
                  self->padding == VALID);
}

void Conv2DLayer_forwardFloat(Conv2DLayerHandle self, Dim2D inputDim, float* input, float* output) {
    convolve2DFloat(input, output, self->weightsFloat, inputDim.x, inputDim.y, self->dim.x, self->dim.y,
                    self->padding == VALID);
}