//
// Created by nick on 7/13/26.
//

#ifndef BENCTH_PARSER_H
#define BENCTH_PARSER_H

#include "bencthc/src/scanner.h"

typedef enum { EXPR_BINARY, EXPR_UNARY, EXPR_LITERAL, EXPR_VARIABLE, EXPR_GROUPING, EXPR_ASSIGN } ExprType;

typedef struct Expr {
  ExprType type;
  union {
    struct { struct Expr* left; enum TokenType operator; struct Expr* right; } binary;
    struct { enum TokenType op; struct Expr* operand; } unary;
    struct { int value; } literalExpr;
    struct { char* name; } variable;
    struct { struct Expr* inner; } grouping;
    struct { char* name; struct Expr* value; } assign;
  };
} Expr;

typedef enum { STMT_RETURN, STMT_EXPR, STMT_DECL } StmtType;

typedef struct {

  StmtType type;

  union {

    struct { Expr* value; } returnStmt;
    struct { Expr* value; } exprStmt;
    struct { char* identifier; Expr* value; } declStmt;
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

Parser* parse(const Scanner* s);
void printProgram(const Program* p);

#endif //BENCTH_PARSER_H