/*!***********************************************************************************
 * @file     conv2dlayer.h
 * @author 	 Linus Crugnola	<linus.crugnola@epfl.ch>
 * @date     07.06.2024
 *
 * Description:
 *  This file contains the definition of a two dimensional convolutional layer.
 *
 *************************************************************************************/

#ifndef CONV2DLAYER_H
#define CONV2DLAYER_H

#include "config-cnn.h"
#include <stdbool.h>
#include <stdint.h>

#include "fxp32.h"

/**
 * @brief a 2D dimension
 *
 * @param x the rows of a matrix
 * @param y the columns of a matrix
 */
typedef struct __Dim2D {
    uint16_t x;
    uint16_t y;
} Dim2D;

/**
 * @brief a padding type for the convolutional layer
 */
typedef enum __Conv2DPadding {
    VALID,
    SAME
} Conv2DPadding;

/**
 * @brief a convolutional layer
 *
 * @param dim the dimension of the layers kernel
 * @param padding the padding type
 * @param weightsFxp the weights of the layer in fixed point
 * @param weightsFloat the weights of the layer in float
 */
typedef struct __Conv2DLayer {
    Dim2D dim;
    Conv2DPadding padding;
    fxp32* weightsFxp;
    float* weightsFloat;
} Conv2DLayer;

typedef struct __Conv2DLayer* Conv2DLayerHandle;

/**
 * @brief constructor and destructor for the layer in case of dynamic allocation
 */
#ifdef DYN_ALLOCATION
/**
 * @brief create a new convolutional layer
 *
 * @param dim the dimension layers kernel
 * @param padding the padding type
 *
 * @return the new layer
 */
Conv2DLayerHandle Conv2DLayer_create(Dim2D dim, Conv2DPadding padding);

/**
 * @brief destroy the convolutional layer
 *
 * @param self the layer to destroy
 */
void Conv2DLayer_destroy(Conv2DLayerHandle self);
#endif

/**
 * @brief set the weights of the layer
 *
 * @param self the layer
 * @param weights the weights to set, expected size: self->dim.x * self->dim.y
 */
void Conv2DLayer_setWeightsFxp(Conv2DLayerHandle self, fxp32* weights);
void Conv2DLayer_setWeightsFloat(Conv2DLayerHandle self, float* weights);

/**
 * @brief transform the weights of the layer to fixed point
 *
 * @param self the layer
 */
void Conv2DLayer_transformWeightsToFxp(Conv2DLayerHandle self);

/**
 * @brief forward pass of the layer
 *
 * @param self the layer
 * @param inputDim the input dimension
 * @param input the input data, expected size: inputDim.x * inputDim.y
 * @param output the output data, size is expected to match padding and inputDim
 */
void Conv2DLayer_forwardFxp(Conv2DLayerHandle self, Dim2D inputDim, fxp32* input, fxp32* output);
void Conv2DLayer_forwardFloat(Conv2DLayerHandle self, Dim2D inputDim, float* input, float* output);

/**
 * @brief 2D convolution of fixed point and float data
 *
 * @remark exported for testability, functions used in the forward pass of the model above
 *
 * @param input the input data
 * @param output the output data
 * @param kernel the kernel
 * @param inx the rows of the input data
 * @param iny the columns of the input data
 * @param kerx the rows of the kernel
 * @param kery the columns of the kernel
 * @param valid the padding type
 *
 * @throw EXIT_FAILURE if the kernel dimensions are not odd
 */
void convolve2DFxp(fxp32* input, fxp32* output, fxp32* kernel, int inx, int iny, int kerx, int kery, bool valid);
void convolve2DFloat(float* input, float* output, float* kernel, int inx, int iny, int kerx, int kery, bool valid);

#endif // CONV2DLAYER_H