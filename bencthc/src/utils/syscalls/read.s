    .globl b_syscall_read
    .text

b_syscall_read:

    # incoming
    # %rdi = fd, %rsi = buf, %rdx = count

    mov $0, %rax
    syscall

    ret