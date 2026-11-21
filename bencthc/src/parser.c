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

  if (p_peek()->type == type) { 
    
    p_consume(type, ""); 
    return 1;
  }
  else {

    return 0;
  }
}

Expression* parsePrimaryExpression(Context* context) {

  Token* token = p_advance();
  Expression* curr = b_alloc(blindParserArena, sizeof(Expression));

  switch (token->type) {

    case IDENTIFIER :

      //check for inentifier redefinition, but allow current function shadowing
      if (isDefined(symbolTable, token) &&
          b_strcmp(context->currentFunction->identifier->literal.b_string, 
                   token->literal.b_string) != 0) {
          
          die("redefinition of identifier");
      }

      curr->type = EXPRESSION_IDENTIFIER;
      curr->identifier.identifier = token->literal.b_string;

    case INTEGER :

      curr->type = EXPRESSION_CONSTANT;
      curr->constant.constant = token->literal.b_integer;

    default :

      die("expected identifier or integer for primary expression");
  }

  return curr;
}

Expression* parseMultiplicativeExpression() {

  Expression* accumulator = parsePrimaryExpression();

  while (p_peek()->type == STAR || p_peek()->type == FORWARD_SLASH) {

    Expression* curr = b_alloc(blindParserArena, sizeof(Expression));
    curr->type = EXPRESSION_BINARY;
    curr->binary.left = accumulator;
    curr->binary.operator = p_advance()->type;
    curr->binary.right = parsePrimaryExpression();
    accumulator = curr;
  }

  return accumulator;
}

Expression* parseAdditiveExpression() {

  Expression* accumulator = parseMultiplicativeExpression();

  while (p_peek()->type == PLUS || p_peek()->type == MINUS) {

    Expression* curr = b_alloc(blindParserArena, sizeof(Expression));
    curr->type = EXPRESSION_BINARY;
    curr->binary.left = accumulator;
    curr->binary.operator = p_advance()->type;
    curr->binary.right = parseMultiplicativeExpression();
    accumulator = curr;
  }

  return accumulator;
}

Expression* parseAssignmentExpression() {

  Expression* accumulator = parseAdditiveExpression();

  while (p_peek()->type == EQUALS) {

    Expression* curr = b_alloc(blindParserArena, sizeof(Expression));
    curr->type = EXPRESSION_BINARY;
    curr->binary.left = accumulator;
    curr->binary.operator = p_advance()->type;
    curr->binary.right = parseAdditiveExpression();
    accumulator = curr;
  }

  return accumulator;
}

Expression* parseExpression() {

  return parseAssignmentExpression();
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

  return curr;
}

Statement* parseStatement() {

  Statement* curr = b_alloc(blindParserArena, sizeof(Statement));

  if (p_try(RETURN)) {

    curr->type = STATEMENT_JUMP;

    //check for empty expression
    if (p_try(SEMICOLON)) { 
      
      curr->jump.expression = NULL; 
      return curr; 
    }
    curr->jump.expression = parseExpression();
    return curr;
  }

  curr->type = STATEMENT_EXPRESSION;

  //check for empty expression
  if (p_try(SEMICOLON)) {

    curr->expression.expression = NULL;
    return curr;
  }
  curr->expression.expression = parseExpression();
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
  while (p_peek()->type == INT) {

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
  while (p_peek()->type != RIGHT_BRACE) {

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

  return function;
}

Source* parse(Scanner* s) {
  
  //set up scanner
  scanner = s;  

  //source can only begin with main function definition returning int
  if (p_peek()->type != INT) { die("main function does not return int"); }
  if (p_peekNext()->type != IDENTIFIER) { die("source does not begin with function definition"); }
  if (b_strcmp(p_peekNext()->literal.b_string, "main") != 0) { die("source does not begin with definition for \"main\""); }

  Source* source = b_alloc(blindParserArena, sizeof(Source));
  source->main = parseFunction();

  return source;
}
