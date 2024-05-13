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

int* alloc_array_ret() {
    printf("allocate array and return\n");
    int* myarr = (int*)my_malloc(5 * sizeof(int));

    for (int i = 0; i < 5; ++i) {
        myarr[i] = i;
    }

    return myarr;
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
    int* ret = alloc_array_ret();
    alloc_array();
    return EXIT_SUCCESS;
}