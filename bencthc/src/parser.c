//
// Created by nick on 7/13/26.
//

#include "parser.h"

#include "utils/exit.h"
#include "utils/allocator.h"
#include "utils/iHateLibC.h"
#include "symbols.h"
#include "utils/string.h"
#include "structs.h"

//forward declarations
static Arena* blindParserArena;
static Scanner* scanner;

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

      //check for inentifier redefinition
      if (isDefined(context->symbolTable, token)) {
          
          die("redefinition of identifier");
      }
      else {

        insertSymbol(context->symbolTable, token);
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

Expression* parseMultiplicativeExpression(Context* context) {

  Expression* accumulator = parsePrimaryExpression(context);

  while (p_peek()->type == STAR || p_peek()->type == FORWARD_SLASH) {

    Expression* curr = b_alloc(blindParserArena, sizeof(Expression));
    curr->type = EXPRESSION_BINARY;
    curr->binary.left = accumulator;
    curr->binary.operator = p_advance()->type;
    curr->binary.right = parsePrimaryExpression(context);
    accumulator = curr;
  }

  return accumulator;
}

Expression* parseAdditiveExpression(Context* context) {

  Expression* accumulator = parseMultiplicativeExpression(context);

  while (p_peek()->type == PLUS || p_peek()->type == MINUS) {

    Expression* curr = b_alloc(blindParserArena, sizeof(Expression));
    curr->type = EXPRESSION_BINARY;
    curr->binary.left = accumulator;
    curr->binary.operator = p_advance()->type;
    curr->binary.right = parseMultiplicativeExpression(context);
    accumulator = curr;
  }

  return accumulator;
}

Expression* parseAssignmentExpression(Context* context) {

  Expression* accumulator = parseAdditiveExpression(context);

  while (p_peek()->type == EQUALS) {

    Expression* curr = b_alloc(blindParserArena, sizeof(Expression));
    curr->type = EXPRESSION_BINARY;
    curr->binary.left = accumulator;
    curr->binary.operator = p_advance()->type;
    curr->binary.right = parseAdditiveExpression(context);
    accumulator = curr;
  }

  return accumulator;
}

Expression* parseExpression(Context* context) {

  return parseAssignmentExpression(context);
}

Initializer* parseInitializer(Context* context) {

  Initializer* curr = b_alloc(blindParserArena, sizeof(Initializer));

  curr->assignmentExpression = parseAssignmentExpression(context);

  return curr;
}

Declaration* parseDeclaration(Context* context) {

  Declaration* curr = b_alloc(blindParserArena, sizeof(Declaration));

  curr->typeSpecifier = p_consume(INT, "only integer declarations are supported");

  curr->identifier = p_consume(IDENTIFIER, "expected identifier for declaration");

  //there is an initializer
  if (p_peek() == EQUALS) { curr->initializer = parseInitializer(context); }

  return curr;
}

Statement* parseStatement(Context* context) {

  Statement* curr = b_alloc(blindParserArena, sizeof(Statement));

  if (p_try(RETURN)) {

    curr->type = STATEMENT_JUMP;

    //check for empty expression
    if (p_try(SEMICOLON)) { 
      
      curr->jump.expression = NULL; 
      return curr; 
    }
    curr->jump.expression = parseExpression(context);
    return curr;
  }

  curr->type = STATEMENT_EXPRESSION;

  //check for empty expression
  if (p_try(SEMICOLON)) {

    curr->expression.expression = NULL;
    return curr;
  }
  curr->expression.expression = parseExpression(context);
  return curr;
}

Function* parseFunction(Context* context) {

  Function* function = b_alloc(blindParserArena, sizeof(Function));

  //push new scope for function
  function->symbolTable = newSymbolTable(context->symbolTable);
  context->symbolTable = function->symbolTable;

  function->returnType = p_consume(INT, "expected integer return type for function definition");
  function->identifier = p_consume(IDENTIFIER, "expected identifier after function return type");

  //in C89 compound statements (the `{ ... }` part of a function) puts all declaration before any statements

  //declarations
  //only INT right now, but check for other types later
  
  Declaration* declarationHead = NULL;
  Declaration* declarationTail = NULL;
  while (p_peek()->type == INT) {

    if (!declarationHead) {

      declarationHead = parseDeclaration(context);
      declarationTail = declarationHead;
    }
    else {

      declarationTail->next = parseDeclaration(context);
      declarationTail = declarationTail->next;
    }
  }
  function->declarations = declarationHead;

  //statements
  Statement* statementHead = NULL;
  Statement* statementTail = NULL;
  while (p_peek()->type != RIGHT_BRACE) {

    if (!statementHead) {

      statementHead = parseStatement(context);
      statementTail = statementHead;
    }
    else {

      statementTail->next = parseStatement(context);
      statementTail = statementTail->next;
    }
  }
  function->statements = statementHead;

  //pop back to previous scope
  context->symbolTable = context->symbolTable->outerScope;

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

  //create parsing context
  Context* context = b_alloc(blindParserArena, sizeof(Context));

  source->symbolTable = newSymbolTable(NULL);
  context->symbolTable = source->symbolTable;

  source->main = parseFunction(context);

  return source;
}
