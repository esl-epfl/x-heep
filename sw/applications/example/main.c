#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
    /* write something to stdout */
    printf("hello world!\n");
    float a = 1.3;
    float b = 3.7;
    float c = 0.0;
    c = a * b;
    printf("Result: %f\n", c);
    return EXIT_SUCCESS;
}



















// #include <stdio.h>

// int main(){

//     printf("Hello World!\n");

//     float a = 10.0;
//     float b = 5.0;

//     printf("Res:\t%f\n", (a * b) + a - 2.0);

//     printf("Ciao\n");
//     // make app PROJECT=epilepsy_C COMPILER=gcc ARCH=rv32imfc
//     return 0;
// }