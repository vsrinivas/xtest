.globl _zencpy2
_zencpy2:
	/* _zencpy2(void *dst, void *src, size_t len) */
	/*	  rdi,	     rsi,	rdx	    */

	cmpq	$32, %rdx
	jbe	1f
	movq	%rdx, %rcx
	shrq	$3, %rcx
	andq	$7, %rdx
lp:	movq	(%rsi), %rax
	movnti	%rax, (%rdi)
	addq	$8, %rsi
	addq	$8, %rdi
	subq	$1, %rcx
	jnz	lp
1:	movq	%rdx, %rcx
	rep movsb
	sfence
	ret

