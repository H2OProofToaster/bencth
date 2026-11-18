//
// Created by nick on 8/2/26.
//

#ifndef BENCTH_STRUCTS_H
#define BENCTH_STRUCTS_H

#include "utils/allocator.h"

//first used in scanning/lexing

//logical value that a token can have
typedef enum {

  //single characters
  EQUALS, PLUS, LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
  SEMICOLON, MINUS, STAR,
  EXCLAMATION, TILDE, PERCENT, AMPERSAND, COMMA,

  //single OR double characters
  FORWARD_SLASH, DOUBLE_FORWARD_SLASH,

  //single OR triple characters
  ELLIPSIS,

  //literals
  IDENTIFIER, INTEGER, STRING,

  //keywords
  RETURN, INT,

  B_EOF,
} TokenType;

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

  TokenType type;

  //encodes token from buffer with pointer to location, and length
  char* lexeme;
  size_t length;

  //this should be guaranteed to exist
  //value for numbers, strings, chars, and identifiers, null terminated if it's a string (locked af)
  Literal literal;

} Token;

//created during scanning/lexing and passed off to parser
typedef struct {

  //arena
  Arena* a;

  //token storage
  Token* tokens;
  size_t count;
  size_t current;

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
  TokenType type;

} Keyword;

//I really don't like this
static const Keyword keywords[] = {
  {"return", 6, RETURN},
  {"int", 3, INT},
};

//AST nodes

typedef enum PrimaryExpressionType { PRIMARY_IDENTIFIER, PRIMARY_CONSTANT } PrimaryExpressionType;
typedef struct PrimaryExpression {

  PrimaryExpressionType type;
  union {

    struct identifier { char* identifier; } identifier;
    struct constant { int constant; } constant;
  };
} PrimaryExpression;

typedef struct MultiplicativeExpression {

  PrimaryExpression* left;
  TokenType operator;
  MultiplicativeExpression* right;
} MultiplicativeExpression;

typedef struct 

#endif //BENCTH_STRUCTS_H
