//
// Created by nick on 8/2/26.
//

#ifndef BENCTH_STRUCTS_H
#define BENCTH_STRUCTS_H

#include "bencthc/src/utils/allocator.h"

typedef long unsigned int size_t;

//first used in scanning/lexing

//logical value that a token can have
enum TokenType{

  //single characters
  EQUALS, PLUS, LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
  SEMICOLON, MINUS, STAR,

  //single OR double characters
  FORWARD_SLASH, DOUBLE_FORWARD_SLASH,

  //literals
  IDENTIFIER, INTEGER, STRING, TYPE,

  //keywords
  RETURN, INT,

  B_EOF,
};

//holds operable literals for tokens
//i.e. the actual int you can pull from, or the string
typedef union {

  int b_integer;
  //idk why doubles were here in the first place;
  //nowhere near implementing them 8/2/2026
  //double b_double;
  char b_char;
  char* b_string;

} Literal;

//token created when lexing
typedef struct {

  //arena for token storage, freed after codegen
  Arena* a;

  //line in source
  int line;

  //above is handled in addToken()
  //------------------------------
  //below is handled in consumeX()

  enum TokenType type;

  //encodes token from buffer with pointer to location, and length
  char* lexeme;
  size_t length;

  //this should be guaranteed to exist
  //value for numbers and strings/chars, null terminated if it's a string (locked af)
  Literal literal;

} Token;

//created durring scanning/lexing and passed off to parser
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

//holds the info of a keyword for the static array
typedef struct {

  const char* keyword;
  size_t length;
  enum TokenType type;

} Keyword;

//I really don't like this
static const Keyword keywords[] = {
  {"return", 6, RETURN},
  {"int", 3, INT},
};

#endif //BENCTH_STRUCTS_H