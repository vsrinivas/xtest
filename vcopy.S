.globl vcopy
vcopy:
	/* vcopy(void *dst, void *src, size_t len) */
	/*	  rdi,	     rsi,	rdx	    */
	xorl	%eax, %eax
	cmpq	$16, %rdx
	jbe	2f
1:
	movdqu	(%rsi, %rax, 1), %xmm0
	movdqu	%xmm0, (%rdi, %rax, 1)
	addq	$16, %rax
	sub	$16, %rdx
	cmpq	$16, %rdx
	jge	1b
	jz	.Lout
	addq	%rax, %rsi
	addq	%rax, %rdi

2:	movq	%rdx, %rcx
	rep movsb
.Lout:
	ret
