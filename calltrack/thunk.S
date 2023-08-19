.data
depth:  .quad 0

.text
.global x86stuff
x86stuff:
    pushq %rcx
    movl $16, %ecx
 
     .align 16
0:  call 1f
.Ltrap1:
    pause
    lfence
    jmp .Ltrap1
 
    .align 16
1:  call 2f
.Ltrap2:
   pause
   lfence
   jmp .Ltrap2
   .align 16
2:
   subl $1, %ecx
   jnz 0b
   /* Prevent speculation past JNZ, until RAS is overwritten */
   lfence
 
   addq $256, %rsp
   popq %rcx
   ret
   int3;

.global mcount
mcount:
        movq    $depth, %rax
        incq    (%rax)
        ret

.global __x86_return_thunk
__x86_return_thunk:
        xorl %eax, %eax		// TODO: Eax holds return, oops.
        movq $depth, %rcx
        movq (%rcx), %rdi       // load depth
                // if %edx != 0, set eax = 1
                test %edi, %edi
                setnz %al
        subl %eax, %edi         // sub from depth, either 0 or 1
        cmpl $12, %edi
        jl .Lout
                // stuff rsb;
                xorl %edi, %edi
                call x86stuff
.Lout:
        lfence
        movq %rdi, (%rcx)
        ret
