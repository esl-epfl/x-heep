/*!***********************************************************************************
 * @file     utils.h
 * @author 	 Linus Crugnola	<linus.crugnola@epfl.ch>
 * @date     07.06.2024
 *
 * Description:
 *  This file contains a number of utility functions for the tests.
 *
 *************************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @def SIMULATION or TARGET for the code to run in verilator or on the pynq board
 */
// #define SIMULATION
#define TARGET

/**
 * @def Scale for printing floats (on the pynq board)
 */
#define SCL 100

/**
 * @def ASSERT_FAIL if assertions should fail the program
 * @note otherwise they will just print a message
 */
// #define ASSERT_FAIL

#ifdef SIMULATION
#pragma message ("SIMULATION environment")
#endif
#ifdef TARGET
#pragma message ("TARGET environment")
#endif

/**
 * @def PRINTF macro to print to the console
 * 
 * @note should be used instead of printf to avoid printing in the simulation environment
 */
#ifndef SIMULATION
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

/**
 * @brief Assert close function for float (closef) and fixed point (closei32) numbers
 * 
 * @param a the first number
 * @param b the second number
 * @param prec the precision
 * @param idx the index of the assertion handy for vectors (optional)
 * 
 * @note The functions ending in _s keep track of the maximum difference found in
 * @note The functions ending in _si does also print the index of the assertion
 */
static float maxdiff = 0;
static int32_t maxdiffxp = 0;

// Assert functions, always print if failing
void assert_closef_si(float a, float b, float prec, int idx) {
    float diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > maxdiff) {
        maxdiff = diff;
        if (maxdiff > 0.0001)
#if defined(SIMULATION) || defined(TARGET)
            PRINTF("Max diff float (scaled x10^6): %d\n", (int)(maxdiff * SCL));
#else
            PRINTF("Max diff float: %f\n", maxdiff);
#endif
    }
    if (diff > prec) {
#if defined(SIMULATION) || defined(TARGET)
        PRINTF("AF (scaled x10^6) %d %d %d %d\n", __LINE__, (int)(a*SCL), (int)(b*SCL), idx);
#else
        PRINTF("AF %d %f %f %d\n", __LINE__, a, b, idx);
#endif
#ifdef ASSERT_FAIL
        exit(EXIT_FAILURE);
#endif
    }
}

void assert_closei32_si(int32_t a, int32_t b, int32_t prec, int idx) {
    int32_t diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > maxdiffxp) {
        maxdiffxp = diff;
        if (maxdiffxp > 1)
            PRINTF("Max diff fxp: %d\n", maxdiffxp);
    }
    if (diff > prec) {
        printf("AI %d %d %d %d\n", __LINE__, a, b, idx);
#ifdef ASSERT_FAIL
        exit(EXIT_FAILURE);
#endif
    }
}

void assert_closef_s(float a, float b, float prec) {
    float diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > maxdiff) {
        maxdiff = diff;
        if (maxdiff > 0.0001)
#if defined(SIMULATION) || defined(TARGET)
            PRINTF("Max diff float (scaled x10^6): %d\n", (int)(maxdiff * SCL));
#else
            PRINTF("Max diff float: %f\n", maxdiff);
#endif
    }
    if (diff > prec) {
#if defined(SIMULATION) || defined(TARGET)
        PRINTF("AF (scaled x10^6) %d %d %d\n", __LINE__, (int)(a*SCL), (int)(b*SCL));
#else
        PRINTF("AF %d %f %f\n", __LINE__, a, b);
#endif
#ifdef ASSERT_FAIL
        exit(EXIT_FAILURE);
#endif
    }
}

void assert_closei32_s(int32_t a, int32_t b, int32_t prec) {
    int32_t diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > maxdiffxp) {
        maxdiffxp = diff;
        if (maxdiffxp > 1)
            PRINTF("Max diff fxp: %d\n", maxdiffxp);
    }
    if (diff > prec) {
        printf("AI %d %d %d\n", __LINE__, a, b);
#ifdef ASSERT_FAIL
        exit(EXIT_FAILURE);
#endif
    }
}

void assert_closef_i(float a, float b, float prec, int idx) {
    float diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > prec) {
#if defined(SIMULATION) || defined(TARGET)
        PRINTF("AF (scaled x10^6) %d %d %d %d\n", __LINE__, (int)(a*SCL), (int)(b*SCL), idx);
#else
        PRINTF("AF %d %f %f %d\n", __LINE__, a, b, idx);
#endif
#ifdef ASSERT_FAIL
        exit(EXIT_FAILURE);
#endif
    }
}

void assert_closei32_i(int32_t a, int32_t b, int32_t prec, int idx) {
    int32_t diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > prec) {
        printf("AI %d %d %d %d\n", __LINE__, a, b, idx);
#ifdef ASSERT_FAIL
        exit(EXIT_FAILURE);
#endif
    }
}

void assert_closef(float a, float b, float prec) {
    float diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > prec) {
#if defined(SIMULATION) || defined(TARGET)
        PRINTF("AF (scaled x10^6) %d %d %d\n", __LINE__, (int)(a*SCL), (int)(b*SCL));
#else
        PRINTF("AF %d %f %f\n", __LINE__, a, b);
#endif
#ifdef ASSERT_FAIL
        exit(EXIT_FAILURE);
#endif
    }
}

void assert_closei32(int32_t a, int32_t b, int32_t prec) {
    int32_t diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > prec) {
        printf("AI %d %d %d\n", __LINE__, a, b);
#ifdef ASSERT_FAIL
        exit(EXIT_FAILURE);
#endif
    }
}

/**
 * @brief Export a vector to a file
 * 
 * @param a the vector to export
 * @param size the size of the vector
 * @param filename the name of the file
 * 
 * @note This function is not available in the target and simulation environment
 */
#if defined(SIMULATION) || defined(TARGET)
#define VECTOR_EXPORT(...)
#else
void vectorExport(float* a, int size, char filename[]) {
    FILE *filePointer;
    filePointer = fopen(filename, "w");
    if (filePointer == NULL) {
        printf("Failed to create file.\n");
        return;
    }
    for (int i=0; i<size; ++i) {
        fprintf(filePointer, "%.5f\n", a[i]);
    }
    fclose(filePointer);
}

#define VECTOR_EXPORT(...) vectorExport(__VA_ARGS__)
#endif

#endif /* UTILS_H */
