.global now
now:
	rdtscp
	lfence
	shlq	$32, %rdx
	orq	%rdx, %rax
	ret

.global load
load:
	movq (%rdi), %rax
	ret

.global touch
touch:
	xorl %eax, %eax
	movq %rax, (%rdi)
	ret
