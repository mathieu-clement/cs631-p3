
.global find_max
.func find_max

# int find_max(int *array, int n)
find_max:
    # r0 : pointer to array 
    # r1 : size of array => int
    # r2 : max => int
    # r3 : i (for loop index) => int
    # r4 : value of array[r0]
    sub sp, sp, #8
    str r4, [sp] /* Preserve r4 */
    ldr r4, [r0] /* array[i] = array[0] */
    mov r2, r4  /* max = array[0] */
    mov r3, #0
    cmp r1, #1          /* if n == 1 */
    beq end             /*      goto end */

loop:
    cmp r3, r1          /* if i == n */
    beq end             /*      goto end */
    cmp r4, r2          /* if array[i] > max */
    movge r2, r4        /*      max = array[i] */
    add r3, r3, #1      /* i++ */
    cmp r3, r1          /* if i == n */
    beq end             /*      goto end */
    ldr r4, [r0, r3, LSL #2]    /* array[i] => array[0+i] */
    b loop

end:
    ldr r4, [sp]        /* Restore r4 */
    add sp, sp, #8
    mov r0, r2          /* return max */
    bx lr
