#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

extern char __heap_start[];
extern char __heap_end[];

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

// this works now
int* alloc_array_ret() {
    printf("allocate array and return\n");
    int* myarr = (int*)my_malloc(5 * sizeof(int));

    for (int i = 0; i < 5; ++i) {
        myarr[i] = i;
    }

    return myarr;
}

// this works now
void alloc_array() {
    printf("allocate arrays\n");
    int* arr = (int*)my_malloc(10 * sizeof(int));
    int* arr2 = (int*)my_malloc(5 * sizeof(int));
    int* arr3 = (int*)my_malloc(15 * sizeof(int));

    free(arr3);
    free(arr2);
    free(arr);
}

// this seems to work fine
void test_malloc_free() {
    for (int i = 0; i < 10; ++i) {
        int* arr = (int*)my_malloc(5 * sizeof(int));
        int* arr2 = (int*)my_malloc(5 * sizeof(int));
        free(arr);
        free(arr2);
    }
}

// FIXME: this is weird, why is NULL 0x8 suddenly???
void test_extensive() {
    int heep_size_bytes = __heap_end - __heap_start;
    printf("heep_size_bytes: %d\n", heep_size_bytes);
    char* ptr = __heap_start;
    printf("heep start: %p\n", ptr);
    while (ptr < __heap_end) {
        char* old_ptr = ptr;
        ptr = (char*)malloc(1);
        printf("ptr: %p\n", ptr);
        if ((ptr == 0x8) || (ptr == NULL)) {
            printf("Failed to allocate memory\n");
            break;
        }
        printf("offset: %d\n", ptr - old_ptr);
    }
}


int main(int argc, char *argv[]) {
    printf("heep_start: %p\n", __heap_start);
    printf("heep_end: %p\n", __heap_end);
    int* ret = alloc_array_ret();
    alloc_array();
    test_malloc_free();
    test_extensive();
    return EXIT_SUCCESS;
}