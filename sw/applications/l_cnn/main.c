#include "cnn.h"
#include "fxp32.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "test_cnnWeights.h"

#define COMP_PREC 0.01f
#define COMP_PREC_I32 512

void compareVectorsFloat(float* a, float* b, int size, float prec) {
    for (int i = 0; i < size; ++i) {
        assert_closef(a[i], b[i], prec, i);
    }
}

void compareVectorsFxp(fxp32* a, fxp32* b, int size, int32_t prec) {
    for (int i = 0; i < size; ++i) {
        assert_closei32(a[i], b[i], prec, i);
    }
}

void test_cnn() {
    Dim2D inputDim = {3u, 15u};
    Dim2D layer1Dim = {3u, 5u};
    Dim2D layer2Dim = {3u, 1u};
    CnnHandle cnn = Cnn_create(inputDim, layer1Dim, layer2Dim, SAME, VALID);

    Cnn_setWeights1Float(cnn, ke1);
    Cnn_setWeights2Float(cnn, ke2);
    // Conv2DLayer_setWeightsFxp(cnn->layer1, ke1_fxp);
    // Conv2DLayer_setWeightsFxp(cnn->layer2, ke2_fxp);

    float* result = (float*)my_calloc(15, sizeof(float));
    float* inter = (float*)my_malloc(2*15* sizeof(float));
    // fxp32* result_fxp = (fxp32*)my_calloc(1*15, sizeof(fxp32));

    Cnn_forwardFloat(cnn, inp, result);
    for (int i = 0; i < 15; ++i) {
        printf("layer2 output[%d] = %d\n", i, (int)(100000*result[i]));
    }
    // Cnn_forwardFxp(cnn, inp_fxp, result_fxp);

    compareVectorsFloat(result, res, 15, COMP_PREC*10);
    // compareVectorsFxp(result_fxp, res_fxp, 15, COMP_PREC_I32*10);

    free(result);
    // free(result_fxp);
    Cnn_destroy(cnn);
}

int main(int argc, char *argv[]) {
    PRINTF("\033[0m====== Test CNN =========\n");
    test_cnn();
    PRINTF("\033[1;32m====== Test CNN passed ==\n");
    PRINTF("\033[0m====== Test CNN end =====\n\n");
    return EXIT_SUCCESS;
}