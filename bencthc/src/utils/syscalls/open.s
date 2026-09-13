    .globl b_syscall_open
    .text

b_syscall_open:

    # incoming
    # %rdi = path, %rsi = flags, %rdx = mode

    mov $2, %rax
    syscall

    ret