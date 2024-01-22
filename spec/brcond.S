.global brcond
brcond:
	// uint64_t brcond(uint64_t *, uint64_t)
	//   take in a counter (%rsi), decrement and return.
	//   if the counter is non-zero, perform a load of (%rdi)
	movq %rsi, %rax
	decq %rax
	jz 1f		// skip the load when the ctr decrements to 0
	lfence
	lock addq $0, (%rdi)
	//movq (%rdi), %rcx
	nop
1:	ret
