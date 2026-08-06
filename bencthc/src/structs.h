//
// Created by nick on 8/2/26.
//

#ifndef BENCTH_STRUCTS_H
#define BENCTH_STRUCTS_H

#include "utils/allocator.h"
#include "utils/iHateLibC.h"

//first used in scanning/lexing

//logical value that a token can have
typedef enum {

  //single characters
  EQUALS, PLUS, LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
  SEMICOLON, MINUS, STAR,

  //single OR double characters
  FORWARD_SLASH, DOUBLE_FORWARD_SLASH,

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

//first used in parsing

//symbols for lookup, stored in a linked list
//ik, but we aren't really caring about lookup time right now
typedef struct Symbol {

  Token* token;
  struct Symbol* next;

  int offset; //filled in at codegen for stack lookup
} Symbol;
typedef struct SymbolTable{

  Symbol* head;

  struct SymbolTable* outerScope;
} SymbolTable;

//encoding the grammar
typedef enum { EXPR_BINARY, EXPR_UNARY, EXPR_LITERAL, EXPR_VARIABLE, EXPR_GROUPING, EXPR_ASSIGN } ExprType;
typedef struct Expr {
  ExprType type;
  union {
    struct { struct Expr* left; TokenType operator; struct Expr* right; } binary;
    struct { TokenType op; struct Expr* operand; } unary;
    struct { int value; } literal;
    struct { char* name; } variable;
    struct { struct Expr* inner; } grouping;
    struct { char* name; struct Expr* expr; } assign;
  };
} Expr;

typedef enum { STMT_RETURN, STMT_EXPR, STMT_DECL } StmtType;
typedef struct {

  StmtType type;

  union {

    struct { Expr* expr; } returnStmt;
    struct { char* identifier; Expr* expr; } exprStmt;
    struct { TokenType type; char* identifier; Expr* expr; } declStmt;
  };
} Stmt;

typedef struct {

  char* identifier;
  Stmt** stmts;
  size_t count;

  SymbolTable* symbolTable;
} Function;

typedef struct {

  Function* function;

  SymbolTable* symbolTable;
} Program;

typedef struct {

  //node storage
  Arena* a;

  //from scanner
  Token* tokens;
  size_t count;

  //next token index
  size_t current;

  //head of ast
  Program* program;
} Parser;

#endif //BENCTH_STRUCTS_H