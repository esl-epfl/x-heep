    .section .text
    .globl add_asm_function2

add_asm_function2:
    flw	f0,0(a0)
    flw	f1,0(a1)
    #check if it gets executed in a branch shadow
    cv.starti 0, start_loop
    cv.endi   0, end_loop
    cv.counti  0, 100
    start_loop:
    .balign 4
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
    end_loop:
    ret              # Return from the function