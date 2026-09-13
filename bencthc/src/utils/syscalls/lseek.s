    .globl b_syscall_lseek
    .text

b_syscall_lseek:

    # incoming
    # %rdi = fd, %rsi = offset, %rdx = whence

    mov $8, %rax
    syscall

    ret