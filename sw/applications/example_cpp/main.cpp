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

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
}

#include "MyClass.hpp"
#include "core_v_mini_mcu.h"
#include "x-heep.h"

#define PRINTF_IN_FPGA 1

#if TARGET_SIM && PRINTF_IN_SIM
#ifndef TEST_MODE
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define PRINTF_TEST(...)
#else
#define PRINTF(...)
#define PRINTF_TEST(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif
#elif PRINTF_IN_FPGA && !TARGET_SIM
#ifndef TEST_MODE
#define PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define PRINTF_TEST(...)
#else
#define PRINTF(...)
#define PRINTF_TEST(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif
#else
#define PRINTF(...)
#define PRINTF_TEST(...)
#endif

int main()
{
    MyClass myObject(10);  // Create an object with initial value 10
    myObject.printValue(); // Print the initial value

    myObject.setValue(20); // Change the value to 20
    myObject.printValue(); // Print the updated value

    int value = myObject.getValue();          // Get the value
    PRINTF("Retrieved Value: %d\n\r", value); // Print the retrieved value

    int return_value = !(value == 20 * 5);

    PRINTF_TEST("0:%d&\n", return_value);

    return return_value ? EXIT_FAILURE : EXIT_SUCCESS;
}