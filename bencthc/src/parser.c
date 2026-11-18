//
// Created by nick on 7/13/26.
//

#include "parser.h"

#include "utils/exit.h"
#include "utils/allocator.h"
#include "utils/iHateLibC.h"
#include "utils/symbols.h"
#include "utils/string.h"
#include "structs.h"

//forward declarations
static Arena* blindParserArena;
static Scanner* scanner;
static SymbolTable* symbolTable;

//helpers for tokenstream
Token* p_lookN(const size_t n) {

  if (scanner->current >= scanner->count) { die("end of tokenstream"); }

  return &scanner->tokens[scanner->current + n - 1];
}

Token* p_peek() {

  return p_lookN(1);
}

Token* p_peekNext() {

  return p_lookN(2);
}

Token* p_advance() {

  Token* t = &scanner->tokens[scanner->current];
  if (scanner->current < scanner->count) { scanner->current++; }
  return t;
}

int p_check(const TokenType type) {

  return p_peek()->type == type;
}

Token* p_consume(const TokenType type, const char* err) {

  if (p_check(type)) { return p_advance(); }

  die(err);
}

int p_try(const TokenType type) {

  if (p_peek(type)) { 
    
    p_consume(type, ""); 
    return 1;
  }
  else {

    return 0;
  }
}

PrimaryExpression* parsePrimaryExpression() {

  Token* token = p_advance();
  PrimaryExpression* curr = b_alloc(blindParserArena, sizeof(PrimaryExpression));

  switch (token->type) {

    case IDENTIFIER :

      curr->type = PRIMARY_IDENTIFIER;
      curr->identifier.identifier = token->literal.b_string;

    case INTEGER :

      curr->type = PRIMARY_CONSTANT;
      curr->constant.constant = token->literal.b_integer;
  }

  return curr;
}

MultiplicativeExpression* parseMultiplicativeExpression() {

  PrimaryExpression* left = parsePrimaryExpression();

  if (p_peek() == STAR || p_peek() == FORWARD_SLASH) {

    MultiplicativeExpression* curr = b_alloc(blindParserArena, sizeof(MultiplicativeExpression));
    curr->left = left;
    curr->operator = p_advance();
    curr->right = parseMultiplicativeExpression();
    return curr;
  }
  else { 

    MultiplicativeExpression* curr = b_alloc(blindParserArena, sizeof(MultiplicativeExpression));
    curr->left = left;
    return curr;
  }
}

AdditiveExpression* parseAdditiveExpression() {

  MultiplicativeExpression* curr = parseMultiplicativeExpression();

  while (p_peek() == PLUS || p_peek() == MINUS) {

    MultiplicativeExpression* temp = b_alloc(blindParserArena, sizeof(MultiplicativeExpression));
    temp->left = curr;
    temp->operator = p_advance();
    temp->right = parseMultiplicativeExpression();
    curr = temp;
  }

  return curr;
}

AssignmentExpression* parseAssignmentExpression() {

  AdditiveExpression* curr = parseAdditiveExpression();

  while (p_peek() == EQUALS) {

    AdditiveExpression* temp = b_alloc(blindParserArena, sizeof(AdditiveExpression));
    temp->left = curr;
    temp->operator = p_advance();
    temp->right = parseAdditiveExpression();
    curr = temp;
  }

  return curr;
}

Expression* parseExpression() {

  AssignmentExpression* left = parseAssignmentExpression();

  return left;
}

Initializer* parseInitializer() {

  Initializer* curr = b_alloc(blindParserArena, sizeof(Initializer));

  curr->assignmentExpression = parseAssignmentExpression();

  return curr;
}

Declaration* parseDeclaration() {

  Declaration* curr = b_alloc(blindParserArena, sizeof(Declaration));

  curr->typeSpecifier = p_consume(INT, "only integer declarations are supported");

  curr->identifier = p_consume(IDENTIFIER, "expected identifier for declaration");

  //there is an initializer
  if (p_peek() == EQUALS) { curr->initializer = parseInitializer(); }
}

Statement* parseStatement() {

  Statement* curr = b_alloc(blindParserArena, sizeof(Statement));

  if (p_try(RETURN)) {

    curr->type = STATEMENT_JUMP;

    //check for empty expression
    if (p_try(SEMICOLON)) { 
      
      curr->jumpStatement.expression = NULL; 
      return curr; 
    }
    curr->jumpStatement.expression = parseExpression();
    return curr;
  }

  curr->type = STATEMENT_EXPRESSION;

  //check for empty expression
  if (p_try(SEMICOLON)) {

    curr->expressionStatement.expression = NULL;
    return curr;
  }
  curr->expressionStatement.expression = parseExpression();
  return curr;
}

Function* parseFunction() {

  Function* function = b_alloc(blindParserArena, sizeof(Function));

  function->returnType = p_consume(INT, "expected integer return type for function definition");
  function->identifier = p_consume(IDENTIFIER, "expected identifier after function return type");

  //in C89 compound statements (the `{ ... }` part of a function) puts all declaration before any statements

  //declarations
  //only INT right now, but check for other types later
  
  Declaration* declarationHead = NULL;
  Declaration* declarationTail = NULL;
  while (p_peek() == INT) {

    if (!declarationHead) {

      declarationHead = parseDeclaration();
      declarationTail = declarationHead;
    }
    else {

      declarationTail->next = parseDeclaration();
      declarationTail = declarationTail->next;
    }
  }
  function->declarations = declarationHead;

  //statements
  Statement* statementHead = NULL;
  Statement* statementTail = NULL;
  while (p_peek() != RIGHT_BRACE) {

    if (!statementHead) {

      statementHead = parseStatement();
      statementTail = statementHead;
    }
    else {

      statementTail->next = parseStatement();
      statementTail = statementTail->next;
    }
  }
  function->statements = statementHead;
}

Source* parse() {

  //source can only begin with main function definition returning int
  if (p_peek()->type != INT) { die("main function does not return int"); }
  if (p_peekNext()->type != IDENTIFIER) { die("source does not begin wit function definition"); }
  if (b_strcmp(p_peekNext()->literal.b_string, "main") != 0) { die("source does not begin with definition for \"main\""); }

  Source* source = b_alloc(blindParserArena, sizeof(Source));
  source->main = parseFunction();
}
