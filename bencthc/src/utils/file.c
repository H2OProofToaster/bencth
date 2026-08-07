//
// Created by nick on 6/26/26.
//

#include "file.h"

#include "allocator.h"
#include "syscalls/syscall.h"
#include "iHateLibC.h"

int b_fopen(const char* path, const int flags, const int mode) {

  return b_syscall_open(path, flags, mode);
}

int b_fopenRead(const char* path) {

  return b_syscall_open(path, O_RDONLY, 0);
}

int b_fopenWrite(const char* path) {

  return b_syscall_open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
}

int b_fclose(const int fd) {

  return b_syscall_close(fd);
}

int b_fstat(const int fd, struct stat* st) {

  return b_syscall_fstat(fd, st);
}

size_t b_fsize(const int fd) {

  struct stat st;

  if (b_fstat(fd, &st) < 0) { return -1; }

  return (size_t)st.st_size;
}

Arena* b_fread(const int fd) {

  struct stat st;

  //error while opening
  if (b_fstat(fd, &st) < 0) { return NULL; }

  const size_t size = (size_t)st.st_size;

  Arena* data = b_allocArenaSize(sizeof(Arena) + size + 1);

  char* buf = b_alloc(data, size + 1);

  for (size_t i = 0; i < size;) {

    const ssize_t n = b_syscall_read(fd, buf + i, size - i);

    //error while reading
    if (n < 0) {

      b_free(data);
      return NULL;
    }

    //unexpected EOF
    if (n == 0) { break; }

    i += n;
  }

  buf[size] = '\0';

  return data;
}

long b_fwrite(const int fd, const char* data, const size_t size) {

  return b_syscall_write(fd, data, size);
}