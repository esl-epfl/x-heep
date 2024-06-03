    .section .text
    .globl add

add:
    add a0, a0, a1   # Add the values in a0 and a1, store the result in a0
    ret              # Return from the function