
extern "C" {
    #include <stdio.h>
    #include <stdlib.h>
}

#include "MyClass.hpp"

MyClass::MyClass(int initialValue) : value(initialValue) {}

void MyClass::setValue(int newValue) {
    value = newValue;
}

int MyClass::getValue() {
    return value*5;
}

void MyClass::printValue() {
    printf("Value: %d\n\r",  value);
}
