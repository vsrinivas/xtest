// zen1-2
.align 64
.skip 64 - (srso_safe_ret - srso_untrain_ret), 0xcc

.global srso_untrain_ret
srso_untrain_ret:
        .byte 0x48
        .byte 0xb8
srso_safe_ret:
        addq $8, %rsp
        ret
        int3
        int3
        int3
        lfence
        call srso_safe_ret
        int3



.global __x86_return_thunk
__x86_return_thunk:
        call srso_safe_ret
        int3
