.func real_add
real_add:
    stmfd sp!, {r8, r9, r10}
    add r2, r2, r3
    add r1, r1, r2
    add r0, r0, r1
    ldmia sp!, {r8, r9, r10}
    bx lr
.endfunc

.global add_function
.func add_function
add_function:
    sub sp, sp, #8
    mov r7, #123
    str r7, [sp]
    str lr, [sp, #4]

    mov r8, #108
    mov r9, #109
    mov r10, #110
    
    bl real_add
    ldr r7, [sp]
    ldr lr, [sp, #4]
    add sp, sp, #8

    mov r0, #99
    cmp r0, #46
    movne r0, #1
    moveq r0, #2
    movge r0, #3
    movle r0, #4
    bx lr
.endfunc
