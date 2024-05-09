#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Define SIMULATION if you want to disable printing
// #define SIMULATION

// Enable or disable printing
#ifndef SIMULATION
#define PRINTF(...) printf(__VA_ARGS__)
#else
#pragma message ("Simulation mode, no printing")
#define PRINTF(...)
#endif


// Assert functions, always print if failing
void assert_closef(float a, float b, float prec) {
    float diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > prec) {
        printf("AF %d %f %f\n", __LINE__, a, b);
        exit(EXIT_FAILURE);
    }
}

void assert_closei32(int32_t a, int32_t b, int32_t prec) {
    int32_t diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > prec) {
        printf("AI %d %d %d\n", __LINE__, a, b);
        exit(EXIT_FAILURE);
    }
}

#endif /* UTILS_H */
