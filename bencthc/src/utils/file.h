//
// Created by nick on 6/26/26.
//

#ifndef BENCTHC_FILE_H
#define BENCTHC_FILE_H

int b_fopen(const char* path);
int b_fclose(int fd);
char* b_fread(int fd);

#endif //BENCTHC_FILE_H