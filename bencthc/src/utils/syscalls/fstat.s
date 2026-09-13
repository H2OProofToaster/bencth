    .globl b_syscall_fstat
    .text

b_syscall_fstat:

    # incoming
    # %rdi = fd, %rsi = statbuf

    mov $5, %rax
    syscall

    ret