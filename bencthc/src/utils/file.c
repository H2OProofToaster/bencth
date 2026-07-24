//
// Created by nick on 6/26/26.
//

#include "file.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "bencthc/src/utils/allocator.h"

typedef
long unsigned int size_t;

int b_fopen(const char* path) {

  return open(path, O_RDWR | O_CREAT);
}

int b_fclose(const int fd) {

  return close(fd);
}

int b_fstat(const int fd, struct stat* st) {

  return fstat(fd, st);
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

  for (size_t i = 0; i < size;) {

    const ssize_t n = read(fd, (char*)data + sizeof(Arena) + i, size - i);

    //error while reading
    if (n < 0) {

      b_free(data);
      return NULL;
    }

    //unexpected EOF
    if (n == 0) { break; }

    i += n;
  }

  ((char*)data + sizeof(Arena))[size] = '\0';

  return data;
}

int b_fwrite(const int fd, const char* data, const size_t size) {

  return (int)write(fd, data, size);
}