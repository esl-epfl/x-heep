#include "constants.h"

    .section .text
    .globl mul_by_const_asm_function

mul_by_const_asm_function:
    li t0, MULTIPLY_CONSTANT    # Load the constant into temporary register t0
    mul a0, a0, t0              # Multiply the value in a0 by the constant in t0
    ret                         # Return from the function