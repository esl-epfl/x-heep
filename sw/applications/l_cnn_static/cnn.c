// clang-format off

#include "cnn.h"

#include <stdlib.h>
// clang-format on

// CnnHandle Cnn_create(Dim2D inputDim, Dim2D layer1Dim, Dim2D layer2Dim, Conv2DPadding layer1Pad,
//                      Conv2DPadding layer2Pad) {
//     CnnHandle self = (CnnHandle)malloc(sizeof(Cnn));
//     // Create layers
//     self->layer1 = Conv2DLayer_create(layer1Dim, layer1Pad);
//     self->layer2 = Conv2DLayer_create(layer2Dim, layer2Pad);

//     self->inputDim = inputDim;
//     // TODO: calculate
//     self->outputDim = (Dim2D){1u, 256u};
//     return self;
// }

// void Cnn_destroy(CnnHandle self) {
//     Conv2DLayer_destroy(self->layer1);
//     Conv2DLayer_destroy(self->layer2);
//     free(self);
// }

void Cnn_forwardFxp(CnnHandle self, fxp32* input, fxp32* output) {
#ifdef DYN_ALLOCATION
    fxp32* layer1Output = (fxp32*)calloc(self->inputDim.x * self->inputDim.y, sizeof(fxp32));
    Conv2DLayer_forwardFxp(self->layer1, self->inputDim, input, layer1Output);
    Conv2DLayer_forwardFxp(self->layer2, self->inputDim, layer1Output, output);
    free(layer1Output);
#else
    Conv2DLayer_forwardFxp(self->layer1, self->inputDim, input, self->layer1OutputFxp);
    Conv2DLayer_forwardFxp(self->layer2, self->inputDim, self->layer1OutputFxp, output);
#endif
}

void Cnn_forwardFloat(CnnHandle self, float* input, float* output) {
#ifdef DYN_ALLOCATION
    float* layer1Output = (float*)calloc(self->inputDim.x * self->inputDim.y, sizeof(float));
    Conv2DLayer_forwardFloat(self->layer1, self->inputDim, input, layer1Output);
    Conv2DLayer_forwardFloat(self->layer2, self->inputDim, layer1Output, output);
    free(layer1Output);
#else
    Conv2DLayer_forwardFloat(self->layer1, self->inputDim, input, self->layer1Output);
    Conv2DLayer_forwardFloat(self->layer2, self->inputDim, self->layer1Output, output);
#endif
}

void Cnn_predictFxp(CnnHandle self, fxp32* acc, fxp32* ppg, fxp32* output) {
    Cnn_forwardFxp(self, acc, output);
    for (int i = 0; i < self->outputDim.y * self->outputDim.x; ++i) {
        output[i] = ppg[i] - output[i];
    }
}

void Cnn_predictFloat(CnnHandle self, float* acc, float* ppg, float* output) {
    Cnn_forwardFloat(self, acc, output);
    for (int i = 0; i < self->outputDim.y * self->outputDim.x; ++i) {
        output[i] = ppg[i] - output[i];
    }
}

// float Cnn_sampleLoss(CnnHandle self, complex_t* ypred, complex_t* ytrue) {
//     float loss = 0.0f;
//     // NOTE: could be optimized by reusing .r and .i for the abs
//     fxpMul* abs = (fxpMul*)calloc(self->outputDim.y * self->outputDim.x, sizeof(fxpMul));
//     for (int i = 0; i < self->outputDim.y * self->outputDim.x; ++i) {
//         abs[i] = fxp32_pow2(ytrue[i].r - ypred[i].r) + fxp32_pow2(ytrue[i].i - ypred[i].i);
//     }
//     for (int i = 0; i < self->outputDim.y * self->outputDim.x; ++i) {
//         loss += fxp32_fxpMulToFloat(abs[i]);
//     }
//     return loss;
// }

void Cnn_freezeModel(CnnHandle self) {
    Conv2DLayer_transformWeightsToFxp(self->layer1);
    Conv2DLayer_transformWeightsToFxp(self->layer2);
}