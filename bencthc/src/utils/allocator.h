//
// Created by nick on 6/27/26.
//

#ifndef BENCTH_ALLOCATOR_H
#define BENCTH_ALLOCATOR_H

#include "iHateLibC.h"

#define ARENA_SIZE (64 * 1024 * 1024)

typedef struct {

  size_t curr;
  size_t cap;
} Arena;

Arena* b_allocArena();
Arena* b_allocArenaSize(size_t size);
void* b_alloc(Arena* a, size_t size);
void b_free(Arena* a);

#endif //BENCTH_ALLOCATOR_H