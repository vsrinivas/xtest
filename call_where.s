.global add_start
.text
add_start:
    .rept 1536
    .p2align 7
    nop
    nop
    leaq ., %rax
    addq %rax, (%rdi)
    ret
    ud2
    .endr
