#include "fxp32.h"
#include "cnn.h"
#include "conv2dlayer.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define COMP_PREC 0.0001f
#define COMP_PREC_I32 512

void floatToFxpVector(float* a, fxp32* b, int size) {
    for (int i = 0; i < size; ++i) {
        b[i] = fxp32_fromFloat(a[i]);
    }
}

void FxpToFloatVector(fxp32* a, float* b, int size) {
    for (int i = 0; i < size; ++i) {
        b[i] = fxp32_toFloat(a[i]);
    }
}

void compareVectorsFloat(float* a, float* b, int size, float prec) {
    for (int i = 0; i < size; ++i) {
        assert_closef_si(a[i], b[i], prec, i);
    }
}

void compareVectorsFxp(fxp32* a, fxp32* b, int size, float prec) {
    for (int i = 0; i < size; ++i) {
        assert_closei32_si(a[i], b[i], prec, i);
    }
}

#include "sample_0_normalized.h"
#include "initial_weights_0.h"

void test_lossTrack1() {

    // Create the CNN
    Conv2DLayer layer1;
    layer1.dim = (Dim2D){3u, 21u};
    layer1.padding = SAME;
    layer1.weightsFloat = weights1;

    Conv2DLayer layer2;
    layer2.dim = (Dim2D){3u, 1u};
    
    layer2.padding = VALID;
    layer2.weightsFloat = weights2;

    Cnn cnn;
    cnn.layer1 = &layer1;
    cnn.layer2 = &layer2;
    cnn.inputDim = (Dim2D){3u, 256u};
    cnn.outputDim = (Dim2D){1u, 256u};
    cnn.fftBits = 8;
    cnn.learningRate = 1e-7f;
    cnn.momentum = 1e-2f;

    // Forward pass
    Cnn_train(&cnn, xin_0, ppg_0, 100, true);
}

int main() {
    PRINTF("\033[1;93m====== Test Train ========\n");
    PRINTF("\033[0m====== Test Weights 1 ====\n");
    test_lossTrack1();
    PRINTF("\033[1;32m====== Test passed =======\n");
    PRINTF("\033[0m====== Test Train end ====\n\n");
    return EXIT_SUCCESS;
}