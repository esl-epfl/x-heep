#include "fxp32.h"
#include "cnn.h"
#include "conv2dlayer.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "testdata_learn.h"

#define COMP_PREC 0.000001f

#ifndef DYN_ALLOCATION
complex_t ypred[256];
complex_t ytrue[256];
#endif

void test_learn_sampleLoss() {
#ifdef DYN_ALLOCATION
    complex_t* ypred = (complex_t*)calloc(256, sizeof(complex_t));
    complex_t* ytrue = (complex_t*)calloc(256, sizeof(complex_t));
#endif
    for (int i = 0; i < 256; ++i) {
        ypred[i].r = fxp32_fromFloat(arp[i]);
        ypred[i].i = fxp32_fromFloat(aip[i]);
        ytrue[i].r = fxp32_fromFloat(art[i]);
        ytrue[i].i = fxp32_fromFloat(ait[i]);
    }
    Cnn cnn;
    cnn.outputDim = (Dim2D){1u, 256u};    
    float loss = Cnn_sampleLoss(&cnn, ypred, ytrue);
    PRINTF("Loss:         %d\n", (int)(loss*SCL));
    PRINTF("Expected:     %d\n", (int)(ares*SCL));
    PRINTF("Diff:         %d\n", (int)((ares - loss)*SCL));
    assert_closef(loss, ares, 0.005f);
#ifdef DYN_ALLOCATION
    free(ypred);
    free(ytrue);
#endif
}

void test_learn_sampleLoss_2() {
#ifdef DYN_ALLOCATION
    complex_t* ypred = (complex_t*)calloc(256, sizeof(complex_t));
    complex_t* ytrue = (complex_t*)calloc(256, sizeof(complex_t));
#endif
    for (int i = 0; i < 256; ++i) {
        ypred[i].r = fxp32_fromFloat(arp_2[i]);
        ypred[i].i = fxp32_fromFloat(aip_2[i]);
        ytrue[i].r = fxp32_fromFloat(art_2[i]);
        ytrue[i].i = fxp32_fromFloat(ait_2[i]);
    }
    Cnn cnn;
    cnn.outputDim = (Dim2D){1u, 256u};
    float loss = Cnn_sampleLoss(&cnn, ypred, ytrue);
    PRINTF("Loss:         %d\n", (int)(loss*SCL));
    PRINTF("Expected:     %d\n", (int)(ares_2*SCL));
    PRINTF("Diff:         %d\n", (int)((ares_2 - loss)*SCL));
    assert_closef(loss, ares_2, 0.005f);
#ifdef DYN_ALLOCATION
    free(ypred);
    free(ytrue);
#endif
}

void test_learn_sampleLoss_3() {
#ifdef DYN_ALLOCATION
    complex_t* ypred = (complex_t*)calloc(256, sizeof(complex_t));
    complex_t* ytrue = (complex_t*)calloc(256, sizeof(complex_t));
#endif
    for (int i = 0; i < 256; ++i) {
        ypred[i].r = fxp32_fromFloat(arp_3[i]);
        ypred[i].i = fxp32_fromFloat(aip_3[i]);
        ytrue[i].r = fxp32_fromFloat(art_3[i]);
        ytrue[i].i = fxp32_fromFloat(ait_3[i]);
    }
    Cnn cnn;
    cnn.outputDim = (Dim2D){1u, 256u};
    float loss = Cnn_sampleLoss(&cnn, ypred, ytrue);
    PRINTF("Loss:         %d\n", (int)(loss*SCL));
    PRINTF("Expected:     %d\n", (int)(ares_3*SCL));
    PRINTF("Diff:         %d\n", (int)((ares_3 - loss)*SCL));
    assert_closef(loss, ares_3, 0.01f);
#ifdef DYN_ALLOCATION
    free(ypred);
    free(ytrue);
#endif
}

void test_learn_sampleLoss_4() {
#ifdef DYN_ALLOCATION
    complex_t* ypred = (complex_t*)calloc(256, sizeof(complex_t));
    complex_t* ytrue = (complex_t*)calloc(256, sizeof(complex_t));
#endif
    for (int i = 0; i < 256; ++i) {
        ypred[i].r = fxp32_fromFloat(arp_4[i]);
        ypred[i].i = fxp32_fromFloat(aip_4[i]);
        ytrue[i].r = fxp32_fromFloat(art_4[i]);
        ytrue[i].i = fxp32_fromFloat(ait_4[i]);
    }
    Cnn cnn;
    cnn.outputDim = (Dim2D){1u, 256u};
    float loss = Cnn_sampleLoss(&cnn, ypred, ytrue);
    PRINTF("Loss:         %d\n", (int)(loss*SCL));
    PRINTF("Expected:     %d\n", (int)(ares_4*SCL));
    PRINTF("Diff:         %d\n", (int)((ares_4 - loss)*SCL));
    assert_closef(loss, ares_4, 0.01f);
#ifdef DYN_ALLOCATION
    free(ypred);
    free(ytrue);
#endif
}

void test_sampleLossFloat() {
    float loss = 0.0f;
    for (int i = 0; i < 256; ++i) {
        loss += (art[i] - arp[i])*(art[i] - arp[i]) + (ait[i] - aip[i])*(ait[i] - aip[i]);
    }
    PRINTF("Loss float:   %d\n", (int)(loss*SCL));
}

void test_sampleLossFloat_2() {
    float loss = 0.0f;
    for (int i = 0; i < 256; ++i) {
        loss += (art_2[i] - arp_2[i])*(art_2[i] - arp_2[i]) + (ait_2[i] - aip_2[i])*(ait_2[i] - aip_2[i]);
    }
    PRINTF("Loss 2 float: %d\n", (int)(loss*SCL));
}

int main() {
    PRINTF("====== Test Learn ========\n");
    PRINTF("====== Test Loss =======\n");
    test_learn_sampleLoss();
    test_sampleLossFloat();
    test_learn_sampleLoss_2();
    test_sampleLossFloat_2();
    test_learn_sampleLoss_3();
    test_learn_sampleLoss_4();
    PRINTF("====== Test passed =======\n");
    PRINTF("====== Test Learn end ====\n\n");
    return EXIT_SUCCESS;
}

/*
Note: Error is about 100x smaller than the last step with 1000 epochs so I think it's fine since we average
TODO: Figure out why the error is consistently a multiple of the same value
*/