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
  enum TokenType type;

} Keyword;

//I really don't like this
static const Keyword keywords[] = {
  {"return", 6, RETURN},
  {"int", 3, INT},
};

//first used in parsing

//encoding the grammar
typedef enum { EXPR_BINARY, EXPR_UNARY, EXPR_LITERAL, EXPR_VARIABLE, EXPR_GROUPING, EXPR_ASSIGN } ExprType;
typedef struct Expr {
  ExprType type;
  union {
    struct { struct Expr* left; enum TokenType operator; struct Expr* right; } binary;
    struct { enum TokenType op; struct Expr* operand; } unary;
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
    struct { Expr* expr; } exprStmt;
    struct { char* identifier; Expr* expr; } declStmt;
  };
} Stmt;

typedef struct {

  char* identifier;
  Stmt** stmts;
  size_t count;
} Function;

typedef struct {

  Function* function;
} Program;

//symbols for lookup, stored in a linked list
//ik, but we aren't really caring about lookup time right no
typedef struct Symbol {

  Token* token;
  struct Symbol* next;

  int offset; //filled in at codegen for stack lookup
} Symbol;
typedef struct SymbolTable{

  Symbol* head;

  struct SymbolTable* outerScope;
} SymbolTable;

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

  //symbol table
  SymbolTable* symbolTable;
} Parser;

#endif //BENCTH_STRUCTS_H