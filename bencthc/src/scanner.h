//
// Created by nick on 7/3/26.
//

#ifndef BENCTH_SCANNER_H
#define BENCTH_SCANNER_H

typedef
long unsigned int size_t;

enum tokenType{

  //single characters
  EQUALS, PLUS, LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
  SEMICOLON,

  //single OR double characters

  //literals
  IDENTIFIER, INTEGER,

  //keywords
  RETURN, INT,

  EOF
};

typedef struct {

  const char* keyword;
  size_t length;
  enum tokenType type;

} keyword;

typedef union {

  size_t b_integer;
  double b_double;
  char b_char;
  char* b_string;

} literal;

typedef struct {

  Arena* a; //arena for token store
  enum tokenType type;
  char* lexeme; //pointer into source
  size_t length; //length of lexeme
  literal literal; //value for numbers and strings/chars
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

Scanner* scan(const char* sourcePath);

#endif //BENCTH_SCANNER_H