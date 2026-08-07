//
// Created by nick on 6/26/26.
//

#ifndef BENCTH_FILE_H
#define BENCTH_FILE_H

#include "allocator.h"
#include "iHateLibC.h"
#include "syscalls/syscall.h"

int b_fopen(const char* path, int flags, int mode);
int b_fopenRead(const char* path);
int b_fopenWrite(const char* path);
int b_fclose(int fd);
int b_fstat(int fd, struct stat* st);
size_t b_fsize(int fd);
Arena* b_fread(int fd);
long b_fwrite(int fd, const char* data, size_t size);

#endif //BENCTH_FILE_H