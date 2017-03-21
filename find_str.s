
.global find_str
.func find_str

# int find_str(char *s, char *sub);
find_str:
    # r0 : "start" -> index of first char if match
    # r1 : character read from s
    # r2 : character read from sub
    # r3 : i (loop)
    # r4 : second operand in comparisons
    # r5 : length of matching string
    # r6 : s
    # r7 : sub

    push {r4, r5, r6, r7, r8, lr}

    mov r6, r0 /* backup s */
    mov r7, r1 /* backup sub */

    mov r0, #0 /* start = 0 */
    ldrb r2, [r7] /* sub[0] */
    
find_start:
    ldrb r1, [r6, r0] /* s[start] */
    cmp r1, #0 /* s[start] ==? 0 */
    beq check_reached_end_of_s /* exit while */
    cmp r1, r2 /* sub[0] ==? s[start]  */
    beq check_reached_end_of_s /* exit while */

    add r0, r0, #1 /* start++ */
    b find_start

check_reached_end_of_s:
    cmp r1, #0 /* s[start] ==? 0 */
    beq no_match

# program continues normally

try_next_start:
    mov r5, #0 /* len = 0 */
    ldrb r1, [r6, r0] /* s[start] */
    cmp r1, #0 /* reached end of s? */
    beq no_match

    mov r3, r0 /* i = start */

try_expanding:
    ldrb r1, [r6, r3] /* s[i] */
    ldrb r2, [r7, r5] /* sub[len] */
    cmp r1, #0 /* if s[i] == 0 */
    beq end_try_next_start
    cmp r2, #0 /* if sub[len] == 0 */
    beq end_try_next_start
    cmp r1, r2 /* s[i] ==? sub[len] */
    bne end_try_next_start

    add r3, #1 /* i++ */
    add r5, #1 /* len++ */
    b try_expanding

end_try_next_start:
    cmp r2, #0
    beq end
    addne  r0, #1
    bne try_next_start

no_match:
    mov r0, #-1
    b end

end:
    pop {r4, r5, r6, r7, r8, lr}
    bx lr
