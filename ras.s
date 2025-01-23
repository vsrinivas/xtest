.global x86_ras_fill
x86_ras_fill:
    pushq %rcx
    movl $16, %ecx
    .align 16 
0:  call 1f
.Ltrap1:
    pause
    lfence
    jmp .Ltrap1
    .align 16
1:  call 2f
.Ltrap2:
    pause
    lfence
    jmp .Ltrap2
    .align 16
2:
    subl $1, %ecx
    jnz 0b
    /* Prevent speculation past JNZ, until RAS is overwritten */
    lfence

    addq $256, %rsp
    popq %rcx
    ret
    lfence
