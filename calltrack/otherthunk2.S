.global mcount
mcount:
        ret

.global __x86_return_thunk
__x86_return_thunk:
	call safe_ret
	ret;
	int3
safe_ret:
        ret
	int3
