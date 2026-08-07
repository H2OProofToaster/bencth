//
// Created by nick on 6/27/26.
//

#include "exit.h"
#include "syscalls/syscall.h"
#include "string.h"

_Noreturn void die(const char* err) {

  //fd is 2 for stderr
  b_syscall_write(2, err, b_strlen(err));
  b_syscall_write(2, "\n", 1);

  b_syscall_exit(1);
}