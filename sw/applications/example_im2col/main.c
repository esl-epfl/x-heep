/*
    Copyright EPFL contributors.
    Licensed under the Apache License, Version 2.0, see LICENSE for details.
    SPDX-License-Identifier: Apache-2.0

    Author: Tommaso Terzano <tommaso.terzano@epfl.ch>
                            <tommaso.terzano@gmail.com>
    
    Info: Example application of im2col algorithm with configurable format, verification and performance analysis.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "x-heep.h"
#include "im2col_lib.h"

/* Test variables */
int errors;
unsigned int cycles;

int main()
{
    for (int i=START_ID; i<3; i++)
    {
      im2col_nchw_int32(i, &cycles);

      #if TEST_EN == 0
      PRINTF("im2col NCHW test %d executed\n\r", i);
      PRINTF_TIM("Total number of cycles: [%d]\n\r", cycles);
      #endif

      errors = verify();

      if (errors != 0)
      {
          #if TEST_EN == 0
          PRINTF("TEST %d FAILED: %d errors\n\r", i, errors);
          return EXIT_FAILURE;
          #else
          PRINTF_TIM("%d:%d:1\n\r", i, cycles);
          #endif
          
      } 
      else
      {
          #if TEST_EN == 0
          PRINTF("TEST PASSED!\n\r\n\r");
          #else
          PRINTF_TIM("%d:%d:0\n\r", i, cycles);                
          #endif
      } 
    }
    
    /* Print the end word for verification */
    PRINTF("&\n\r");

    return EXIT_SUCCESS;
}
