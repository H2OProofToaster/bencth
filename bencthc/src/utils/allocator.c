//
// Created by nick on 6/27/26.
//

#include "allocator.h"
#include <stdlib.h>
#include "bencthc/src/utils/exit.h"

Arena* b_allocArena() {

  return b_allocArenaSize(64 * 1024 * 1024);
}

Arena* b_allocArenaSize(const size_t size) {

  Arena* a = malloc(size);

  //check malloc fail
  if (a == NULL) { die("malloc crash"); }

  a->curr = 0;
  a->cap = size - sizeof(Arena);

  return a;
}

void* b_alloc(Arena* a, size_t size) {

  //round size to avoid shenanigans
  size = (size + 7) & ~7;

  //check for overflow
  if (a->cap < a->curr + size) { die("arena overflow"); }

  char* base = (char*)a + sizeof(Arena);
  void* mem = base + a->curr;
  a->curr += size;

  return mem;
}

void b_free(Arena* a) {

  free(a);
}