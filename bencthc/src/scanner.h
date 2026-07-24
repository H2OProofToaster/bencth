//
// Created by nick on 7/3/26.
//

#ifndef BENCTH_SCANNER_H
#define BENCTH_SCANNER_H

#include "bencthc/src/utils/allocator.h"

typedef long unsigned int size_t;

enum TokenType{

  //single characters
  EQUALS, PLUS, LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
  SEMICOLON,

  //single OR double characters

  //literals
  IDENTIFIER, INTEGER, STRING,

  //keywords
  RETURN, INT,

  B_EOF,
};

typedef struct {

  const char* keyword;
  size_t length;
  enum TokenType type;

} Keyword;

typedef union {

  int b_integer;
  double b_double;
  char b_char;
  char* b_string;

} Literal;

typedef struct {

  Arena* a; //arena for token store
  enum TokenType type;
  char* lexeme; //pointer into source
  size_t length; //length of lexeme
  Literal literal; //value for numbers and strings/chars, null terminated (loc
  int line;

} Token;

typedef struct {

  //arena
  Arena* a;

  //token storage
  Token* tokens;
  size_t count;

  //source storage
  char* source;
  size_t length;

  //source maneuvering
  char* curr;
  int line;
} Scanner;

static const Keyword keywords[] = {
  {"return", 6, RETURN},
  {"int", 3, INT},
};

Scanner* scan(const char* sourcePath);
void printTokens(const Scanner* s);

#endif //BENCTH_SCANNER_H