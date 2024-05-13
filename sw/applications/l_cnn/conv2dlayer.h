#ifndef CONV2DLAYER_H
#define CONV2DLAYER_H

#include <stdbool.h>
#include <stdint.h>

#include "fxp32.h"

// typedef struct __Conv2DLayer {
//     Dim2D dim;
//     Conv2DPadding padding;
//     fxp32* weightsFxp;
//     float* weightsFloat;
// } Conv2DLayer;

// typedef struct __Conv2DLayer* Conv2DLayerHandle;

// Conv2DLayerHandle Conv2DLayer_create(Dim2D dim, Conv2DPadding padding);
// void Conv2DLayer_destroy(Conv2DLayerHandle self);

// bool Conv2DLayer_setWeightsFxp(Conv2DLayerHandle self, fxp32* weights);
// bool Conv2DLayer_setWeightsFloat(Conv2DLayerHandle self, float* weights);
// void Conv2DLayer_transformWeightsToFxp(Conv2DLayerHandle self);

// void Conv2DLayer_forwardFxp(Conv2DLayerHandle self, Dim2D inputDim, fxp32* input, fxp32* output);
// void Conv2DLayer_forwardFloat(Conv2DLayerHandle self, Dim2D inputDim, float* input, float* output);

void convolve2DFxp(fxp32* input, fxp32* output, fxp32* kernel, int inx, int iny, int kerx, int kery, bool valid);
void convolve2DFloat(float* input, float* output, float* kernel, int inx, int iny, int kerx, int kery, bool valid);

#endif // CONV2DLAYER_H