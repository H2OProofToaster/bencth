//
// Created by nick on 7/13/26.
//

#include "parser.h"
#include "scanner.h"
#include "utils/exit.h"
#include "utils/string.h"

//forward declarations
Expr* parseExpr(Parser* p);

static Token* peek(const Parser* p) {

  return &p->tokens[p->current];
}

static Token* advance(Parser* p) {

  Token* t = &p->tokens[p->current];
  if (p->current < p->count) { p->current++; }
  return t;
}

int check(const Parser* p, const enum TokenType type) {

  return peek(p)->type == type;
}

Token* consume(Parser* p, const enum TokenType type, const char* err) {

  if (check(p, type)) { return advance(p); }

  die(err);
  return NULL;
}

Expr* parsePrimary(Parser* p) {

  if (check(p, INTEGER)) {

    Expr* curr = b_alloc(p->a, sizeof(Expr));
    curr->type = EXPR_LITERAL;
    curr->literalExpr.value = advance(p)->literal.b_integer;
    return curr;
  }

  if (check(p, IDENTIFIER)) {

    Expr* curr = b_alloc(p->a, sizeof(Expr));
    curr->type = EXPR_VARIABLE;
    curr->variable.name = advance(p)->literal.b_string;
    return curr;
  }

  if (check(p, LEFT_PAREN)) {

    advance(p); //eat '('
    Expr* inner = parseExpr(p);
    consume(p, RIGHT_PAREN, "expected ')' after expression");

    Expr* curr = b_alloc(p->a, sizeof(Expr));
    curr->type = EXPR_GROUPING;
    curr->grouping.inner = inner;
    return curr;
  }

  die("expected expression");
  return NULL;
}

Expr* parseUnary(Parser* p) {

  //no unary operators currently
  return parsePrimary(p);
}

Expr* parseAddition(Parser* p) {

  Expr* left = parseUnary(p);

  while (check(p, PLUS)) {

    Token* operator = advance(p);
    Expr* right = parseUnary(p);

    Expr* curr = b_alloc(p->a, sizeof(Expr));
    curr->type = EXPR_BINARY;
    curr->binary.left = left;
    curr->binary.operator = operator->type;
    curr->binary.right = right;
    left = curr; //shove already made expr into left
  }

  return left;
}

Expr* parseExpr(Parser* p) {

  return parseAddition(p);
}

Stmt* parseReturnStmt(Parser* p) {

  Stmt* curr = b_alloc(p->a, sizeof(Stmt));
  curr->type = STMT_RETURN;

  consume(p, RETURN, "expected 'return'");
  curr->returnStmt.value = parseExpr(p);
  consume(p, SEMICOLON, "expected ';' after return value");

  return curr;
}

Stmt* parseDeclarationStmt(Parser* p) {

  Stmt* curr = b_alloc(p->a, sizeof(Stmt));
  curr->type = STMT_DECL;

  consume(p, INT, "expected 'int'");
  curr->declStmt.identifier = consume(p, IDENTIFIER, "expected variable name")->literal.b_string;

  if (check(p, EQUALS)) {

    curr->declStmt.value = parseExpr(p);
  }

  consume(p, SEMICOLON, "expected ';' after variable declaration");
  return curr;
}

Stmt* parseExpressionStmt(Parser* p) {

  Stmt* curr = b_alloc(p->a, sizeof(Stmt));
  curr->type = STMT_EXPR;

  curr->exprStmt.value = parseExpr(p);
  consume(p, SEMICOLON, "expected ';' after expression");

  return curr;
}

Stmt* parseStmt(Parser* p) {

  switch (peek(p)->type) {

    case RETURN: return parseReturnStmt(p);
    case INT: return parseDeclarationStmt(p);
    case IDENTIFIER: return parseExpressionStmt(p);

    default:
      die("expected statement");
      return NULL;
  }
}

Function* parseFunction(Parser* p) {

  Function* curr = b_alloc(p->a, sizeof(Function));
  curr->stmts = b_alloc(p->a, sizeof(Stmt*) * (p->count - p->current));
  curr->count = 0;

  consume(p, INT, "expected return type of 'int'");
  curr->identifier = consume(p, IDENTIFIER, "expected function name")->literal.b_string;
  //add that identifier to symbol table
  consume(p, LEFT_PAREN, "expected '('");
  consume(p, RIGHT_PAREN, "expected ')'");
  consume(p, LEFT_BRACE, "expected '{'");

  while (!check(p, RIGHT_BRACE)) {

    curr->stmts[curr->count++] = parseStmt(p);
  }

  consume(p, RIGHT_BRACE, "expected '}'");

  return curr;
}

Program* parseProgram(Parser* p) {

  Program* curr = b_alloc(p->a, sizeof(Program));
  curr->function = parseFunction(p);
  return curr;
}

Parser* parse(const Scanner* s) {

  Arena* a = b_allocArena();

  Parser* p = b_alloc(a, sizeof(Parser));
  p->a = a;
  p->tokens = s->tokens;
  p->count = s->count;
  p->current = 0;

  p->program = parseProgram(p);

  //check that entry is called main
  if (b_strcmp(p->program->function->identifier, "main") != 0) { die("entry function not called main"); }

  return p;
}

#include "bencthc/src/utils/print.h"

void printIndent(int depth) {
  for (int i = 0; i < depth; i++) b_printStringNoNewline("  ");
}

const char* tokenTypeName(enum TokenType t) {
  switch (t) {
    case PLUS:   return "+";
    //case MINUS:  return "-";
    case EQUALS: return "=";
    default:     return "?";
  }
}

void printExpr(Expr* e, int depth) {
  printIndent(depth);

  switch (e->type) {

    case EXPR_LITERAL:
      b_printStringNoNewline("Literal: ");
      b_printInt(e->literalExpr.value);
      break;

    case EXPR_VARIABLE:
      b_printStringNoNewline("Variable: ");
      b_printString(e->variable.name);
      break;

    case EXPR_UNARY:
      b_printStringNoNewline("Unary: ");
      b_printString(tokenTypeName(e->unary.op));
      printExpr(e->unary.operand, depth + 1);
      break;

    case EXPR_BINARY:
      b_printStringNoNewline("Binary: ");
      b_printString(tokenTypeName(e->binary.operator));
      printExpr(e->binary.left, depth + 1);
      printExpr(e->binary.right, depth + 1);
      break;

    case EXPR_GROUPING:
      b_printString("Grouping:");
      printExpr(e->grouping.inner, depth + 1);
      break;

    case EXPR_ASSIGN:
      b_printStringNoNewline("Assign: ");
      b_printString(e->assign.name);
      printExpr(e->assign.value, depth + 1);
      break;
  }
}

void printStmt(Stmt* s, int depth) {
  printIndent(depth);

  switch (s->type) {

    case STMT_RETURN:
      b_printString("Return:");
      printExpr(s->returnStmt.value, depth + 1);
      break;

    case STMT_EXPR:
      b_printString("ExprStmt:");
      printExpr(s->exprStmt.value, depth + 1);
      break;

    case STMT_DECL:
      b_printStringNoNewline("Decl: ");
      b_printString(s->declStmt.identifier);
      if (s->declStmt.value != NULL) {
        printExpr(s->declStmt.value, depth + 1);
      }
      break;
  }
}

void printFunction(Function* f, int depth) {
  printIndent(depth);
  b_printStringNoNewline("Function: ");
  b_printString(f->identifier);

  for (size_t i = 0; i < f->count; i++) {
    printStmt(f->stmts[i], depth + 1);
  }
}

void printProgram(const Program* p) {
  b_printString("Program");
  printFunction(p->function, 1);
}