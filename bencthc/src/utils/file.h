//
// Created by nick on 6/26/26.
//

#ifndef BENCTH_FILE_H
#define BENCTH_FILE_H

#include <sys/stat.h>
#include "bencthc/src/utils/allocator.h"

typedef
long unsigned int size_t;

int b_fopen(const char* path);
int b_fclose(int fd);
int b_fstat(int fd, struct stat* st);
size_t b_fsize(int fd);
Arena* b_fread(int fd);

#endif //BENCTH_FILE_H