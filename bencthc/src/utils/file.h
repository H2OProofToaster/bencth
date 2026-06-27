//
// Created by nick on 6/26/26.
//

#ifndef BENCCH_FILE_H
#define BENCCH_FILE_H

int openFile(const char* path);
int closeFile(int fd);
char* readChar(int fd, int offset);
char* readFile(const char* path);

#endif //BENCCH_FILE_H