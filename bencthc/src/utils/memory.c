//
// Created by nick on 7/4/26.
//

#include "memory.h"

int b_memcmp(const void *s1, const void *s2, const size_t n) {

  const unsigned char* a = s1;
  const unsigned char* b = s2;

  for (int i = 0; i < n; i++) {

    if (a[i] != b[i]) { return 1; }
  }

  return 0;
}
