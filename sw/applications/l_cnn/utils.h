#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Define SIMULATION if you want to disable printing
#define SIMULATION

// Enable or disable printing
#ifndef SIMULATION
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

// Assert functions, always print if failing
void assert_closef(float a, float b, float prec, int idx) {
    float diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > prec) {
        printf("AF %d %d %d %d\n", __LINE__, (int)(a*100000), (int)(b*100000), idx);
        exit(EXIT_FAILURE);
    }
}

void assert_closei32(int32_t a, int32_t b, int32_t prec, int idx) {
    int32_t diff = a - b;
    if (diff < 0) diff = -diff;
    if (diff > prec) {
        printf("AI %d %d %d %d\n", __LINE__, a, b, idx);
        exit(EXIT_FAILURE);
    }
}

// Vector export for plots
#ifndef SIMULATION
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
#else
#define VECTOR_EXPORT(...)
#endif

#endif /* UTILS_H */
