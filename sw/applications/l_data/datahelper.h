#ifndef DATAHELPER_H
#define DATAHELPER_H

#include "fxp32.h"

typedef struct DataMS {
    fxp32 mean;
    fxp32 std;
} DataMS;

typedef struct DataMSf {
    float mean;
    float std;
} DataMSf;

/**
 * Normalize data and return the mean and variance of ppg data
 */
DataMS Data_normalizeAccPpg(fxp32* accx, fxp32* accy, fxp32* accz, fxp32* ppg, int size);
DataMSf Data_normalizeAccPpgf(float* accx, float* accy, float accz, float* ppg, int size);

// Helper functions
bool closef(float a, float b, float prec);
float sqrtf(float x);
DataMS Data_normalizeMV(fxp32* data, int size);
void Data_normalize(fxp32* data, int size);
DataMSf Data_normalizeMVf(float* data, int size);
void Data_normalizef(float* data, int size);

/**
 * Denormalize data with given mean and variance
 */
void Data_denormalize(fxp32* data, int size, DataMS mv);
void Data_denormalizef(float* data, int size, DataMSf mv);

#endif // DATAHELPER_H