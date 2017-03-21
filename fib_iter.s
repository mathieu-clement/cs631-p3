
.global fib_iter
.func fib_iter

# int fib_iter (int n)
fib_iter:
    push {r4, r5}
    # r0 : n => int

    # Handle special case of n == 0
    cmp r0, #0
    bxeq lr

    # Create an array of n + 1 elements on the stack
    add r1,  r0, #1     /* r1 = n + 1 */
    mov r3, #4
    mul r2, r1, r3
    sub sp, r2

    # array[0] = 0
    mov r2, #0
    str r2, [sp]

    # array[1] = 1
    mov r2, #1
    str r2, [sp, #4]

    # array[2 ... n] = array[i-1] + array[i-2]

    # r3 : i => int
    mov r3, #1 /* starts at 1, will be incremented to 2 immediately */

    # r2 : i-1 or i-2
    # r4 : array[i-1] 
    # r5 : array[i-2]

loop:
    add r3, #1 /* ++i */
    cmp r3, r1 /* loop end condition */
    beq end

    # array[i-1]
    sub r2, r3, #1
    ldr r4, [sp, r2, LSL #2]

    # array[i-2]
    sub r2, r3, #2
    ldr r5, [sp, r2, LSL #2]

    # array[i-1] + array[i-2]
    add r4, r5
    str r4, [sp, r3, LSL #2]

    b loop

end:
    # return array[n]
    ldr r0, [sp, r0, LSL #2]
    # restore stack
    mov r3, #4
    mul r2, r1, r3
    add sp, r2
    pop {r4, r5}
    bx lr
