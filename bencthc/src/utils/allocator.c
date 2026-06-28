//
// Created by nick on 6/27/26.
//

#include "allocator.h"
#include <stdlib.h>
#include "bencthc/src/utils/exit.h"

Arena* b_allocArena() {

  Arena* a = malloc(64 * 1024 * 1024); //64MB

  a->buff = a;
  a->curr = 0;
  a->cap = 64 * 1024 * 1024;

  return a;
}

Arena* b_allocArenaSize(const size_t size) {

  Arena* a = malloc(size);

  a->buff = a;
  a->curr = 0;
  a->cap = size;

  return a;
}

void* b_alloc(Arena* a, size_t size) {

  //round size to avoid shenanigans
  size = (size + 7) & ~7;

  //check for space
  if (a->cap - a->curr < size) {

    die("arena overflow");
  }

  void* mem = a->buff + a->curr;
  a->curr += size;

  return mem;
}
