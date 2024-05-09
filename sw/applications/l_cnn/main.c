#include "cnn.h"
#include "fxp32.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "test_cnnWeights.h"
#include "testdata_s2.h"

#define COMP_PREC 0.01f
#define COMP_PREC_I32 512

void compareVectorsFloat(float* a, float* b, int size, float prec) {
    for (int i = 0; i < size; ++i) {
        assert_closef(a[i], b[i], prec, i);
    }
}

void compareVectorsFxp(fxp32* a, fxp32* b, int size, int32_t prec) {
    for (int i = 0; i < size; ++i) {
        // printf("b[%d] address: %d\n", i, &b[i]);
        assert_closei32(a[i], b[i], prec, i);
    }
}

void test_same_layer1() {
    // fxp32* result_fxp = (fxp32*)calloc(xin1*yin1, sizeof(fxp32));
    float* result = (float*)calloc(xin1*yin1, sizeof(float));

    // convolve2DFxp(input1_fxp, result_fxp, kernel1_fxp, xin1, yin1, xker1, yker1, false);
    convolve2DFloat(input1, result, kernel1, xin1, yin1, xker1, yker1, false);

    // compareVectorsFxp(result1_fxp, result_fxp, xin1*yin1, COMP_PREC_I32);
    compareVectorsFloat(result1, result, xin1*yin1, COMP_PREC);

    // free(result_fxp);
    free(result);
}

void test_same_layer2() {
    // fxp32* result_fxp = (fxp32*)calloc(xin2*yin2, sizeof(fxp32));
    float* result = (float*)calloc(xin2*yin2, sizeof(float));

    // convolve2DFxp(input2_fxp, result_fxp, kernel2_fxp, xin2, yin2, xker2, yker2, false);
    convolve2DFloat(input2, result, kernel2, xin2, yin2, xker2, yker2, false);
    
    // compareVectorsFxp(result2_fxp, result_fxp, xin2*yin2, COMP_PREC_I32);
    compareVectorsFloat(result2, result, xin2*yin2, COMP_PREC);

    // free(result_fxp);
    free(result);
}

void test_same_layer3() {
    // fxp32* result_fxp = (fxp32*)calloc(xin3*yin3, sizeof(fxp32));
    float* result = (float*)calloc(xin3*yin3, sizeof(float));


    // convolve2DFxp(input3_fxp, result_fxp, kernel3_fxp, xin3, yin3, xker3, yker3, false);
    convolve2DFloat(input3, result, kernel3, xin3, yin3, xker3, yker3, false);

    // compareVectorsFxp(result3_fxp, result_fxp, xin3*yin3, COMP_PREC_I32);
    compareVectorsFloat(result3, result, xin3*yin3, COMP_PREC);

    // free(result_fxp);
    free(result);
}

void test_valid_layer1() {
    fxp32* result_fxp = (fxp32*)calloc(yout4*yout4, sizeof(fxp32));
    float* result = (float*)calloc(xout4*yout4, sizeof(float));

    convolve2DFxp(input4_fxp, result_fxp, kernel4_fxp, xin4, yin4, xker4, yker4, true);
    convolve2DFloat(input4, result, kernel4, xin4, yin4, xker4, yker4, true);

    compareVectorsFxp(result4_fxp, result_fxp, xout4*yout4, COMP_PREC_I32);
    compareVectorsFloat(result4, result, xout4*yout4, COMP_PREC);

    free(result_fxp);
    free(result);
}

void test_valid_layer2() {
    fxp32* result_fxp = (fxp32*)calloc(xout5*yout5, sizeof(fxp32));
    float* result = (float*)calloc(xout5*yout5, sizeof(float));

    convolve2DFxp(input5_fxp, result_fxp, kernel5_fxp, xin5, yin5, xker5, yker5, true);
    convolve2DFloat(input5, result, kernel5, xin5, yin5, xker5, yker5, true);

    compareVectorsFxp(result5_fxp, result_fxp, xout5*yout5, COMP_PREC_I32);
    compareVectorsFloat(result5, result, xout5*yout5, COMP_PREC);

    free(result_fxp);
    free(result);
}

void test_valid_layer3() {
    fxp32* result_fxp = (fxp32*)calloc(xout6*yout6, sizeof(fxp32));
    float* result = (float*)calloc(xout6*yout6, sizeof(float));

    convolve2DFxp(input6_fxp, result_fxp, kernel6_fxp, xin6, yin6, xker6, yker6, true);
    convolve2DFloat(input6, result, kernel6, xin6, yin6, xker6, yker6, true);

    compareVectorsFxp(result6_fxp, result_fxp, xout6*yout6, COMP_PREC_I32);
    compareVectorsFloat(result6, result, xout6*yout6, COMP_PREC);

    free(result_fxp);
    free(result);
}

void test_cnn() {
    CnnHandle cnn = Cnn_create((Dim2D){3u, 15u}, (Dim2D){3u, 5u}, (Dim2D){3u, 1u}, SAME, VALID);

    Conv2DLayer_setWeightsFloat(cnn->layer1, ke1);
    Conv2DLayer_setWeightsFloat(cnn->layer2, ke2);
    Conv2DLayer_setWeightsFxp(cnn->layer1, ke1_fxp);
    Conv2DLayer_setWeightsFxp(cnn->layer2, ke2_fxp);

    float* result = (float*)calloc(1*15, sizeof(float));
    // fxp32* result_fxp = (fxp32*)calloc(1*15, sizeof(fxp32));

    Cnn_forwardFloat(cnn, inp, result);
    // Cnn_forwardFxp(cnn, inp_fxp, result_fxp);

    compareVectorsFloat(result, res, 15, COMP_PREC*10);
    // compareVectorsFxp(result_fxp, res_fxp, 15, COMP_PREC_I32*10);

    free(result);
    // free(result_fxp);
    Cnn_destroy(cnn);
}

void compare_tf_conv_3() {
    CnnHandle cnn = Cnn_create((Dim2D){3u, 256u}, (Dim2D){3u, 21u}, (Dim2D){3u, 1u}, SAME, VALID);

    Conv2DLayer_setWeightsFloat(cnn->layer1, weights1_2);
    Conv2DLayer_setWeightsFloat(cnn->layer2, weights2_2);

    float* result = (float*)calloc(1*256, sizeof(float));
    Cnn_forwardFloat(cnn, xin_2, result);

    compareVectorsFloat(result, xout_2, 256, COMP_PREC);

    Cnn_predictFloat(cnn, xin_2, ppg_2, result);

    compareVectorsFloat(result, ppgf_2, 256, COMP_PREC);

    free(result);

    // Cnn_freezeModel(cnn);

    // fxp32* result_fxp = (fxp32*)calloc(1*256, sizeof(fxp32));
    // Cnn_forwardFxp(cnn, xin_2_fxp, result_fxp);

    // compareVectorsFxp(result_fxp, xout_2_fxp, 256, COMP_PREC_I32);

    // Cnn_predictFxp(cnn, xin_2_fxp, ppg_2_fxp, result_fxp);

    // compareVectorsFxp(result_fxp, ppgf_2_fxp, 256, COMP_PREC_I32);

    // free(result_fxp);
    Cnn_destroy(cnn);
}

int main() {
    PRINTF("\033[1;93m====== Test CNN =========\n");
    PRINTF("\033[0m====== Test Same ========\n");
    test_same_layer1();
    PRINTF("\033[1;32m====== Test 1 passed ====\n");
    test_same_layer2();
    PRINTF("\033[1;32m====== Test 2 passed ====\n");
    test_same_layer3();
    PRINTF("\033[1;32m====== Test 3 passed ====\n");
    PRINTF("\033[0m====== Test Same end ====\n\n");
    PRINTF("\033[0m====== Test Valid =======\n");
    test_valid_layer1();
    PRINTF("\033[1;32m====== Test 1 passed ====\n");
    test_valid_layer2();
    PRINTF("\033[1;32m====== Test 2 passed ====\n");
    test_valid_layer3();
    PRINTF("\033[1;32m====== Test 3 passed ====\n");
    PRINTF("\033[0m====== Test Valid end ===\n\n");
    PRINTF("\033[0m====== Test CNN =========\n");
    test_cnn();
    PRINTF("\033[1;32m====== Test CNN passed ==\n");
    PRINTF("\033[0m====== Test CNN end =====\n\n");
    return EXIT_SUCCESS;
    PRINTF("\033[0m====== Comp TF ==========\n");
    PRINTF("\033[0m====== Sample 3 =========\n");
    // compare_tf_conv_3();
    PRINTF("\033[1;32m====== Comp TF passed ===\n");
    PRINTF("\033[0m====== Comp TF end ======\n");
    return EXIT_SUCCESS;
}