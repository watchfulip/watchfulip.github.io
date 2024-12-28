@Watchful_IP read ascii hex from UART and write to memory
@20-12-24 V0.0.2
@	HEX ASCII MUST BE UPPERCASE  (could add mask to convert lower to upper)

@first stage ida > mem offset is + 0x180000
.global _start

    push {r4-r8, lr}     @ Save reg

    ldr r7, =0x23E03EF0  + 0x180000 @ Load printf address (0x23E03EF0 + 0x180000)
    @ldr r0, =0x23E207DD + 0x180000   @ Load start message
    ldr r0, =start_msg   @ Load start message
    mov r1, lr           @ Save LR for start message
    blx r7               @ Call printf

    ldr r6, =0x23E03F94 + 0x180000 @ getc()
    ldr r10, =0x21010000    @ data dest

    ldr r8, =4930560          @ number of required inputs

input_loop:

@ Read first char
blx r6               @ Call getc()
mov r4, r0          @ Store first character in r4

@ Read second char
blx r6               @ Call getc()
mov r5, r0          @ Store second character in r5

@ Convert ASCII hex to int

    mov r1, r4           
    mov r2, r5           
    @ldr r0, =key_msg     	@DEBUG Load key message    
    @blx r7               	@DEBUG Call printf

sub r4, r4, #'0'
sub r5, r5, #'0'
cmp r4, #9
ble skip1
sub r4, r4, #7      @ Adjust for A-F
skip1:
cmp r5, #9
ble skip2
sub r5, r5, #7      @ Adjust for A-F
skip2:

@ Combine into a single byte
lsl r4, r4, #4      @ Shift left by 4 bits
orr r4, r4, r5      @ Combine with second digit

    strb r4, [r10], #1    @ Store byte and increment address
    subs r8, r8, #2      @ Decrement counter by 2
    bne input_loop       @ Continue if counter is not zero

    ldr r0, =end_msg     @ Load end message
    mov r1, lr           @ Load current LR for end message
    blx r7               @ Call printf

    pop {r4-r8, lr}      @ Restore saved registers
    bx lr                @ Return to U-Boot

start_msg:
    .asciz "Program started. LR: 0x%08x\n"
key_msg:
    .asciz "Keys entered: 0x%02X  0x%02X  \n"
end_msg:
    .asciz "Program ended. Returning to LR: 0x%08x\n"


