#ifndef CNN_H
#define CNN_H

#include "conv2dlayer.h"
#include "fxp32.h"

// TODO: perhaps we could optimize but we need 256 as value...
/**
 * @brief a 2D dimension
 * @param x the rows of a matrix
 * @param y the columns of a matrix
 */
typedef struct __Dim2D {
    uint16_t x;
    uint16_t y;
} Dim2D;

typedef enum __Conv2DPadding {
    VALID,
    SAME
} Conv2DPadding;

/**
 * @brief a two layer cnn model
 */
typedef struct __Cnn {
    Dim2D inputDim;
    Dim2D outputDim;
    Dim2D layer1Dim;
    Dim2D layer2Dim;
    Conv2DPadding layer1Pad;
    Conv2DPadding layer2Pad;
    float* layer1Weights;
    float* layer2Weights;
} Cnn;

typedef struct __Cnn* CnnHandle;

typedef struct __complex_t {
    int32_t r;
    int32_t i;
} complex_t;

CnnHandle Cnn_create(Dim2D inputDim, Dim2D layer1Dim, Dim2D layer2Dim, Conv2DPadding layer1Pad,
                     Conv2DPadding layer2Pad);
void Cnn_destroy(CnnHandle self);

// void Cnn_forwardFxp(CnnHandle self, fxp32* input, fxp32* output);
void Cnn_forwardFloat(CnnHandle self, float* input, float* output);

void Cnn_setWeights1Float(CnnHandle self, float* weights);
void Cnn_setWeights2Float(CnnHandle self, float* weights);
// void Cnn_setWeights1Fxp(CnnHandle self, fxp32* weights);
// void Cnn_setWeights2Fxp(CnnHandle self, fxp32* weights);

void Cnn_predictFxp(CnnHandle self, fxp32* acc, fxp32* ppg, fxp32* output);
void Cnn_predictFloat(CnnHandle self, float* acc, float* ppg, float* output);

float Cnn_sampleLoss(CnnHandle self, complex_t* ypred, complex_t* ytrue);

void Cnn_freezeModel(CnnHandle self);

#endif // CNN_H