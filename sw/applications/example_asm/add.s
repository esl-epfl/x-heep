    .section .text
    .globl add_asm_function

add_asm_function:
    flw	f0,0(a0)
    flw	f1,0(a1)
    #check if it gets executed in a branch shadow
    li t0, 100
    loop1:
    fadd.s fa0, f0, f1   # Add the values in a0 and a1, store the result in a0
    fadd.s fa0, fa0, f1   # Add the values in a0 and a1, store the result in a0
    fadd.s fa0, fa0, f1   # Add the values in a0 and a1, store the result in a0
    fadd.s fa0, fa0, f1   # Add the values in a0 and a1, store the result in a0
    fadd.s fa0, fa0, f1   # Add the values in a0 and a1, store the result in a0
    fadd.s fa0, fa0, f1   # Add the values in a0 and a1, store the result in a0
    fadd.s fa0, fa0, f1   # Add the values in a0 and a1, store the result in a0
    fadd.s fa0, fa0, f1   # Add the values in a0 and a1, store the result in a0
    fadd.s fa0, fa0, f1   # Add the values in a0 and a1, store the result in a0
    fadd.s fa0, fa0, f1   # Add the values in a0 and a1, store the result in a0
    addi t0,t0,-1
    bnez t0, loop1
    ret              # Return from the function