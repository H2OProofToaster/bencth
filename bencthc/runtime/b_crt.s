    .globl _start

_start:

    # move argc into rdi
    movl (%rsp), %edi

    # move argv into rsi
    leaq 8(%rsp), %rsi

    # 16-byte align rsp
    and $-16, %rsp

    call main

    # get main's return value
    movl %eax, %edi

    # exit_group syscall
    mov $231, %rax
    syscall