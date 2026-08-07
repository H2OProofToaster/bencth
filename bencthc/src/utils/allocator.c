//
// Created by nick on 6/27/26.
//

#include "allocator.h"

#include "exit.h"
#include "iHateLibC.h"
#include "syscalls/syscall.h"

Arena* b_allocArena() {

  return b_allocArenaSize(ARENA_SIZE);
}

Arena* b_allocArenaSize(const size_t size) {

  //fd and offset are ignored for NULL addr
  Arena* a = b_syscall_mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  //check mmap fail
  if (a == (void*)-1) { die("could not allocate memory"); }

  a->curr = 0;
  a->cap = size - sizeof(Arena);

  return a;
}

void* b_alloc(Arena* a, size_t size) {

  //round size to nearest byte to avoid shenanigans
  size = size + 7 & ~7;

  //check for overflow
  if (a->cap < a->curr + size) { die("arena overflow"); }

  char* base = (char*)a + sizeof(Arena);
  void* mem = base + a->curr;
  a->curr += size;

  return mem;
}

void b_free(Arena* a) {

  b_syscall_munmap(a, a->cap + sizeof(Arena));
}