//
// Created by nick on 6/26/26.
//

#include "file.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

//READ-ONLY
int b_fopen(const char* path) {

  return open(path, O_RDONLY);
}

int b_fclose(const int fd) {

  return close(fd);
}

char* b_fread(const int fd) {

  struct stat st;

  //error while opening
  if (fstat(fd, &st) < 0) { return NULL;}

  const size_t size = (size_t)st.st_size;

  char* data = malloc(size + 1);

  for (size_t i = 0; i < size;) {

    const ssize_t n = read(fd, data + i, size - i);

    //error while reading
    if (n < 0) {

      free(data);
      return NULL;
    }

    //unexpected EOF
    if (n == 0) { break; }

    i += n;
  }

  data[size] = '\0';

  return data;
}
