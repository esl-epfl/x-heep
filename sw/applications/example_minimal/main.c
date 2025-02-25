// Remember to build this app with make app COMPILER_FLAGS=-Os 
#include "syscalls.h"
int main(){
    _writestr("hi world");
    return 0;
}