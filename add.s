.global add
.func add

add:
    add r5, r6, r7
    add r2, r2, r3
    add r1, r1, r2
    add r0, r0, r1
    bx lr
