//
// Created by nick on 6/26/26.
//

#include "print.h"

#include "string.h"
#include "syscalls/syscall.h"

void b_printString(const char *s) {

  b_syscall_write(1, s, b_strlen(s));
  b_syscall_write(1, "\n", 1);
}

void b_printStringNoNewline(const char *s) {

  b_syscall_write(1, s, b_strlen(s));
}

void b_printInt(int i) {

  //ripped off from b_intToString
  const int negative = i < 0;
  i = negative ? -i : i;

  char temp[INT_TO_STRING_SIZE];
  int count = 0;
  int n = 0;

  while (i > 0) {

    temp[n++] = (char)('0' + i % 10);
    i /= 10;
    count++;
  }

  if (negative) { temp[n++] = '-'; }

  b_syscall_write(1, temp, count);
}

void b_printChar(const char c) {

  b_syscall_write(1, &c, 1);
}