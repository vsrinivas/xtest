.global now
now:
	mfence
	lfence
	rdtscp
	mfence
	shlq	$32, %rdx
	orq	%rdx, %rax
	ret
