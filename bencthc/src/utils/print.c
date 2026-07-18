//
// Created by nick on 6/26/26.
//

#include "print.h"

#include <stdio.h>

void b_printString(const char *s) {

  printf("%s\n", s);
}

void b_printStringNoNewline(const char *s) {

  printf("%s", s);
}

void b_printInt(const size_t n) {

  printf("%ld\n", n);
}

void b_printChar(const char c) {

  printf("%c", c);
}