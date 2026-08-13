/*
 * The recurrence is order-sensitive on purpose: a plain sum commutes, and so
 * is blind to "right targets, wrong order".
 */

.global add_start
.type add_start,@function
.text
.balign 128
add_start:
    .rept 1536

    .p2align 7, 0xcc
    leaq .(%rip), %rax          /* rax = address of this block */
    movq (%rdi), %rdx
    leaq (%rdx,%rdx,2), %rdx    /* rdx *= 3 */
    addq %rax, %rdx
    movq %rdx, (%rdi)
    ret

    ud2
    .endr
.size add_start, . - add_start

.section .note.GNU-stack,"",@progbits
