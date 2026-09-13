    .globl b_syscall_munmap
    .text

b_syscall_munmap:

    # incoming
    # %rdi = addr, %rsi = length

    mov $11, %rax
    syscall

    ret