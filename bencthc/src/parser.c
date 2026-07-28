//
// Created by nick on 7/13/26.
//

#include "parser.h"
#include "scanner.h"
#include "utils/exit.h"
#include "utils/string.h"

//forward declarations
Expr* parseExpr(Parser* p, IdentifierTable* iT);
static Arena* parserArena = NULL;

Token* peek(const Parser* p) {

  return &p->tokens[p->current];
}

Token* peekNext(const Parser* p) {

  return &p->tokens[p->current + 1];
}

Token* advance(Parser* p) {

  Token* t = &p->tokens[p->current];
  if (p->current < p->count) { p->current++; }
  return t;
}

int check(const Parser* p, const enum TokenType type) {

  return peek(p)->type == type;
}

int isType(const Token* t, const TypeTable* tT) {

  for (size_t i = 0; i < tT->count; i++) {

    if (b_strcmp(t->literal.b_string, tT->types[i]->token->literal.b_string) == 0) { return 1; }
  }

  switch (t->type) {

    case INT:
      return 1;

    default:
      return 0;
  }
}

Token* consume(Parser* p, const enum TokenType type, const char* err) {

  if (check(p, type)) { return advance(p); }

  die(err);
  return NULL;
}

IdentifierTable* initIdentifierTables(IdentifierTable* outer) {

  IdentifierTable* iT = b_alloc(parserArena, sizeof(IdentifierTable));

  iT->symbolTable = b_alloc(parserArena, sizeof(SymbolTable));
  iT->typeTable = b_alloc(parserArena, sizeof(TypeTable));
  iT->symbolTable->count = 0;
  iT->typeTable->count = 0;
  iT->outerScope = outer;
  return iT;
}

void addSymbol(SymbolTable* table, Token* token) {

  Symbol* s = b_alloc(parserArena, sizeof(Symbol));
  s->token = token;
  table->symbols[++table->count] = s;
}

Expr* parsePrimary(Parser* p, IdentifierTable* iT) {

  if (check(p, INTEGER)) {

    Expr* curr = b_alloc(parserArena, sizeof(Expr));
    curr->type = EXPR_LITERAL;
    curr->literal.value = advance(p)->literal.b_integer;
    return curr;
  }

  if (check(p, IDENTIFIER)) {

    Expr* curr = b_alloc(parserArena, sizeof(Expr));
    curr->type = EXPR_VARIABLE;
    Token* t = advance(p);
    curr->variable.name = t->literal.b_string;

    addSymbol(iT->symbolTable, t);

    return curr;
  }

  if (check(p, LEFT_PAREN)) {

    advance(p); //eat '('
    Expr* inner = parseExpr(p, iT);
    consume(p, RIGHT_PAREN, "expected ')' after expression");

    Expr* curr = b_alloc(parserArena, sizeof(Expr));
    curr->type = EXPR_GROUPING;
    curr->grouping.inner = inner;
    return curr;
  }

  die("expected expression");
  return NULL;
}

Expr* parseUnary(Parser* p, IdentifierTable* iT) {

  //no unary operators currently
  return parsePrimary(p, iT);
}

Expr* parseMultiplication(Parser* p, IdentifierTable* iT) {

  Expr* left = parseUnary(p, iT);

  while (check(p, STAR) || check(p, FORWARD_SLASH)) {

    Token* operator = advance(p);
    Expr* right = parseUnary(p, iT);

    Expr* curr = b_alloc(parserArena, sizeof(Expr));
    curr->type = EXPR_BINARY;
    curr->binary.left = left;
    curr->binary.operator = operator->type;
    curr->binary.right = right;
    left = curr;
  }

  return left;
}

Expr* parseAddition(Parser* p, IdentifierTable* iT) {

  Expr* left = parseMultiplication(p, iT);

  while (check(p, PLUS) || check(p, MINUS)) {

    Token* operator = advance(p);
    Expr* right = parseMultiplication(p, iT);

    Expr* curr = b_alloc(parserArena, sizeof(Expr));
    curr->type = EXPR_BINARY;
    curr->binary.left = left;
    curr->binary.operator = operator->type;
    curr->binary.right = right;
    left = curr; //shove already made expr into left
  }

  return left;
}

Expr* parseExpr(Parser* p, IdentifierTable* iT) {

  return parseAddition(p, iT);
}

Stmt* parseReturnStmt(Parser* p, IdentifierTable* iT) {

  Stmt* curr = b_alloc(parserArena, sizeof(Stmt));
  curr->type = STMT_RETURN;

  consume(p, RETURN, "expected 'return'");
  curr->returnStmt.expr = parseExpr(p, iT);
  consume(p, SEMICOLON, "expected ';' after return value");

  return curr;
}

Stmt* parseDeclarationStmt(Parser* p, IdentifierTable* iT) {

  Stmt* curr = b_alloc(parserArena, sizeof(Stmt));
  curr->type = STMT_DECL;

  consume(p, INT, "expected 'int'");
  curr->declStmt.identifier = consume(p, IDENTIFIER, "expected variable name")->literal.b_string;

  if (check(p, EQUALS)) {

    curr->declStmt.expr = parseExpr(p, iT);
  }

  consume(p, SEMICOLON, "expected ';' after variable declaration");
  return curr;
}

Stmt* parseExpressionStmt(Parser* p, IdentifierTable* iT) {

  Stmt* curr = b_alloc(parserArena, sizeof(Stmt));
  curr->type = STMT_EXPR;

  curr->exprStmt.expr = parseExpr(p, iT);
  consume(p, SEMICOLON, "expected ';' after expression");

  return curr;
}

Stmt* parseStmt(Parser* p, IdentifierTable* iT) {

  switch (peek(p)->type) {

    case RETURN: return parseReturnStmt(p, iT);
    case IDENTIFIER:
      consume(p, EQUALS, "expected '=' after variable");
      return parseExpressionStmt(p, iT);

    default:

      if (isType(peek(p), iT->typeTable)) {

        return parseDeclarationStmt(p, iT);
      }

      die("expected statement");
      return NULL;
  }
}

Function* parseFunction(Parser* p, IdentifierTable* iT) {

  Function* curr = b_alloc(parserArena, sizeof(Function));
  curr->stmts = b_alloc(parserArena, sizeof(Stmt*) * (p->count - p->current));
  curr->count = 0;
  IdentifierTable* innerIT = initIdentifierTables(iT);

  consume(p, INT, "expected return type of 'int'");
  Token* t = consume(p, IDENTIFIER, "expected function name");
  curr->identifier = t->literal.b_string;

  //function identifier gets added to the scope its in only
  //i.e. you can still name a variable foo inside a function foo()
  addSymbol(iT->symbolTable, t);

  consume(p, LEFT_PAREN, "expected '('");
  consume(p, RIGHT_PAREN, "expected ')'");
  consume(p, LEFT_BRACE, "expected '{'");

  while (!check(p, RIGHT_BRACE)) {

    curr->stmts[curr->count++] = parseStmt(p, innerIT);
  }

  consume(p, RIGHT_BRACE, "expected '}'");

  return curr;
}

Program* parseProgram(Parser* p, IdentifierTable* iT) {

  Program* curr = b_alloc(parserArena, sizeof(Program));
  curr->function = parseFunction(p, iT);
  return curr;
}

Parser* parse(const Scanner* s) {

  parserArena = b_allocArena();

  Parser* p = b_alloc(parserArena, sizeof(Parser));
  p->a = parserArena;
  p->tokens = s->tokens;
  p->count = s->count;
  p->current = 0;

  p->iT = initIdentifierTables(NULL);

  p->program = parseProgram(p, p->iT);

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
      b_printInt(e->literal.value);
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
      printExpr(e->assign.expr, depth + 1);
      break;
  }
}

void printStmt(Stmt* s, int depth) {
  printIndent(depth);

  switch (s->type) {

    case STMT_RETURN:
      b_printString("Return:");
      printExpr(s->returnStmt.expr, depth + 1);
      break;

    case STMT_EXPR:
      b_printString("ExprStmt:");
      printExpr(s->exprStmt.expr, depth + 1);
      break;

    case STMT_DECL:
      b_printStringNoNewline("Decl: ");
      b_printString(s->declStmt.identifier);
      if (s->declStmt.expr != NULL) {
        printExpr(s->declStmt.expr, depth + 1);
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