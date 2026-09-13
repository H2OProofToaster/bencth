    .globl b_syscall_mmap
    .text

b_syscall_mmap:

    # incoming
    # %rdi = addr, %rsi = length, %rdx = prot, %rcx = flags, %r8 = fd, %r9 = offset

    # syscall clobbers %rcx
    mov %rcx, %r10

    mov $9, %rax
    syscall

    ret