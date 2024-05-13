// clang-format off
#include "cnn.h"

#include <stdlib.h>
#include <stdio.h>
// clang-format on

CnnHandle Cnn_create(Dim2D inputDim, Dim2D layer1Dim, Dim2D layer2Dim, Conv2DPadding layer1Pad,
                     Conv2DPadding layer2Pad) {
    CnnHandle self = (CnnHandle)my_malloc(sizeof(Cnn));
    // Create layers
    self->inputDim = inputDim;
    self->layer1Dim = layer1Dim;
    self->layer2Dim = layer2Dim;
    // TODO: calculate
    self->outputDim = (Dim2D){1u, 256u};

    self->layer1Pad = layer1Pad;
    self->layer2Pad = layer2Pad;

    self->layer1Weights = NULL;
    self->layer2Weights = NULL;

    return self;
}

void Cnn_destroy(CnnHandle self) {
    // free(self->layer1Weights);
    // free(self->layer2Weights);
    free(self);
}

void Cnn_forwardFxp(CnnHandle self, fxp32* input, fxp32* output) {
    // fxp32* layer1Output = (fxp32*)my_calloc(self->inputDim.x * self->inputDim.y, sizeof(fxp32));
    // Conv2DLayer_forwardFxp(self->layer1, self->inputDim, input, layer1Output);
    // Conv2DLayer_forwardFxp(self->layer2, self->inputDim, layer1Output, output);
    // free(layer1Output);
    printf("Cnn_forwardFxp not implemented\n");
}

void Cnn_forwardFloat(CnnHandle self, float* input, float* output) {
    float* layer1Output = (float*)my_calloc(self->inputDim.x * self->inputDim.y, sizeof(float));
    convolve2DFloat(input, layer1Output, self->layer1Weights, self->inputDim.x, self->inputDim.y, self->layer1Dim.x,
                    self->layer1Dim.y, self->layer1Pad == VALID);
    convolve2DFloat(layer1Output, output, self->layer2Weights, self->inputDim.x, self->inputDim.y, self->layer2Dim.x,
                    self->layer2Dim.y, self->layer2Pad == VALID);
    for (int i = 0; i < 15; ++i) {
        printf("layer2 output[%d] = %d\n", i, (int)(100000*output[i]));
    }
    free(layer1Output);
}

void Cnn_setWeights1Float(CnnHandle self, float* weights) {
    self->layer1Weights = weights;
}

void Cnn_setWeights2Float(CnnHandle self, float* weights) {
    self->layer2Weights = weights;
}

// void Cnn_setWeights1Fxp(CnnHandle self, fxp32* weights) {
//     for (int i = 0; i < self->layer1Dim.x * self->layer1Dim.y; ++i) {
//         self->layer1Weights[i] = fxp32_fxpMulToFloat(weights[i]);
//     }
// }

// void Cnn_setWeights2Fxp(CnnHandle self, fxp32* weights) {
//     for (int i = 0; i < self->layer2Dim.x * self->layer2Dim.y; ++i) {
//         self->layer2Weights[i] = fxp32_fxpMulToFloat(weights[i]);
//     }
// }

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

float Cnn_sampleLoss(CnnHandle self, complex_t* ypred, complex_t* ytrue) {
    float loss = 0.0f;
    // NOTE: could be optimized by reusing .r and .i for the abs
    fxpMul* abs = (fxpMul*)my_calloc(self->outputDim.y * self->outputDim.x, sizeof(fxpMul));
    for (int i = 0; i < self->outputDim.y * self->outputDim.x; ++i) {
        abs[i] = fxp32_pow2(ytrue[i].r - ypred[i].r) + fxp32_pow2(ytrue[i].i - ypred[i].i);
    }
    for (int i = 0; i < self->outputDim.y * self->outputDim.x; ++i) {
        loss += fxp32_fxpMulToFloat(abs[i]);
    }
    return loss;
}

void Cnn_freezeModel(CnnHandle self) {
    // Conv2DLayer_transformWeightsToFxp(self->layer1);
    // Conv2DLayer_transformWeightsToFxp(self->layer2);
    printf("Cnn_freezeModel not implemented\n");
}