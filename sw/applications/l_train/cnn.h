/*!***********************************************************************************
 * @file     cnn.h
 * @author 	 Linus Crugnola	<linus.crugnola@epfl.ch>
 * @date     07.06.2024
 *
 * Description:
 *  This file contains the definition of a CNN model with two convolutional layers.
 *
 *************************************************************************************/

#ifndef CNN_H
#define CNN_H

#include "config-cnn.h"

#include "conv2dlayer.h"
#include "fxp32.h"

/**
 * @brief a two layer cnn model
 *
 * @param layer1 the first convolutional layer
 * @param layer2 the second convolutional layer
 * @param inputDim the input dimension
 * @param outputDim the output dimension
 * @param fftBits the number of bits for the fft (i.e. the log2 of the outputDim.y
 * @param learningRate the learning rate for the model
 * @param momentum the momentum for the model
 */
typedef struct __Cnn {
    Conv2DLayerHandle layer1;
    Conv2DLayerHandle layer2;
    Dim2D inputDim;
    Dim2D outputDim;
    uint8_t fftBits;
    float learningRate;
    float momentum;
} Cnn;

typedef struct __Cnn* CnnHandle;

/**
 * @brief a complex fixed point number
 *
 * @param r the real part of the number
 * @param i the imaginary part of the
 *
 * @note used for compatibility with the sylt-fft library
 */
typedef struct __complex_t {
    int32_t r;
    int32_t i;
} complex_t;

/**
 * @brief constructor and destructor for the model in case of dynamic allocation
 */
#ifdef DYN_ALLOCATION
/**
 * @brief create a new model and corresponding layers
 *
 * @param inputDim the input dimension
 * @param layer1Dim the dimension of the first layer
 * @param layer2Dim the dimension of the second layer
 * @param outputDim the output dimension
 * @param layer1Pad the padding of the first layer
 * @param layer2Pad the padding of the second layer
 * @param fftBits the number of bits for the fft
 * @param learningRate the learning rate for the model
 * @param momentum the momentum for the model
 *
 * @return the new model
 */
CnnHandle Cnn_create(Dim2D inputDim, Dim2D layer1Dim, Dim2D layer2Dim, Dim2D outputDim, Conv2DPadding layer1Pad,
                     Conv2DPadding layer2Pad, uint8_t fftBits, float learningRate, float momentum);

/**
 * @brief destroy the model
 *
 * @param self the model
 */
void Cnn_destroy(CnnHandle self);
#endif

/**
 * @brief forward pass of the model for float and fixed point data
 *
 * @param self the model
 * @param input the input data, expected size: self->inputDim.x * self->inputDim.y
 */
void Cnn_forwardFxp(CnnHandle self, fxp32* input, fxp32* output);
void Cnn_forwardFloat(CnnHandle self, float* input, float* output);

/**
 * @brief predict the output of the model for float and fixed point data
 *
 * @param self the model
 * @param acc the accelerometer data, expected size: self->inputDim.x * self->inputDim.y
 * @param ppg the ppg data, expected size: self->outputDim.x * self->outputDim.y
 * @param output the output data, expected size: self->outputDim.x * self->outputDim.y
 */
void Cnn_predictFxp(CnnHandle self, fxp32* acc, fxp32* ppg, fxp32* output);
void Cnn_predictFloat(CnnHandle self, float* acc, float* ppg, float* output);

/**
 * @brief train the model
 *
 * @param self the model
 * @param acc the accelerometer data, expected size: self->inputDim.x * self->inputDim.y
 * @param ppg the ppg data, expected size: self->outputDim.x * self->outputDim.y
 * @param nEpochs the number of epochs to train the model
 * @param logAllLosses whether to log all the losses or just the first and last one
 *
 * @note the model is trained using SGD with momentum
 * @note for now the model is trained using only one sample of acc and ppg data
 */
void Cnn_train(CnnHandle self, float* acc, float* ppg, int nEpochs, bool logAllLosses);

/**
 * @brief freeze the model, converts all weights to fixed point in order to
 *        use the Cnn_forwardFxp function for faster forward passing
 *
 * @param self the model
 */
void Cnn_freezeModel(CnnHandle self);

/**
 * @brief calculate the loss of the model for a sample
 *
 * @remark exported for testability, function used in Cnn_train
 *
 * @param self the model
 * @param ypredfft the fft of the predicted output of the model
 *                 expected size: self->outputDim.x * self->outputDim.y
 * @param ytruefft the fft of the true output of the model
 *                 expected size: self->outputDim.x * self->outputDim.y
 *
 * @return the loss of the model
 */
float Cnn_sampleLoss(CnnHandle self, complex_t* ypredfft, complex_t* ytruefft);

#endif // CNN_H