//
// Created by nick on 6/26/26.
//

#include "file.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

//READ-ONLY
int openFile(const char* path) {

  return open(path, O_RDONLY);
}

int closeFile(const int fd) {

  return close(fd);
}

char* readChar(const int fd, const int offset) {

  struct stat size;
  fstat(fd, &size);

  char* data = malloc(1);

  read(fd, &data, offset);

  return data;
}

char* readFile(const char* path) {


}
