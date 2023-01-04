.globl _zencpy2
_zencpy2:
	/* _zencpy2(void *dst, void *src, size_t len) */
	/*	  rdi,	     rsi,	rdx	    */

	cmpq	$32, %rdx
	jbe	1f
	movq	%rdx, %rcx
	shrq	$3, %rcx
	andq	$7, %rdx
	rep movsq
1:	movq	%rdx, %rcx
	rep movsb
	ret

