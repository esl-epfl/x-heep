#include "cnn.h"

#include <stdlib.h>

CnnHandle Cnn_create(Dim2D inputDim, Dim2D layer1Dim, Dim2D layer2Dim, Conv2DPadding layer1Pad,
                     Conv2DPadding layer2Pad) {
    CnnHandle self = (CnnHandle)malloc(sizeof(Cnn));
    // Create layers
    self->layer1 = Conv2DLayer_create(layer1Dim, layer1Pad);
    self->layer2 = Conv2DLayer_create(layer2Dim, layer2Pad);

    self->inputDim = inputDim;
    // TODO: calculate
    self->outputDim = (Dim2D){1u, 256u};
    return self;
}

void Cnn_destroy(CnnHandle self) {
    Conv2DLayer_destroy(self->layer1);
    Conv2DLayer_destroy(self->layer2);
    free(self);
}

void Cnn_forward(CnnHandle self, fxp32* input, fxp32* output) {
    fxp32* layer1Output = (fxp32*)calloc(self->inputDim.x * self->inputDim.y, sizeof(fxp32));
    Conv2DLayer_forwardFxp(self->layer1, self->inputDim, input, layer1Output);
    Conv2DLayer_forwardFxp(self->layer2, self->inputDim, layer1Output, output);
    free(layer1Output);
}

void Cnn_predict(CnnHandle self, fxp32* acc, fxp32* ppg, fxp32* output) {
    Cnn_forward(self, acc, output);
    for (int i = 0; i < self->outputDim.y * self->outputDim.x; ++i) {
        output[i] = ppg[i] - output[i];
    }
}