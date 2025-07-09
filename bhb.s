.global bhb_scrub_short
bhb_scrub_short:
	mov $5, %ecx
	call 801f
	jmp 805f
	.align 64
801:	call 802f
	ret
	.align 64
802:	movl $5, %eax
803:	jmp 804f
	nop
804:	sub $1, %eax
	jnz 803b
	sub $1, %ecx
	jnz 801b
	ret
805:	lfence
	ret
	lfence

.global bhb_scrub_long
bhb_scrub_long:
	ret
