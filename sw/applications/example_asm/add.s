    .section .text
    .globl add_asm_function

add_asm_function:
    add a0, a0, a1   # Add the values in a0 and a1, store the result in a0
    ret              # Return from the function