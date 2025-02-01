.globl ras_calls
	/* ras_calls(int *depth, int calls) */
	/*           %rdi	 %rsi	    */
ras_calls:
	decq	%rsi
	jz	1f
	call	ras_calls
1:	incq	(%rdi)
	ret
	ud2


	/* Create a deep chain of CALLS, but don't use RET to get out */
	/* Instead, stash the address of the outermost RET (2:) and
	 * jmp there from the innermost frame */
.global ras_calls2
	/* ras_calls2(int *depth, int calls, void *ret_addr) */
	/*	      %rdi	  %rsi,      %rdx	     */
ras_calls2:
	incq	(%rdi)
	test	%rdx, %rdx
	jnz	0f
	lea	2f(%rip), %rdx
0:	decq	%rsi
	jz	1f
	call	ras_calls2
1:	movq	(%rdi), %rax	/* Restore %rsp before we get out, from |depth| */
	shlq	$3, %rax
	subq	$8, %rax 	/* The one RET will remove */
	addq	%rax, %rsp
	jmp	*%rdx
2:
	ret
	ud2
