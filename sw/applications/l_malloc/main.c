#include <stdlib.h>
#include <stdio.h>

void* my_calloc(size_t num, size_t size) {
    void *ptr = calloc(num, size);
    if (ptr != NULL) {
        printf("Allocated memory range: [%p - %p]\n", ptr, (char*)ptr + num * size - 1);
    } else {
        printf("Failed to allocate memory\n");
    }
    return ptr;
}

void* my_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr != NULL) {
        printf("Allocated memory range: [%p - %p]\n", ptr, (char*)ptr + size - 1);
    } else {
        printf("Failed to allocate memory\n");
    }
    return ptr;
}

typedef struct __Dim2D {
    int x;
    int y;
} Dim2D;

Dim2D* alloc_struct() {
    printf("allocate struct\n");
    return (Dim2D*)my_malloc(sizeof(Dim2D));
}

void alloc_array() {
    printf("allocate arrays\n");
    int* arr = (int*)my_malloc(10 * sizeof(int));
    int* arr2 = (int*)my_malloc(5 * sizeof(int));
    int* arr3 = (int*)my_malloc(15 * sizeof(int));

    free(arr3);
    free(arr2);
    free(arr);
}

int main(int argc, char *argv[]) {
    Dim2D* dim = alloc_struct();
    alloc_array();
    return EXIT_SUCCESS;
}