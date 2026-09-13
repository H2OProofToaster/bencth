    .globl b_syscall_exit
    .text

b_syscall_exit:

    # incoming
    # %rdi = status

    mov $60, %rax
    syscall

    # hypothetically shouldn't need to do this
    ret