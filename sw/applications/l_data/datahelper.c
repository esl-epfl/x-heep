#include "datahelper.h"

#include <stdio.h>

DataMS Data_normalizeAccPpg(fxp32* accx, fxp32* accy, fxp32* accz, fxp32* ppg, int size) {
    DataMS mv = Data_normalizeMV(ppg, size);
    Data_normalize(accx, size);
    Data_normalize(accy, size);
    Data_normalize(accz, size);
    return mv;
}

DataMSf Data_normalizeAccPpgf(float* accx, float* accy, float accz, float* ppg, int size) {
    DataMSf mv = Data_normalizeMVf(ppg, size);
    Data_normalizef(accx, size);
    Data_normalizef(accy, size);
    accz = (accz - mv.mean) / mv.std;
    return mv;
}

bool closef(float a, float b, float prec) {
    float diff = a - b;
    if (diff < 0)
        diff = -diff;
    return diff <= prec;
}

float sqrtf(float x) {
    float guess = x / 2;
    float eps = 0.00001;
    while (closef(guess, x / guess, eps) == false) {
        guess = (guess + x / guess) / 2;
    }
    return guess;
}

// FIXME:
DataMS Data_normalizeMV(fxp32* data, int size) {
    // calculate mean
    DataMS mv;
    fxpMul sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += (fxpMul)data[i];
    }
    mv.mean = (fxp32)((sum << FRACTIONAL_BITS) / fxp32_fromInt(size));

    // calculate variance
    fxpMul std = 0;
    for (int i = 0; i < size; ++i) {
        std += fxp32_mul(data[i] - mv.mean, data[i] - mv.mean);
        printf("data: %f\n", fxp32_toFloat(data[i]));
        printf("data - mean: %f\n", fxp32_toFloat(data[i] - mv.mean));
        printf("std: %f\n", fxp32_toFloat((fxp32)std));
    }
    mv.std = fxp32_sqrt((fxp32)((std << FRACTIONAL_BITS) / fxp32_fromInt(size)));

    // normalize data
    for (int i = 0; i < size; ++i) {
        data[i] = fxp32_div(data[i] - mv.mean, mv.std);
    }

    return mv;
}

DataMSf Data_normalizeMVf(float* data, int size) {
    // calculate mean
    DataMSf mv;
    float sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += data[i];
    }
    mv.mean = sum / size;

    // calculate variance
    float std = 0;
    for (int i = 0; i < size; ++i) {
        std += (data[i] - mv.mean) * (data[i] - mv.mean);
    }
    mv.std = sqrtf(std / size);

    // normalize data
    for (int i = 0; i < size; ++i) {
        data[i] = (data[i] - mv.mean) / mv.std;
    }

    return mv;
}

// FIXME:
void Data_normalize(fxp32* data, int size) {
    // calculate mean
    fxp32 sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += data[i];
    }
    fxp32 mean = fxp32_div(sum, fxp32_fromInt(size));

    // calculate variance
    fxp32 var = 0;
    for (int i = 0; i < size; ++i) {
        var += fxp32_mul(data[i] - mean, data[i] - mean);
    }
    var = fxp32_sqrt(fxp32_div(var, fxp32_fromInt(size)));

    // normalize data
    for (int i = 0; i < size; ++i) {
        data[i] = fxp32_div(data[i] - mean, var);
    }
}

void Data_normalizef(float* data, int size) {
    // calculate mean
    float sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += data[i];
    }
    float mean = sum / size;

    // calculate variance
    float var = 0;
    for (int i = 0; i < size; ++i) {
        var += (data[i] - mean) * (data[i] - mean);
    }
    var = sqrtf(var / size);

    // normalize data
    for (int i = 0; i < size; ++i) {
        data[i] = (data[i] - mean) / var;
    }
}

void Data_denormalize(fxp32* data, int size, DataMS mv) {
    for (int i = 0; i < size; ++i) {
        data[i] = fxp32_mul(data[i], mv.std) + mv.mean;
    }
}

void Data_denormalizef(float* data, int size, DataMSf mv) {
    for (int i = 0; i < size; ++i) {
        data[i] = data[i] * mv.std + mv.mean;
    }
}