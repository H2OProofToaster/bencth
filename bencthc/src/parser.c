//
// Created by nick on 7/13/26.
//

#include "parser.h"

#include "utils/exit.h"
#include "utils/string.h"
#include "structs.h"

//forward declarations
Expr* parseExpr(SymbolTable* sT);
static Arena* parserArena = NULL;
static Parser* p = NULL;

Token* p_peek() {

  return &p->tokens[p->current];
}

Token* p_peekNext() {

  return &p->tokens[p->current + 1];
}

Token* p_advance() {

  Token* t = &p->tokens[p->current];
  if (p->current < p->count) { p->current++; }
  return t;
}

int check(const TokenType type) {

  return p_peek()->type == type;
}

int isType(const Token* t) {

  /*
  for (size_t i = 0; i < tT->count; i++) {

    if (b_strcmp(t->literal.b_string, tT->types[i]->token->literal.b_string) == 0) { return 1; }
  }
  */

  switch (t->type) {

    case INT:
      return 1;

    default:
      return 0;
  }
}

Token* consume(const TokenType type, const char* err) {

  if (check(type)) { return p_advance(); }

  die(err);
}

SymbolTable* initSymbolTable(SymbolTable* outer) {

  SymbolTable* sT = b_alloc(parserArena, sizeof(SymbolTable));
  sT->head = NULL;
  sT->outerScope = outer;
  return sT;
}

int isDefined(const SymbolTable* sT, const char* name) {

  for (const Symbol* s = sT->head; s != NULL; s = s->next) {

    if (b_strcmp(s->token->literal.b_string, name) == 0) { return 1; }
  }

  return 0;
}

void addSymbol(SymbolTable* sT, Token* token) {

  if (isDefined(sT, token->literal.b_string)) { die("Symbol already exists"); }

  Symbol* s = b_alloc(parserArena, sizeof(Symbol));
  s->token = token;
  s->next = sT->head;
  sT->head = s;
}

Expr* parsePrimary(SymbolTable* sT) {

    if (check(INTEGER)) {

    Expr* curr = b_alloc(parserArena, sizeof(Expr));
    curr->type = EXPR_LITERAL;
    curr->literal.value = p_advance()->literal.b_integer;
    return curr;
  }

  if (check(IDENTIFIER)) {

    Expr* curr = b_alloc(parserArena, sizeof(Expr));
    curr->type = EXPR_VARIABLE;
    Token* t = p_advance();
    curr->variable.name = t->literal.b_string;
    return curr;
  }

  if (check(LEFT_PAREN)) {

    p_advance(); //eat '('
    Expr* inner = parseExpr(sT);
    consume(RIGHT_PAREN, "expected ')' after expression");

    Expr* curr = b_alloc(parserArena, sizeof(Expr));
    curr->type = EXPR_GROUPING;
    curr->grouping.inner = inner;
    return curr;
  }

  die("expected expression");
}

Expr* parseUnary(SymbolTable* sT) {

  //no unary operators currently
  return parsePrimary(sT);
}

Expr* parseMultiplication(SymbolTable* sT) {

  Expr* left = parseUnary(sT);

  while (check(STAR) || check(FORWARD_SLASH)) {

    Token* operator = p_advance();
    Expr* right = parseUnary(sT);

    Expr* curr = b_alloc(parserArena, sizeof(Expr));
    curr->type = EXPR_BINARY;
    curr->binary.left = left;
    curr->binary.operator = operator->type;
    curr->binary.right = right;
    left = curr;
  }

  return left;
}

Expr* parseAddition(SymbolTable* sT) {

  Expr* left = parseMultiplication(sT);

  while (check(PLUS) || check(MINUS)) {

    Token* operator = p_advance();
    Expr* right = parseMultiplication(sT);

    Expr* curr = b_alloc(parserArena, sizeof(Expr));
    curr->type = EXPR_BINARY;
    curr->binary.left = left;
    curr->binary.operator = operator->type;
    curr->binary.right = right;
    left = curr; //shove already made expr into left
  }

  return left;
}

Expr* parseExpr(SymbolTable* sT) {

  return parseAddition(sT);
}

Stmt* parseReturnStmt(SymbolTable* sT) {

  Stmt* curr = b_alloc(parserArena, sizeof(Stmt));
  curr->type = STMT_RETURN;

  consume(RETURN, "expected 'return'");
  curr->returnStmt.expr = parseExpr(sT);
  consume(SEMICOLON, "expected ';' after return value");

  return curr;
}

Stmt* parseDeclarationStmt(SymbolTable* sT) {

  Stmt* curr = b_alloc(parserArena, sizeof(Stmt));
  curr->type = STMT_DECL;

  consume(INT, "expected 'int'");
  curr->declStmt.type = INT;
  Token* identifier = consume(IDENTIFIER, "expected variable name");
  curr->declStmt.identifier = identifier->literal.b_string;

  if (p_peek()->type == EQUALS) {

    p_advance(); //eat '='
    curr->declStmt.expr = parseExpr(sT);
  }
  else { curr->declStmt.expr = NULL; }

  consume(SEMICOLON, "expected ';' after variable declaration");

  addSymbol(sT, identifier);

  return curr;
}

Stmt* parseExpressionStmt(SymbolTable* sT) {

  Stmt* curr = b_alloc(parserArena, sizeof(Stmt));
  curr->type = STMT_EXPR;

  curr->exprStmt.identifier = consume(IDENTIFIER, "expected identifier")->literal.b_string;
  consume(EQUALS, "expected '=' after identifier");
  curr->exprStmt.expr = parseExpr(sT);
  consume(SEMICOLON, "expected ';' after expression");

  return curr;
}

Stmt* parseStmt(SymbolTable* sT) {

  switch (p_peek()->type) {

    case RETURN: return parseReturnStmt(sT);
    case IDENTIFIER: return parseExpressionStmt(sT);

    default:

      if (isType(p_peek())) { return parseDeclarationStmt(sT); }

      die("expected statement");
  }
}

Function* parseFunction(SymbolTable* sT) {

  Function* curr = b_alloc(parserArena, sizeof(Function));
  curr->stmts = b_alloc(parserArena, sizeof(Stmt*) * (p->count - p->current));
  curr->count = 0;

  SymbolTable* innerST = initSymbolTable(sT);
  curr->symbolTable = innerST;
  innerST->outerScope = sT;

  consume(INT, "expected return type of 'int'");
  Token* t = consume(IDENTIFIER, "expected function name");
  curr->identifier = t->literal.b_string;

  //function identifier gets added to the scope its in only
  //i.e. you can still name a variable foo inside a function foo()
  addSymbol(sT, t);

  consume(LEFT_PAREN, "expected '('");
  consume(RIGHT_PAREN, "expected ')'");
  consume(LEFT_BRACE, "expected '{'");

  while (!check(RIGHT_BRACE)) {

    curr->stmts[curr->count++] = parseStmt(innerST);
  }

  consume(RIGHT_BRACE, "expected '}'");

  return curr;
}

Program* parseProgram() {

  Program* curr = b_alloc(parserArena, sizeof(Program));
  curr->symbolTable = initSymbolTable(NULL);
  curr->function = parseFunction(curr->symbolTable);
  return curr;
}

Parser* parse(const Scanner* s) {

  parserArena = b_allocArena();

  p = b_alloc(parserArena, sizeof(Parser));
  p->a = parserArena;
  p->tokens = s->tokens;
  p->count = s->count;
  p->current = 0;

  p->program = parseProgram();

  //check that entry is called main
  if (b_strcmp(p->program->function->identifier, "main") != 0) { die("entry function not called main"); }

  return p;
}