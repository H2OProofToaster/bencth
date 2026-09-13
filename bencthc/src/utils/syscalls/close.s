    .globl b_syscall_close
    .text

b_syscall_close:

    # incoming
    # %rdi = fd

    mov $3, %rax
    syscall

    ret