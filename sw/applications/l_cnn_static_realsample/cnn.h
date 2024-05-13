#ifndef CNN_H
#define CNN_H

#include "conv2dlayer.h"
#include "fxp32.h"

/**
 * @brief a two layer cnn model
 */
typedef struct __Cnn {
    Conv2DLayerHandle layer1;
    Conv2DLayerHandle layer2;
    Dim2D inputDim;
    Dim2D outputDim;
#ifndef DYN_ALLOCATION
    float* layer1Output;
    fxp32* layer1OutputFxp;
#endif
} Cnn;

typedef struct __Cnn* CnnHandle;

typedef struct __complex_t {
    int32_t r;
    int32_t i;
} complex_t;

// CnnHandle Cnn_create(Dim2D inputDim, Dim2D layer1Dim, Dim2D layer2Dim, Conv2DPadding layer1Pad,
//                      Conv2DPadding layer2Pad);
// void Cnn_destroy(CnnHandle self);

void Cnn_forwardFxp(CnnHandle self, fxp32* input, fxp32* output);
void Cnn_forwardFloat(CnnHandle self, float* input, float* output);

void Cnn_predictFxp(CnnHandle self, fxp32* acc, fxp32* ppg, fxp32* output);
void Cnn_predictFloat(CnnHandle self, float* acc, float* ppg, float* output);

// float Cnn_sampleLoss(CnnHandle self, complex_t* ypred, complex_t* ytrue);

void Cnn_freezeModel(CnnHandle self);

#endif // CNN_H