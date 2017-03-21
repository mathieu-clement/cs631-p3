
.global sum_array
.func sum_array

# int sum_array(int *array, int n)
sum_array:
    # r0 : pointer to array (address is incremented directly to access elements) 
    # r1 : size of array => int
    # r2 : sum => int
    # r3 : i (for loop index) => int
    # r4 : value of array[r0]
    sub sp, sp, #8
    push {r4}
    mov r2, $0 // sum = 0
    mov r3, $0 // i = 0

    cmp r1, $0 // n == 0
    beq end

loop:
    ldr r4, [r0]
    add r0, $4
    add r2, r4 // sum += array[i]
    add r3, $1
    cmp r3, r1 // i < n
    bne loop

end:
    mov r0, r2
    pop {r4}
    add sp, sp, #8
    bx lr
