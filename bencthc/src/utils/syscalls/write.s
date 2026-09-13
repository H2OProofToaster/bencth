    .globl b_syscall_write
    .text

b_syscall_write:

    # incoming
    # %rdi = rd, %rsi = buf, %rdx = count

    mov $1, %rax
    syscall

    ret