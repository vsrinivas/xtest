.globl vcopy2
vcopy2:
	/* vcopy2(void *dst, void *src, size_t len) */
	/*	  rdi,	     rsi,	rdx	    */
	xorl	%eax, %eax
	cmpq	$32, %rdx
	jbe	2f
1:
	vmovdqu	(%rsi, %rax, 1), %ymm0
	vmovdqu	%ymm0, (%rdi, %rax, 1)
	addq	$32, %rax
	sub	$32, %rdx
	cmpq	$32, %rdx
	jge	1b
	jz	.Lout
	addq	%rax, %rsi
	addq	%rax, %rdi

2:	movq	%rdx, %rcx
	rep movsb
.Lout:
	ret
