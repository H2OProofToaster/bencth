//
// Created by nick on 7/2/26.
//

#ifndef BENCTH_STRING_H
#define BENCTH_STRING_H

#include "bencthc/src/scanner.h"

typedef
long unsigned int size_t;

void b_lexemeToLiteral(Token* t);
size_t b_strlen(const char* data);
int b_strcmp(const char* a, const char* b);
char* b_intToString(Arena* a, int i);

#endif //BENCTH_STRING_H