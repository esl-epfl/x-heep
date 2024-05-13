#include "utils.h"

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
    int* arr2 = (int*)my_malloc(10 * sizeof(int));
    int* arr3 = (int*)my_malloc(10 * sizeof(int));

    free(arr3);
    free(arr2);
    free(arr);
}

int main(int argc, char *argv[]) {
    Dim2D* dim = alloc_struct();
    alloc_array();
    return EXIT_SUCCESS;
}