/*!***********************************************************************************
 * @file     cnn.c
 * @author 	 Linus Crugnola	<linus.crugnola@epfl.ch>
 * @date     07.06.2024
 *
 * Description:
 *  This file contains the implementation of a CNN model with two convolutional layers.
 *
 *************************************************************************************/

// Include order matters here for the sytl-fft library
// clang-format off
#include "sylt-fft/fft.h"

#include "cnn.h"

#include "coremark.h"

#include <stdlib.h>
#include <stdio.h>
// clang-format on

// Private variables of the CNN
static complex_t ppgfft[256];
static complex_t outputfft[256];

static float forwardPass[256];
static float layer1Output[3 * 256];

static complex_t gradGToW2[256];
static float gradLToM[3 * 256];
static float w1Velocity[3 * 21] = {0.0f};
static float w2Velocity[3] = {0.0f};

#ifndef DYN_ALLOCATION
static fxp32 layer1OutputFxp[3 * 256];
#endif

#ifdef DYN_ALLOCATION
CnnHandle Cnn_create(Dim2D inputDim, Dim2D layer1Dim, Dim2D layer2Dim, Dim2D outputDim, Conv2DPadding layer1Pad,
                     Conv2DPadding layer2Pad, uint8_t fftBits, float learningRate, float momentum) {
    CnnHandle self = (CnnHandle)malloc(sizeof(Cnn));
    // Create layers
    self->layer1 = Conv2DLayer_create(layer1Dim, layer1Pad);
    self->layer2 = Conv2DLayer_create(layer2Dim, layer2Pad);

    self->inputDim = inputDim;
    self->outputDim = outputDim;
    self->fftBits = fftBits;
    self->learningRate = learningRate;
    self->momentum = momentum;

    return self;
}

void Cnn_destroy(CnnHandle self) {
    Conv2DLayer_destroy(self->layer1);
    Conv2DLayer_destroy(self->layer2);
    free(self);
}
#endif

void Cnn_forwardFxp(CnnHandle self, fxp32* input, fxp32* output) {
#ifdef DYN_ALLOCATION
    fxp32* layer1Output = (fxp32*)calloc(self->inputDim.x * self->inputDim.y, sizeof(fxp32));
    Conv2DLayer_forwardFxp(self->layer1, self->inputDim, input, layer1Output);
    Conv2DLayer_forwardFxp(self->layer2, self->inputDim, layer1Output, output);
    free(layer1Output);
#else
    Conv2DLayer_forwardFxp(self->layer1, self->inputDim, input, layer1OutputFxp);
    Conv2DLayer_forwardFxp(self->layer2, self->inputDim, layer1OutputFxp, output);
#endif
}

void Cnn_forwardFloat(CnnHandle self, float* input, float* output) {
    Conv2DLayer_forwardFloat(self->layer1, self->inputDim, input, layer1Output);
    Conv2DLayer_forwardFloat(self->layer2, self->inputDim, layer1Output, output);
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

float Cnn_sampleLoss(CnnHandle self, complex_t* ypredfft, complex_t* ytruefft) {
    float loss = 0.0f;
    float ytr, ypr, yti, ypi;
    for (int i = 0; i < self->outputDim.y * self->outputDim.x; ++i) {
        ytr = fxp32_toFloat(ytruefft[i].r);
        ypr = fxp32_toFloat(ypredfft[i].r);
        yti = fxp32_toFloat(ytruefft[i].i);
        ypi = fxp32_toFloat(ypredfft[i].i);
        loss += (ytr - ypr) * (ytr - ypr) + (yti - ypi) * (yti - ypi);
    }
    return loss;
}

void fft(complex_t* cpx, int8_t bits) {
    fft_fft((fft_complex_t*)cpx, bits);
    for (int i = 0; i < (1 << bits); i++) {
        cpx[i].r <<= bits;
        cpx[i].i <<= bits;
    }
}

void arrToComplex(float* arr, complex_t* cpx, int size, int offset) {
    for (int i = 0; i < size; i++) {
        cpx[i].r = fxp32_fromFloat(arr[i + offset]);
        cpx[i].i = 0;
    }
}

void arrOnesComplex(complex_t* cpx, int size) {
    for (int i = 0; i < size; i++) {
        cpx[i].r = fxp32_fromFloat(1.0f);
        cpx[i].i = 0;
    }
}

void Cnn_sgdStep(CnnHandle self, float* acc, float* ppg) {
    // Calculate the intermediate function g_i and store its conj in outputfft
    for (int i = 0; i < self->outputDim.y * self->outputDim.x; ++i) {
        outputfft[i].r = ppgfft[i].r - outputfft[i].r;
        outputfft[i].i = outputfft[i].i - ppgfft[i].i;
    }
    // Calculate the gradient w.r to w_2
    float dL_dw2[self->layer2->dim.x * self->layer2->dim.y];
    for (int i = 0; i < self->layer2->dim.x * self->layer2->dim.y; ++i) {
        // Calculate the dg_i/dw_2,i
        arrToComplex(layer1Output, gradGToW2, self->inputDim.y, i * self->inputDim.y);
        fft(gradGToW2, self->fftBits);
        // calculate the product and create sum
        dL_dw2[i] = 0.0f;
        for (int j = 0; j < self->inputDim.y; ++j) {
            dL_dw2[i] += 2 * (fxp32_toFloat(outputfft[j].r) * fxp32_toFloat(gradGToW2[j].r) -
                              fxp32_toFloat(outputfft[j].i) * fxp32_toFloat(gradGToW2[j].i));
        }
    }
    // Subtract update velocity and second layer weights
    for (int i = 0; i < self->layer2->dim.x * self->layer2->dim.y; ++i) {
        w2Velocity[i] = self->momentum * w2Velocity[i] - dL_dw2[i] * self->learningRate;
        self->layer2->weightsFloat[i] += w2Velocity[i];
    }
#ifdef LOG_GRADIENTS
    for (int i = 0; i < self->layer2->dim.x * self->layer2->dim.y; ++i) {
        printf("dL_dw2[%d]: %f\n", i, w2Velocity[i]);
    }
#endif

    // get ppg-output in time domain and store in output
    for (int i = 0; i < self->outputDim.y * self->outputDim.x; i++) {
        forwardPass[i] = ppg[i] - forwardPass[i];
    }

    // fill the values in the dL/dm matrix
    for (int i = 0; i < self->inputDim.x; i++) {
        float factor = -512 * self->layer2->weightsFloat[i];
        for (int j = 0; j < self->inputDim.y; j++) {
            gradLToM[i * self->inputDim.y + j] = forwardPass[j] * factor;
        }
    }

    // Calculate the gradient from m to w_1
    float dL_dw1[self->layer1->dim.x * self->layer1->dim.y];
    for (int i = 0; i < self->layer1->dim.x * self->layer1->dim.y; ++i) {
        dL_dw1[i] = 0.0f;
    }
    int cx = self->layer1->dim.x / 2;
    int cy = self->layer1->dim.y / 2;
    for (int i = 0; i < self->inputDim.x; ++i) {
        for (int j = 0; j < self->inputDim.y; ++j) {
            // For every item in dL/dm add the contribution to every weight
            for (int m = 0; m < self->layer1->dim.x; ++m) {
                for (int n = 0; n < self->layer1->dim.y; ++n) {
                    float curAcc = 0.0f;
                    if (m + i - cx >= 0 && m + i - cx < self->inputDim.x && n + j - cy >= 0 &&
                        n + j - cy < self->inputDim.y) {
                        curAcc = acc[(m + i - cx) * self->inputDim.y + n + j - cy];
                    }
                    dL_dw1[m * self->layer1->dim.y + n] += gradLToM[i * self->inputDim.y + j] * curAcc;
                }
            }
        }
    }
    // Update the velocity and first layer weights
    for (int i = 0; i < self->layer1->dim.x * self->layer1->dim.y; ++i) {
        w1Velocity[i] = self->momentum * w1Velocity[i] - dL_dw1[i] * self->learningRate;
        self->layer1->weightsFloat[i] += w1Velocity[i];
    }
#ifdef LOG_GRADIENTS
    for (int i = 0; i < self->layer1->dim.x * self->layer1->dim.y; ++i) {
        printf("dL_dw1[%d]: %f\n", i, w1Velocity[i]);
    }
#endif
}

// TODO: Implement batch processing for more efficient training
void Cnn_train(CnnHandle self, float* acc, float* ppg, int nEpochs, bool logAllLosses) {
    // FFT of the ppg
    arrToComplex(ppg, ppgfft, self->outputDim.y * self->outputDim.x, 0);
    fft(ppgfft, self->fftBits);
    for (int i = 0; i < nEpochs; ++i) {
        start_time();
        Cnn_forwardFloat(self, acc, forwardPass);
        arrToComplex(forwardPass, outputfft, self->outputDim.y * self->outputDim.x, 0);
        fft(outputfft, self->fftBits);
// #ifdef TRACK_LOSS
//         if (logAllLosses)
//             printf("loss epoch %d: %d\n", i, (int)Cnn_sampleLoss(self, outputfft, ppgfft));
//         else if (i == 0 || i == nEpochs - 1)
//             printf("loss epoch %d: %d\n", i + 1, (int)Cnn_sampleLoss(self, outputfft, ppgfft));
// #endif
        Cnn_sgdStep(self, acc, ppg);
        stop_time();
        printf("Time epoch %d: %d\n", i+1, get_time());
    }
}

void Cnn_freezeModel(CnnHandle self) {
    Conv2DLayer_transformWeightsToFxp(self->layer1);
    Conv2DLayer_transformWeightsToFxp(self->layer2);
}