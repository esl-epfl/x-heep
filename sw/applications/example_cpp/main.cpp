/*
 * Copyright 2024 EPFL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Juan Sapriza <juan.sapriza@epfl.ch>
 */



extern "C" {
    #include <stdio.h>
    #include <stdlib.h>
}

#include "MyClass.hpp"
#include "core_v_mini_mcu.h"
#include "x-heep.h"

#if TARGET_SIM && PRINTF_IN_SIM
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#elif PRINTF_IN_FPGA && !TARGET_SIM
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define PRINTF(...)
#endif

int main()
{
    MyClass myObject(10); // Create an object with initial value 10
    myObject.printValue(); // Print the initial value

    myObject.setValue(20); // Change the value to 20
    myObject.printValue(); // Print the updated value

    int value = myObject.getValue(); // Get the value
    PRINTF("Retrieved Value: %d\n\r" ,value); // Print the retrieved value

    return value == 20*5 ? EXIT_SUCCESS : EXIT_FAILURE;
}